## CPP内存管理入门初探
### 代码内容大部分引用至侯捷老师

### 1.[primitives](primitives.md)
- 内存管理入门

### 2.[allocator](allocator.md)
- 基础分配器

### 3.[alloc](alloc.md)
- 标准库分配器

## vc6.0 malloc设计一览
### SBH（small block heap）之始 _heap_init()和__sbh_heap_init()
#### _heap_init()里的操作一览
  - HeapCreate()向操作系统申请内存块,赋值给_crtheap,方便以后进行管理,准备16个Header,有点类似于alloc的free_list,但是更底层,几乎可以看成是二进制管理,准确点的话应该说是free_list模仿自header

## vc6.0 malloc执行顺序-->调用栈顺序(debug模式下调用)
- 1. _heap_init():堆初始化
- 2. _ioinit():io初始化，第一次分配内存,分配256字节,即100H
- 3. _heap_alloc_dbg:调整内存区块大小和补充内存块信息,第二次分配内存，分配调整过后的大小,同时做调整内存块指针的工作，基本可以看成是free_list构建链表间指针的操作,而且一直持有链表指针，就算把内存分配给客户之后，也掌握着链表的指向，存储着内存的信息
- 4. _heap_alloc_base:检查调整过后的Block大小，如果小于或等于小区块大小(1016=1024-8bytes)，那么交给sbh进行分配，否则交给操作系统进行分配
- 5. __sbh_alloc_block:加上8bytes的cookie，同时对内存大小进行上调到16的倍数
- 6. __sbh_alloc_new_region:一个header内含有两个指针，一个指针指向实际逻辑内存地址的位置，一个指针指向内存管理空间，也就是region．region负责管理当前内存块的使用情况．region持有32个tagGroup，每个tagGroup持有两个指针，做出一个双向链表，总共有64条双向链表.总共16k左右．为了良好管理虚拟内存的使用情况．此函数调用的结果是获取一块新的region，以供管理使用．
- 7. __sbh_alloc_new_group:将虚拟内存进行切块划分，同时将每一块串到上面分配好的region里的最后一个group指针上进行管理，每一块都和附近的块进行串联，形成一整块的链表队列．
  - 最后一条链表的特殊之处：所有大于1kb的块，均归它所管
  - tagEntry(4096bytes):将虚拟内存切块划分的单位，持有块大小，前后块等信息．以及保留数据块，提供给用户使用．前后块指针方便将每一块串起来，串成一个双向链表，提供给region的group块进行管理
  - malloc和free本质是group里的双向链表之间指针的变动
  - group块进行切割分配，将本身内部保留区的指针给传到外部，那么客户就认为自己获得了这一块内存的所有权．切割的时候获得的指针是指向cookie头的，在return回去的时候，要将指针进行偏移，直到指向真正的客户使用区块．
  - 每分配一块内存给客户，那块内存前后都夹有无人区，方便编译器检测，是否超过内存边界．
- 找寻新的内存区块进行分配的时候，如果对应位置上的链表没有管理内存区块，是空链表，那么会在region中往后寻找最接近的链表，找到之后再在对应链表管理的区块上进行切割．
- 如果在当前索引的group上都找不到合适的区块的话，那么会跳到下一个group进行内存区块分配．

## vc6.0 free执行特点分析
- 先计算出对应链表位置进行回收，之后将目的区块无人区的大小进行变动(-1)，则视为该区块已经回收．
- 同时在对应区块上embeded pointer链接回计算出的链表(group)
- 修改region上对应链表的状态，如果本身region对应位上的链表没有区块，挂上回收的区块之后，视为正在管理区块的链表，将其状态位修改为１．如果本身已经管理有区块，则不需要修改．如果回收之后，链表管理区块为空，那么region也需要对应修改．
- free之时，在pages中，如果上下存在空闲区块，那么会进行上合并或者下合并或者上下一起合并．
- 上下cookie（无人区）的作用：回收内存块的时候，进行检查上下cookie，在进行cookie大小进行变动的时候，如果检查到上cookie或者下cookie是为空标志，那么就可以进行合并．如果没有下cookie，那么就无法进行上合并．本质是检查无人区最后一位是否为０，如果为０则表明该区块是空区块．
- free时，要先找到Header，然后找到对应的group，之后再将其挂上fre_list上
- 分段管理可以区分责任，方便从系统申请和归还
- 全回收：看group首部cntEntries，如果为0则回收此块内存,全回收之时，调用VirtualFree还回操作系统。有两个group可以归还的时候才进行全回收的归还动作。
- Defering:有__sbh_pHeaderDefer是一个指针，指向一个全回收group所属的Header，当一个新的全回收group出现，才将这个group给归还给操作系统，同时Defer指针指向新出现的全回收group.如果出现新的分配block请求，那么会从Defer group中取出block完成分配，同时Defer指针会取消。
- __sbh_indGroupDefer是个索引，指出Region中哪个group是Defer.
- 在分配block之前，会先检查Header的计数器是否为0,如果为0,那么就看Header是否Defer且此次所用的group是否Defer.都吻合的话就取消其Defer身份(令__sbh_pHeaderDefer为null)
- 当归还完所有内存时，就会恢复到初始状态，也就是调用__heap_init状态

## vc6.0 malloc&free学习总结
- 由上至下的allocator->new->malloc->virtual_alloc一层层的封装抽象之下，每一层都不知道附近层的实现，都不互相依赖。其传递的都是信息。
- allocator设计为了避免cookie的空间浪费，new为了封装malloc，给类对象层次之间多一层抽象，而malloc为了减少virtual_alloc的调用次数，以及内存碎块问题，尽心尽力。
- 由此观之，可以大概得出结论：如果不是特别需求，new足以满足日常的内存分配需要，而且在性能和碎块数量都良好的情况下，高层业务逻辑设计已经不太需要自己再重头设计一个内存池之类的抽象。

## loki::allocator
- 三个对象，由上至下进行管理
  ```cpp
  class Chunk{
    //当前管理的一大块内存块的头部指针
    unsigned char* pData_;
    //第一块空闲的内存块的偏移量
    unsigned char firstAvailableBlock_;
    //当前这一大块内存块中，可以切割出多少块小的内存块
    unsigned char blocksAvailable_;
  };
  class FixedAllocator{
    //管理一堆Chunk
    vector<Chunk> chunks_;
    //最近分配的Chunk指针
    Chunk* allocChunk_;
    //最近要回收的Chunk指针
    Chunk* deallocChunk_;
  };
  class SmallObjAllocator{
    //管理一堆FixedAllocator
    vector<FixedAllocator> pool_;
    FixedAllocator* pLastAlloc;
    FixedAllocator* pLastDealloc;
    size_t chunkSize;
    size_t maxObjectSize;
  };
  ```

### Chunk实现细节
```cpp
//初始化申请内存段
//blockSize:区块大小 blocks:区块数量
void FixedAllocator::Chunk::Init(std::size_t blockSize,unsigned char blocks)
{
  pData_ = new unsigned char[blockSize*blocks];
  Reset(blockSize,blocks);
}

//初始化申请的内存段，将pData_切割成内存块，并给其标上序号
//在操作结果上，可以近似的看成，将pData_申请的内存段，给切成一块块连续的内存块，同时在每个小块的头部打上索引号
void FixedAllocator::Chunk::Reset(std::size_t blockSize,unsigned char blocks)
{
  firstAvailableBlock_ = 0;
  blocksAvailable_ = blocks;
  unsigned char i = 0;
  unsigned char* p = pData_;
  //标识索引号
  for(;i!=blocks;p+=blockSize)
    *p=++i;
}

//此函数被上一层调用
void FixedAllocator::Chunk::Release()
{
  delete[] pData_;//释放自己
}

//Chunk分配内存接口
//三层内存管理结构中，最低的一层，负责直接从操作系统中申请内存并进行管理
void* FixedAllocator::Chunk::Allocate(std::size_t blockSize)
{
  //当前内存段没有可用内存块
  if(!blocksAvailable_)
    return 0;
  //找到空闲区块
  unsigned char* pResult = pData_ + (firstAvailableBlock_ * blockSize);
  //修改空闲区块索引值
  //找到当前分配出去的内存块的索引值，并将它保存记录下来
  //它记录有下一块空闲内存块的位置
  firstAvailableBlock_ = *pResult;
  //降低空闲区块数量
  --blocksAvailable_;
  return pResult;
}

//Chunk回收管理内存的接口
void FixedAllocator::Chunk::Deallocate(void* p,std::size_t blockSize)
{
  unsigned char* toRelease = static_cast<unsigned char*>(p);
  //给当前给回内存块，打上索引值，此索引值是下一块空闲内存的索引值
  *toRelease = firstAvailableBlock_;
  //当前指针属于哪一块索引的，只需要将当前指针-头部指针的地址，再除以内存块大小，即可得出索引值
  //此索引值是记录的Chunk中的，最高优先级的索引区块值
  //每次分配时，会第一个分配出去，同时需要读取该内存块的上的索引值，该内存块上的索引值指向下一块空闲内存
  firstAvailableBlock_ = static_cast<unsigned char>((toRelease - pData_)/blockSize);
  //增加空闲区块数量
  ++blocksAvailable_;
}
```

### FixedAllocator实现细节
```cpp
//FixedAllocator内存分配
//本质上是从自身管理的Chunk列表中寻找空闲的Chunk，来进行内存分配
void* FixedAllocator::Allocate()
{
  //目前没有标定chunk或者该chunk已经没有可用区块
  if(allocChunk_ == 0 || allocChunk_->blocksAvailable_ == 0)
  {
    //从头找起
    Chunks::iterator i = chunks_.begin();
    for(;;++i)
    {
      //到达尾端，都没找到
      if(i == chunks_.end())
      {
        //创建新的Chunk挂到容器尾端
        //没有处理，如果新建对象失败的情况
        chunks_.push_back(Chunk());
        //指向末端的Chunk
        Chunk& newHChunk = chunks_.back();
        //设定好索引
        newChunk.Init(blockSize_,numBlocks_);
        //将指针指向新的Chunk，方便之后继续分配
        allocChunk_ = &newChunk;
        //标定回收Chunk的指针
        //因为chunks_是vector，在成长的时候，有可能出现搬动的情况
        //于是需要重设指针，只好将deallocChunk_指向头部
        deallocChunk_ = &chunks.front();
        break;
      }
      if(i->blocksAvailable_ > 0)
      {
        //寻找chunk的过程中找到一个chunk有可用内存块
        //因为是iterator对象，所以需要先解引用取其值，再取其地址
        allocChunk_ = &*i;
        break;
      }
    }
  }
  //向这个chunk取内存块
  //下次再申请分配内存的话，直接从现在这个块上找起
  return allocChunk_->Allocate(blockSize_);
}

//FixedAllocator内存释放
void FixedAllocator::Deallocate(void* p)
{
  deallocChunk_ = VicinityFind(p);
  DoDeallocate(p);
}

//临近查找
//如果传入的指针不是本系统分配的，会找不到对应的chunk,那么会出现死循环
//判断传入指针的地址，是否在管理的内存区块段内。只需要判断头地址和尾地址即可
//于是弄出了头尾一起遍历的特殊设计
//直觉上比一半的遍历效率高，最坏情况下在O(n)
FixedAllocator::Chunk* FixedAllocator::VicinityFind(void* p)
{
  //一个内存段的大小
  const std::size_t chunkLength = numBlocks_ * blockSize_;
  //当前准备释放的内存段的头地址
  Chunk* lo = deallocChunk_;
  //当前准备释放的内存段的尾地址
  Chunk* hi = deallocChunk_ + 1;
  //当前管理的内存段的首地址，也就是容器中第一个内存段的头地址
  Chunk* loBound = &chunks.front();
  //当前管理的内存段的尾地址，也就是容器中最后一个元素再偏移一个元素的头地址
  Chunk* hiBound = &chunks_.back() + 1;
  for(;;)
  {
    //判断lo是否走到了尽头
    if(lo)
    {
      //p落在lo区间内
      if(p >= lo->pData_) && p < lo->pData_ + chunkLength)
      {
        return lo;
      }
      if(lo == loBound)
      {
        lo = 0;
      }
      else
      {
        //lo往上寻找，所以自减
        --lo;
      }
    }
    //判断hi是否走到了尽头
    if(hi)
    {
      //p落在hi的区间内
      if(p >= hi->pData_) && p < hi->pData_ + chunkLength)
      {
        return hi;
      }
      //hi往下寻找，所以自增
      if(++hi == hiBound)
      {
        hi = 0;
      }
    }
  }
  return nullptr;
}

void FixedAllocator::DoDeallocate(void* p)
{
  //对应区块回收
  deallocChunk_->Deallocate(p,blockSize_);
  //全回收处理
  if(deallocChunk_->blocksAvailable_ == numBlocks_)
  {
    Chunk& lastChunk = chunks_.back();//标出最后一个
    //如果最后一个就是当前释放内存的chunk
    if(&lastChunk == deallocChunk_)
    {
      //有两个free chunk，抛弃最后一个
      if(chunks_.size() > 1 && deallocChunk_[-1].blocksAvailable_ == numBlocks_)
      {
        lastChunk.Release();
        chunks.pop_back();
        allocChunk_ = deallocChunk_ = &chunks_.front();
      }
      return ;
    }
    if(lastChunk.blocksAvailable_ == numBlocks_)
    {
      //一个deallocChunk_,一个lastChunk都为空
      //回收lastChunk
      //以后继续从deallocChunk上分配内存
      lastChunk.Release();
      chunks.pop_back();
      allocChunk_ = deallocChunk_;
    }
    else
    {
      //lastChunk不为空
      //将free chunk与最后一个chunk对调
      //从lastChunk上分配内存
      //交换之前，当前deallocChunk_管理的内存，释放了没有？如果不释放，什么时候释放？
      //如果交给上面的尾部释放策略，是否永远不可能释放了？
      //deallocChunk_.Release();
      std::swap(*deallocChunk_,lastChunk);
      allocChunk_ = &chunks_.back();
    }
  }
}
```

### Loki allocator总结
- 精简强悍，手段简单粗暴
- 以数组取代list，以index取代pointer
- 能够简单的判断chunk全回收，之后将内存段归还给系统
- 设计了Deferring（暂缓归还）能力
- allocator用来分配大量小块不带cookie的memory blocks，它的最佳客户是容器，但是它本身却用vector实现,而vecotor应用的是标准库的std::alloc

## 额外allocator分析
- 所有的allocator都由base_allocator进行处理
- 对super class的操作进行封装,super class普通用户无法直接访问调用
- subclass allocator只拥有typedef，constructor和rebind等成员
### 最简单的方案new_allocator&malloc_allocator
- 每当容器有内存需求就调用operator new，每当容器释放内存就调用operator delete。效率相对内存池较低，但是可以在大多数操作系统和硬件上正常发挥作用。
- __gnu_cxx::new_allocator
  - 实现出最简朴的operator new和operator delete语义。即直接调用全局的operator new和operator delete
- __gun_cxx:malloc_allocator
  - 它封装了std::malloc和std::free

### 智能型七大allocator
- 也就是内存池实现，减少从系统中分配内存的调用次数，由自己管理从操作系统获取的内存，来给容器进行内存块上的分配。
- 可以是bitmap index实现，用以索引至一个以2的指数倍成长的buckets
- 较之简单的实现是fixed-size pooling cache.
- __gnu_cxx::bitmap_allocator
  - 一个高效能的allocator，使用bit-map追踪被使用和未被使用的内存块
- __gnu_cxx::pool_allocator
  - 内存池设计逻辑的分配器
- __gnu_cxx::__mt_alloc
- __gnu_cxx::debug_allocator
  - 这是一个wrapper，可以修饰任何一个allocator。它把用户的申请量添加一些，然后由allocator给回内存块，并用多申请的一些内存，来放置size信息。一旦deallocate()收到一个pointer，就检查size并且使用assert保证吻合。
- __gnu_cxx::array_allocator
  - 允许分配一个已知且固定大小的内存块，内存来自std::array objects.使用上这个allocator，大小固定的容器（包括std::string）就无需再调用::operator new和::operator delete。这就允许我们使用STL abstractions而无需在运行时添乱、增加开销。甚至在program startup情况下也可以使用。
  - 静态数组分配,在还没进入main之前，就可以使用，全局静态变量

### VS2013标准分配器与new_allocator
- 最基础的分配器，本质上只是一个封装好的::operator new和::operator delete
- 继承自_Allocator_base
### G4.9标准分配器与new_allocator
- 最基础的分配器，本质上只是一个封装好的::operator new和::operator delete
- 继承自__allocator_base
- __allocator_base是一个define，其本质是__gnu_cxx::new_allocator
### G4.9 malloc_allocator
- 本质上是对std::malloc和std::free的一层封装

### G4.9 bitmap_allocator
#### allocate
- 只对需求一块内存块的用户提供服务，如果一次申请多个内存块，那么交由::operator new和::operator delete处理.
- 直接使用分配器才会一次申请多块内存
- blocks:用户申请的内存块,第一次向系统要64个，之后128,256..
- super-blocks:内存段，每次向系统申请的内存块大小，可以切分成blocks
- bitmap:在链表的头部，用于记录当前内存块的分配情况，使用bits位记录.本身是unsigned int，一个数值记录32个blocks，即在第一次向系统申请64个blocks时，需要两个bitmap来记录其使用情况.1表示在手中，0标示给出去.
- use_count:记录当前blocks由多少块被分配出去了.
- super_block_size:记录super_block_size.如果block_size = 8 bytes,那么super_block_size = 4(use_count) + 2 * 4(bitmap*(blocks/32)) + 64 * 8(blocks*block_size) = 524 bytes
- mini-vector
  - 管理一堆单元块，如果有多个管理的链表，那么就存在着多个单元块，每个单元块各自指向自己管理的super_blocks,自身会自动成长的
  - _M_start:指向一堆单元块的头部
  - _M_finish:指向一堆单元块的尾部元素的下一个位置
  - _M_end_of_storage:容器的尾部单元块的下一个位置
  - 单元块
    - 单元块头部元素指向super_blocks头部元素
    - 单元块尾部元素指向super_blocks尾部元素
- 若不曾全回收，则分配规模会不断倍增，相当惊人。每次全回收造成下一次分配规模减半
- mini-vector的entries无限制，每个entry代表一种value type，不同的value type即使大小相同也不混用,每一个mini-vector的单元块只管理一种类型的元素
- 分配内存区块时，优先从未曾回收区块的super-blocks#1上取，当#1分配完之后，再从已经回收过区块的super-blocks#2上分配内存块。

#### deallocate
- 有mini-vector#2，标记全回收的super-blocks,同时将mini-vector#1上的entry给删除,调用erase动作将vector元素往左推,但是vector自身不会缩小
- 全回收之时，下一次分配规模减半
- mini-vector#2最多管理64个super-blocks，超过64个则立即归还给操作系统.如果新全回收的super-blocks管理的内存块，都大于当前管理的super-blocks，那么直接交回给操作系统，如果小于64个blocks，无条件插入mini-vector#2

## 成员函数const
- 这种修饰的话，不能修改类成员变量
- 本质是对*this指针加常量性，不能修改*this指针指向的值
- const属于签名，加和不加会导致函数重载
- 其本质是调用函数参数的时候，*this也作为一个参数传入,*this多加一个const那么会导致参数类型发生变化
- 当const版本和non-const版本同时存在，const object只能调用const member function，non-const object只能调用non-const member object