## loki::allocator
- 三个对象,由上至下进行管理
  ```cpp
  class Chunk{
    //当前管理的一内存段的头部指针
    unsigned char* pData_;
    //首块空闲的内存块的索引
    //用来快速分配可用内存块
    unsigned char firstAvailableBlock_;
    //当前这内存段中,可以切割出多少块小的内存块
    //也就是内存块的数量
    unsigned char blocksAvailable_;
  };
  class FixedAllocator{
    //管理一堆Chunk,可以看成是指针的指针
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

//初始化申请的内存段,将pData_切割成内存块,并给在内存块首部标上序号
//在操作结果上,可以近似的看成,将pData_申请的内存段,给切成一块块连续的内存块,同时在每个小块的头部打上索引号
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
//三层内存管理结构中,最低的一层,负责直接从操作系统中申请内存并进行管理
void* FixedAllocator::Chunk::Allocate(std::size_t blockSize)
{
  //当前内存段没有可用内存块
  if(!blocksAvailable_)
    return 0;
  //找到空闲区块
  //直接计算其偏移值
  unsigned char* pResult = pData_ + (firstAvailableBlock_ * blockSize);
  //修改空闲区块索引值
  //找到当前分配出去的内存块的头部索引值,因为它记录着下一块空闲内存块的地址,并将它保存下来
  firstAvailableBlock_ = *pResult;
  //减少空闲区块数量
  --blocksAvailable_;
  return pResult;
}

//Chunk回收管理内存的接口
void FixedAllocator::Chunk::Deallocate(void* p,std::size_t blockSize)
{
  unsigned char* toRelease = static_cast<unsigned char*>(p);
  //给当前给回内存块,打上索引值,此索引值是下一块空闲内存的索引值
  *toRelease = firstAvailableBlock_;
  //当前指针属于哪一块索引的,只需要将当前指针-头部指针的地址,再除以内存块大小,即可得出索引值
  //此索引值是记录的Chunk中的,最高优先级的索引区块值
  //每次分配时,会第一个分配出去,同时需要读取该内存块的上的索引值,该内存块上的索引值指向下一块空闲内存
  firstAvailableBlock_ = static_cast<unsigned char>((toRelease - pData_)/blockSize);
  //增加空闲区块数量
  ++blocksAvailable_;
}
```

### FixedAllocator实现细节
```cpp
//FixedAllocator内存分配
//本质上是从自身管理的Chunk列表中寻找空闲的Chunk,来进行内存分配
void* FixedAllocator::Allocate()
{
  //目前没有标定chunk或者该chunk已经没有可用区块
  if(allocChunk_ == 0 || allocChunk_->blocksAvailable_ == 0)
  {
    //从头找起
    Chunks::iterator i = chunks_.begin();
    for(;;++i)
    {
      //到达尾端,都没找到
      if(i == chunks_.end())
      {
        //重新调整vector的大小
        chunks_.reserve(chunks_.size() + 1);
        //手动初始化Chunk
        Chunk newChunk;
        newChunk.Init(blockSize_, numBlocks_);
        //之后再插入到vector中
        chunks_.push_back(newChunk);
        //allocChunk指向新的Chunk
        //当到这个情况时,可以默认前面的Chunk都已经被使用完了
        allocChunk_ = &chunks_.back();
        //deallocChunk_指向vector的首部元素
        //也就是指向最早分配完的Chunk
        //因为其最有可能会回收内存块来提供内存分配
        deallocChunk_ = &chunks_.front();
        break;
      }
      if(i->blocksAvailable_ > 0)
      {
        //寻找chunk的过程中找到一个chunk有可用内存块
        //因为是iterator对象,所以需要先解引用取其值,再取其地址
        allocChunk_ = &*i;
        break;
      }
    }
  }
  //向这个chunk取内存块
  //下次再申请分配内存的话,直接从现在这个块上找起
  return allocChunk_->Allocate(blockSize_);
}

//FixedAllocator内存释放
void FixedAllocator::Deallocate(void* p)
{
  deallocChunk_ = VicinityFind(p);
  DoDeallocate(p);
}

//临近查找
//如果传入的指针不是本系统分配的,会找不到对应的chunk,那么会出现死循环
//判断传入指针的地址,是否在管理的内存区块段内。只需要判断头地址和尾地址即可
//于是弄出了头尾一起遍历的特殊设计
//直觉上比一半的遍历效率高,最坏情况下在O(n)
FixedAllocator::Chunk* FixedAllocator::VicinityFind(void* p)
{
  //一个内存段的大小
  const std::size_t chunkLength = numBlocks_ * blockSize_;
  //当前准备释放的内存段的头地址
  Chunk* lo = deallocChunk_;
  //当前准备释放的内存段的尾地址
  Chunk* hi = deallocChunk_ + 1;
  //当前管理的内存段的首地址,也就是容器中第一个内存段的头地址
  Chunk* loBound = &chunks.front();
  //当前管理的内存段的尾地址,也就是容器中最后一个元素再偏移一个元素的头地址
  Chunk* hiBound = &chunks_.back() + 1;

  //如果其恰好是最后一块,那么在下面的搜索中,hi将会访问非法内存从而导致崩溃
	if (hi == hiBound) hi = 0;
  
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
        //lo往上寻找,所以自减
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
      //hi往下寻找,所以自增
      if(++hi == hiBound)
      {
        hi = 0;
      }
    }
  }
  return nullptr;
}

//此函数处理Chunk的情况,尽量保证空Chunk,也就是还未分配内存出去给客户的Chunk,尽量都在vector的末尾
//减少每次分配时查找的时间
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
      //有两个free chunk,抛弃最后一个
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
      //deallocChunk_.Release();
      std::swap(*deallocChunk_,lastChunk);
      allocChunk_ = &chunks_.back();
    }
  }
}
```

### Loki allocator总结
- 精简强悍,手段简单粗暴
- 以数组取代list,以index取代pointer
- 能够简单的判断chunk全回收,之后将内存段归还给系统
- 设计了Deferring（暂缓归还）能力
- allocator用来分配大量小块不带cookie的memory blocks,它的最佳客户是容器,但是它本身却用vector实现,而vecotor应用的是标准库的std::alloc
