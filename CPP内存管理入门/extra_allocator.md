## 额外allocator分析
- 所有的allocator都由base_allocator进行处理
- 对super class的操作进行封装,super class普通用户无法直接访问调用
- subclass allocator只拥有typedef,constructor和rebind等成员

### 最简单的方案new_allocator&malloc_allocator
- 每当容器有内存需求就调用operator new,每当容器释放内存就调用operator delete。效率相对内存池较低,但是可以在大多数操作系统和硬件上正常发挥作用。
- __gnu_cxx::new_allocator
  - 实现出最简朴的operator new和operator delete语义.即直接调用全局的operator new和operator delete
- __gun_cxx:malloc_allocator
  - 它封装了std::malloc和std::free

### 智能型七大allocator
- 也就是内存池实现,减少从系统中分配内存的调用次数,由自己管理从操作系统获取的内存,来给容器进行内存块上的分配.
- 可以是bitmap index实现,用以索引至一个以2的指数倍成长的buckets
- 较之简单的实现是fixed-size pooling cache.
- __gnu_cxx::bitmap_allocator
  - 一个高效能的allocator,使用bit-map追踪被使用和未被使用的内存块
- __gnu_cxx::pool_allocator
  - 内存池设计逻辑的分配器
- __gnu_cxx::__mt_alloc
- __gnu_cxx::debug_allocator
  - 这是一个wrapper,可以修饰任何一个allocator.它把用户的申请量添加一些,然后由allocator给回内存块,并用多申请的一些内存,来放置size信息.一旦deallocate()收到一个pointer,就检查size并且使用assert保证吻合.
- __gnu_cxx::array_allocator
  - 允许分配一个已知且固定大小的内存块,内存来自std::array objects.使用上这个allocator,大小固定的容器（包括std::string）就无需再调用::operator new和::operator delete.这就允许我们使用STL abstractions而无需在运行时添乱、增加开销.甚至在program startup情况下也可以使用.
  - 静态数组分配,在还没进入main之前,就可以使用,全局静态变量

### VS2013标准分配器与new_allocator
- 最基础的分配器,本质上只是一个封装好的::operator new和::operator delete
- 继承自_Allocator_base
### G4.9标准分配器与new_allocator
- 最基础的分配器,本质上只是一个封装好的::operator new和::operator delete
- 继承自__allocator_base
- __allocator_base是一个define,其本质是__gnu_cxx::new_allocator
### G4.9 malloc_allocator
- 本质上是对std::malloc和std::free的一层封装

### G4.9 bitmap_allocator
#### allocate
- 只对需求一块内存块的用户提供服务,如果一次申请多个内存块,那么交由::operator new和::operator delete处理.
- 直接使用分配器才会一次申请多块内存
- blocks:用户申请的内存块,第一次向系统要64个,之后128,256..
- super-blocks:内存段,每次向系统申请的内存块大小,可以切分成blocks
- bitmap:在链表的头部,用于记录当前内存块的分配情况,使用bits位记录.本身是unsigned int,一个数值记录32个blocks,即在第一次向系统申请64个blocks时,需要两个bitmap来记录其使用情况.1表示在手中,0标示给出去.
- use_count:记录当前blocks由多少块被分配出去了.
- super_block_size:记录super_block_size.如果block_size = 8 bytes,那么super_block_size = 4(use_count) + 2 * 4(bitmap*(blocks/32)) + 64 * 8(blocks*block_size) = 524 bytes
- mini-vector
  - 管理一堆单元块,如果有多个管理的链表,那么就存在着多个单元块,每个单元块各自指向自己管理的super_blocks,自身会自动成长的
  - _M_start:指向一堆单元块的头部
  - _M_finish:指向一堆单元块的尾部元素的下一个位置
  - _M_end_of_storage:容器的尾部单元块的下一个位置
  - 单元块
    - 单元块头部元素指向super_blocks头部元素
    - 单元块尾部元素指向super_blocks尾部元素
- 若不曾全回收,则分配规模会不断倍增,相当惊人.每次全回收造成下一次分配规模减半
- mini-vector的entries无限制,每个entry代表一种value type,不同的value type即使大小相同也不混用,每一个mini-vector的单元块只管理一种类型的元素
- 分配内存区块时,优先从未曾回收区块的super-blocks#1上取,当#1分配完之后,再从已经回收过区块的super-blocks#2上分配内存块.

#### deallocate
- 有mini-vector#2,标记全回收的super-blocks,同时将mini-vector#1上的entry给删除,调用erase动作将vector元素往左推,但是vector自身不会缩小
- 全回收之时,下一次分配规模减半
- mini-vector#2最多管理64个super-blocks,超过64个则立即归还给操作系统.如果新全回收的super-blocks管理的内存块,都大于当前管理的super-blocks,那么直接交回给操作系统,如果小于64个blocks,无条件插入mini-vector#2

## 成员函数const
- 这种修饰的话,不能修改类成员变量
- 本质是对*this指针加常量性,不能修改*this指针指向的值
- const属于签名,加和不加会导致函数重载
- 其本质是调用函数参数的时候,*this也作为一个参数传入,*this多加一个const那么会导致参数类型发生变化
- 当const版本和non-const版本同时存在,const object只能调用const member function,non-const object只能调用non-const member object