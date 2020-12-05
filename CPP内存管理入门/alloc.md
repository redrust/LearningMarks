## std::alloc
- 由free_list[16]和free_list每个元素所指向的单向链表组成。本质上是自己指向自己,自己管理自己。
- 由alloc::allocate()和alloc::deallocate()进行申请和归还.
- 每次申请内存时,都会检查free_list上对应大小的位置,有没有空的还未使用的内存,如果没有,那么就申请当前需要大小的字节的20*2的内存块,比如如果申请96字节内存块大小,那么就申请96*20*2=3840字节的内存,其中20块96字节的内存直接挂在对应位置上,剩下的一半内存放回内存池内以供日后使用.
- free_list自己管理有一个内存池,每次分配之前都会检查pool的空闲内存是否够分配,如果够就取一部分下来挂到free_list对应位置上,如果不够就用malloc从系统申请新的内存挂到内存池上,不够的情况指的是连一个申请对象大小的块都分配不出来.在pool余量,大于等于一个申请对象大小的情况下,都尽可能的分配出去.
- 新申请的内存块大小,遵循所需对象内存大小*20*2+RoundUp(已经申请内存大小>>4),RoundUp（）是将除以16的额外大小进行调整到8的倍数的一个接口,方便内存对其设计.
- 从pool中切割出来准备挂上free_list的区块,数量永远在1~20之间.
- 当所申请的内存,在pool里连一个对象都分配不出来,但是存在内存碎块的时候,除了直接malloc一块新的内存块予以分配之外,还需要对剩下的内存碎块进行特殊处理,将剩下的内存块大小直接挂到free_list对应位置上,交由对应指针进行管理.因为申请的时候就进行了字节对其处理,所以不存在会落空、没有合适存放位置的内存块.
- 如果malloc分配失败之后,就会从申请内存块大小最近的free_list位置上去找,是否有空闲的、还未分配给客户使用的内存,如果有,就取下来,放回pool中,再进行分配给新的free_list[i]进行管理.它会一直扫描下去,直到扫描完整个free_list.
- 如果malloc分配失败,在malloc内部将其申请内存折半,直到拿到内存为止.

### std::alloc源码分析
```cpp
#define __THROW_BAD_ALLOC  cerr << "out of memory" << endl; exit(1)
//----------------------------------------------
// 第1級配置器.
//----------------------------------------------
template <int inst>
class __malloc_alloc_template {
private:
  static void* oom_malloc(size_t);
  static void* oom_realloc(void *, size_t);
  static void (*__malloc_alloc_oom_handler)();

public:
  static void* allocate(size_t n)
  {
    void *result = malloc(n);   //直接使用 malloc()
    if (0 == result) result = oom_malloc(n);
    return result;
  }
  static void deallocate(void *p, size_t /* n */)
  {
    free(p);                    //直接使用 free()
  }
  static void* reallocate(void *p, size_t /* old_sz */, size_t new_sz)
  {
    void * result = realloc(p, new_sz); //直接使用 realloc()
    if (0 == result) result = oom_realloc(p, new_sz);
    return result;
  }
  static void (*set_malloc_handler(void (*f)()))()
  { //類似 C++ 的 set_new_handler().
    void (*old)() = __malloc_alloc_oom_handler;
    __malloc_alloc_oom_handler = f;
    return(old);
  }
};
//----------------------------------------------
template <int inst>
void (*__malloc_alloc_template<inst>::__malloc_alloc_oom_handler)() = 0;

template <int inst>
void* __malloc_alloc_template<inst>::oom_malloc(size_t n)
{
  void (*my_malloc_handler)();
  void* result;

  for (;;) {    //不斷嘗試釋放、配置、再釋放、再配置…
    my_malloc_handler = __malloc_alloc_oom_handler;
    if (0 == my_malloc_handler) { __THROW_BAD_ALLOC; }
    (*my_malloc_handler)();    //呼叫處理常式,企圖釋放記憶體
    result = malloc(n);        //再次嘗試配置記憶體
    if (result) return(result);
  }
}

template <int inst>
void * __malloc_alloc_template<inst>::oom_realloc(void *p, size_t n)
{
  void (*my_malloc_handler)();
  void* result;

  for (;;) {    //不斷嘗試釋放、配置、再釋放、再配置…
    my_malloc_handler = __malloc_alloc_oom_handler;
    if (0 == my_malloc_handler) { __THROW_BAD_ALLOC; }
    (*my_malloc_handler)();    //呼叫處理常式,企圖釋放記憶體.
    result = realloc(p, n);    //再次嘗試配置記憶體.
    if (result) return(result);
  }
}
//----------------------------------------------

typedef __malloc_alloc_template<0>  malloc_alloc;

template<class T, class Alloc>
class simple_alloc {
public:
  static T* allocate(size_t n)
    { return 0 == n? 0 : (T*)Alloc::allocate(n*sizeof(T)); }
  static T* allocate(void)
    { return (T*)Alloc::allocate(sizeof(T)); }
  static void deallocate(T* p, size_t n)
    { if (0 != n) Alloc::deallocate(p, n*sizeof(T)); }
  static void deallocate(T *p)
    { Alloc::deallocate(p, sizeof(T)); }
};
//----------------------------------------------
//第二級配置器
//----------------------------------------------
enum {__ALIGN = 8};                        //小區塊的上調邊界
enum {__MAX_BYTES = 128};                  //小區塊的上限
enum {__NFREELISTS = __MAX_BYTES/__ALIGN}; //free-lists 個數

//本例中兩個 template 參數完全沒有派上用場
template <bool threads, int inst>
class __default_alloc_template {
private:
  //實際上應使用 static const int x = N
  //取代 enum { x = N }, 但目前支援該性質的編譯器不多

  //若bytes为13,则（13+7）&～（7）,即10110&11000得10000即16
  //上取整,保证是8的倍数
  static size_t ROUND_UP(size_t bytes) {
    return (((bytes) + __ALIGN-1) & ~(__ALIGN - 1));
  }

private:
  //union改成struct亦可
  union obj {
    union obj* free_list_link;
  };

private:
  //16根指针
  static obj* volatile free_list[__NFREELISTS];
  
  //若bytes为8,则（8+7）/8-1=0
  //若bytes为16,则(15+7)/8-1=1
  //若bytes为18,则(16+7)/8-1=1
  //计算是由哪个链表进行管理的,找到链表上对应位置的指针
  static size_t FREELIST_INDEX(size_t bytes) {
    return (((bytes) + __ALIGN-1)/__ALIGN - 1);
  }

  // Returns an object of size n, and optionally adds to size n free list.
  //充值,将申请到的内存段链接到对应位置的链表上,提供给以后使用,同时摘下一块用户需求的内存块,将指针给回用户
  //会填充free_list并返回一个(其实就是第一个)区块的起始地址
  static void *refill(size_t n);

  // Allocates a chunk for nobjs of size "size".  nobjs may be reduced
  // if it is inconvenient to allocate the requested number.
  //向系统申请一大块内存
  static char *chunk_alloc(size_t size, int &nobjs);

  // Chunk allocation state.
  static char*  start_free;//指向pool的头
  static char*  end_free;//指向pool的尾
  static size_t heap_size;//分配累计量

public:

  static void * allocate(size_t n)  //n must be > 0
  {
    //指针的指针,要去找到free_list的指针上管理的链表的元素
    obj* volatile *my_free_list;    //obj** my_free_list;
    obj* result;

    //如果超过管理的大小,转去调用第一级分配器
    //也就是malloc
    if (n > (size_t)__MAX_BYTES) {
        return(malloc_alloc::allocate(n));
    }

    //找到应该由第几号链表进行管理
    my_free_list = free_list + FREELIST_INDEX(n);
    //找到对应链表之后,检查一下是否为空
    result = *my_free_list;
    if (result == 0) {
        //如果为空,就从内存池拿一块下来,充值到对应链表上
        void* r = refill(ROUND_UP(n));
        return r;
    }
    //如果不为空,则调整指向,free_list指向下一块空闲区块
    //返回原位置的result给用户使用
    *my_free_list = result->free_list_link;
    return (result);
  }

  //直接调用才会带来的问题
  //如果传入的p并不是从alloc取得,会带来未定义行为
  //如果p的大小不是8的倍数,会带来毁灭性的灾难
  static void deallocate(void *p, size_t n)  //p may not be 0
  {
    //暂存目的指针
    obj* q = (obj*)p;
    //指向指针的指针,进行修改链表的指向
    obj* volatile *my_free_list;   //obj** my_free_list;

    //如果超过大小,那么交由一级分配器进行处理
    //对应的是free
    if (n > (size_t) __MAX_BYTES) {
        malloc_alloc::deallocate(p, n);
        return;
    }
    //找到目的链表的指针
    my_free_list = free_list + FREELIST_INDEX(n);
    //将目标块下一个指针指向当前链表所指向空闲块
    q->free_list_link = *my_free_list;
    //当前链表重新指向目标块
    *my_free_list = q;
  }

  static void * reallocate(void *p, size_t old_sz, size_t new_sz);

};
//----------------------------------------------
// We allocate memory in large chunks in order to
// avoid fragmentingthe malloc heap too much.
// We assume that size is properly aligned.
// We hold the allocation lock.
//----------------------------------------------
//找到第n号链表之后,发现对应链表是空的,于是调用这个接口,获取一大块内存
//其中一半插入到链表上,给链表进行管理,剩下一半放入内存池中
template <bool threads, int inst>
char*
__default_alloc_template<threads, inst>::
chunk_alloc(size_t size, int& nobjs)
{
  char* result;
  //一次申请中,总计需要的内存大小
  size_t total_bytes = size * nobjs;
  //现在可用内存池的大小
  size_t bytes_left = end_free - start_free;

  //如果内存池里的容量大于所需
  //pool的空间足以满足20块需求
  if (bytes_left >= total_bytes) {
      result = start_free;
      //降低pool水位
      start_free += total_bytes;
      //返回申请的内存块指针
      return(result);
  } else if (bytes_left >= size) {//pool空间只满足一块（含）以上的需求
      //改变需求数
      nobjs = bytes_left / size;
      //改变需求总量
      total_bytes = size * nobjs;
      result = start_free;
      start_free += total_bytes;
      return(result);
  } else {//连一块都无法满足,剩下来的内存碎块处理
      //先准备一块内存
      size_t bytes_to_get =
                 2 * total_bytes + ROUND_UP(heap_size >> 4);
      // Try to make use of the left-over piece.
      //将碎片充分利用起来
      if (bytes_left > 0) {//如果ool还有空间
          //找出内存碎块应转移至的第n号链表位置
          obj* volatile *my_free_list =
                 free_list + FREELIST_INDEX(bytes_left);
          //将内存碎块挂上去
          ((obj*)start_free)->free_list_link = *my_free_list;
          *my_free_list = (obj*)start_free;
      }
      //重新向系统申请分配一块新的内存块
      //放入内存池中
      start_free = (char*)malloc(bytes_to_get);
      if (0 == start_free) {//如果malloc分配失败
          //找链表上空闲的链表
          int i;
          obj* volatile *my_free_list, *p;

          //Try to make do with what we have. That can't
          //hurt. We do not try smaller requests, since that tends
          //to result in disaster on multi-process machines.
          //这段注释是针对别的程序,而不是我们写的程序
          for (i = size; i <= __MAX_BYTES; i += __ALIGN) {
              //从申请内存块的大小,向链表右端不断搜索,直到搜索到有空闲块的链表
              my_free_list = free_list + FREELIST_INDEX(i);
              p = *my_free_list;
              if (0 != p) {//如果该链表内有可用区块,拿出一块给内存池使用
                  *my_free_list = p -> free_list_link;
                  start_free = (char*)p;
                  end_free = start_free + i;
                  //递归调用检查内存池
                  return(chunk_alloc(size, nobjs));
                  //Any leftover piece will eventually make it to the
                  //right free list.
              }
          }
          //至此,memory已经山穷水尽
          end_free = 0;       //In case of exception.
          //尝试调用第一级分配器,看看能不能拿到一点什么
          start_free = (char*)malloc_alloc::allocate(bytes_to_get);
          //This should either throw an exception or
          //remedy the situation. Thus we assume it
          //succeeded.
      }
      //调整已分配内存大小
      heap_size += bytes_to_get;
      //调整pool容量
      end_free = start_free + bytes_to_get;
      //递归再次检查内存池,尽可能的获取最大内存
      return(chunk_alloc(size, nobjs));
  }
}
//----------------------------------------------
// Returns an object of size n, and optionally adds
// to size n free list.We assume that n is properly aligned.
// We hold the allocation lock.
//----------------------------------------------
template <bool threads, int inst>
void* __default_alloc_template<threads, inst>::
refill(size_t n)
{   
    //默认一次抓20个块,但是不一定能拿到20个
    int nobjs = 20;
    //nobjs是pass-by-references
    char* chunk = chunk_alloc(n,nobjs);
    obj* volatile *my_free_list;   //obj** my_free_list;
    obj* result;
    obj* current_obj;
    obj* next_obj;
    int i;

    //检查拿到了几个区块,如果只有一个直接返回给客户
    if (1 == nobjs) return(chunk);
    //如果有很多个,那么需要将其挂到free_list上
    my_free_list = free_list + FREELIST_INDEX(n);

    //Build free list in chunk
    //在chunk内建fre_list链表结构
    result = (obj*)chunk;
    *my_free_list = next_obj = (obj*)(chunk + n);
    //第一个要给回用户,所以从1开始
    for (i=1;  ; ++i) {
      current_obj = next_obj;
      //先转型为char进行字节跳转,再转型成obj再链接起来
      next_obj = (obj*)((char*)next_obj + n);
      if (nobjs-1 == i) {
          current_obj->free_list_link = 0;
          break;
      } else {
          current_obj->free_list_link = next_obj;
      }
    }
    return(result);
}
//----------------------------------------------
//alloc static变量初始化
template <bool threads, int inst>
char *__default_alloc_template<threads,inst>::start_free = 0;

template <bool threads, int inst>
char *__default_alloc_template<threads,inst>::end_free = 0;

template <bool threads, int inst>
size_t __default_alloc_template<threads,inst>::heap_size = 0;

template <bool threads, int inst>
typename __default_alloc_template<threads, inst>::obj* volatile
__default_alloc_template<threads, inst>::free_list[__NFREELISTS]
     = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, };
//----------------------------------------------
//令第2級配置器的名稱為 alloc
typedef __default_alloc_template<false,0> alloc;

```

### G2.9 std::alloc易混点分析
```cpp
//易混点1,*号的结合性顺序
obj* volatile *my_free_list,*p;
//==>
obj** p1,*p2;
//==>
obj** p1;
obj* p2;

//易混点2
static void (*set_malloc_handler(void (*f)()))()
//==>
//函数指针
typedef void (*H)();
//返回一个函数指针,而且参数是一个函数指针的接口
static H set_malloc_handler(H f);

//易混点3
//deallcate()完全没有调用free() or delete
//所以向系统申请的内存,基本都还管理在alloc手上
//直到程序退出,才将内存给回系统
```
