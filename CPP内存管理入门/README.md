## CPP内存管理入门初探
### 代码内容大部分引用至侯捷老师
- primitives:基础内存管理手法
- 想要直接调用类构造函数，需要使用placement new
```cpp
//cpp基础内存分配手段测试
void primitives()
{
    //c风格的malloc和free
    void* p1 = malloc(512);	//512 bytes
    free(p1);

    //cpp风格的new和delete
    //new套了一层operator new，而operator new的本质是包装好的malloc
    //如果申请一堆内存，那么new会比malloc多记录一些额外信息，比如一堆内存的大小，方便delete使用
    //delete套了一层operator delete，而operator delete的本质是包装好的free
    //从上述讨论可以看到，如果类里面重载了new操作符，那么当使用new操作符取申请对象时，
    //就会调用到客户重载的new操作符版本了
    complex<int>* p2 = new complex<int>; //one object
    delete p2;             

    void* p3 = ::operator new(512); //512 bytes
    ::operator delete(p3);

    //使用内存分配器进行分配内存
    //一般不使用这个版本，对参数和值需要程序员自己保证，但是在项目中其实很难保证这个事情
    //目前的分配器逻辑，都是生成临时对象，然后分配请求的内存量
    //其本质是操作系统管理的内存池，生成临时子对象，从内存池中申请内存块
    //以下使用 C++ 標準庫提供的 allocators。
    //其接口雖有標準規格，但實現廠商並未完全遵守；下面三者形式略異。
#ifdef _MSC_VER
    //以下兩函數都是 non-static，定要通過 object 調用。以下分配 3 個 ints.
    int* p4 = allocator<int>().allocate(3, (int*)0); 
    allocator<int>().deallocate(p4,3);           
#endif
#ifdef __BORLANDC__
    //以下兩函數都是 non-static，定要通過 object 調用。以下分配 5 個 ints.
    int* p4 = allocator<int>().allocate(5);  
    allocator<int>().deallocate(p4,5);       
#endif
#ifdef __GNUC__
    //以下兩函數都是 static，可通過全名調用之。以下分配 512 bytes.
    //void* p4 = alloc::allocate(512); 
    //alloc::deallocate(p4,512);   
    
    //以下兩函數都是 non-static，定要通過 object 調用。以下分配 7 個 ints.    
	void* p4 = allocator<int>().allocate(7); 
    allocator<int>().deallocate((int*)p4,7);     
	
    //以下兩函數都是 non-static，定要通過 object 調用。以下分配 9 個 ints.	
	void* p5 = __gnu_cxx::__pool_alloc<int>().allocate(9); 
    __gnu_cxx::__pool_alloc<int>().deallocate((int*)p5,9);	
#endif
}
```

### array new,array delete
- 会根据new出对象的大小，决定调用多少次ctor。
- 只调用默认构造函数，如果试图申请一堆类对象，那么目标对象一定要实现默认构造函数。
- 同样，在new的时候调用了多少次ctor，那么在delete时就会调用多少次dtor。
  - 如果array new匹配了普通的delete，那么编译器就只会调用一次dctor。
  - 内存泄露只针对目标对象本身内部持有指针指向堆内存，这种行为导致了内存泄露的后果。
  - 如果目标对象内部只有标准类型，那么行为未定义。
- 而且无法使用参数给予其默认初值。
- array new和new operator在内存块上的布局是不同的
  - new operator：在内存块上布局申请的对象所需要的字节数。
  - array new：在实际存对象的内存之前，还布局有一个整数数值，表明当前申请的对象的数量。
  - 如果不匹配，就会出现内存块匹配错误，那么就会导致core dump，因为内存块上的信息不匹配。
```cpp
class A
{
public:
  int id;
  
  A() : id(0)      { cout << "default ctor. this="  << this << " id=" << id << endl;  }
  A(int i) : id(i) { cout << "ctor. this="  << this << " id=" << id << endl;  }
  ~A()             { cout << "dtor. this="  << this << " id=" << id << endl;  }
};
void test_array_new_and_placement_new()
{
	cout << "\ntest_placement_new().......... \n";	
	
size_t size = 3;
	
{
    //case 1
    //模擬 memory pool 的作法, array new + placement new. 崩潰 

   	A* buf = (A*)(new char[sizeof(A)*size]);
   	A* tmp = buf;   
	   
	cout << "buf=" << buf << "  tmp=" << tmp << endl;	
   	
   	for(int i = 0; i < size; ++i)
	    new (tmp++) A(i);  			//3次 ctor 

	cout << "buf=" << buf << "  tmp=" << tmp << endl;
		    
    //!	delete [] buf;    	//crash. why?
    //因為這其實是個 char array，看到 delete [] buf; 編譯器會企圖喚起多次 A::~A. 
    // 但 array memory layout 中找不到與 array 元素個數 (本例 3) 相關的信息, 
    // -- 整個格局都錯亂 (從我對 VC 的認識而言)，於是崩潰。 
	delete buf;     	//dtor just one time, ~[0]	

	cout << "\n\n";
}

{
    //case 2
    //回頭測試單純的 array new
	
   	A* buf = new A[size];  //default ctor 3 次. [0]先於[1]先於[2])
	         //A必須有 default ctor, 否則 [Error] no matching function for call to 'jj02::A::A()'
   	A* tmp = buf;   
	   
	cout << "buf=" << buf << "  tmp=" << tmp << endl;	
   	
   	for(int i = 0; i < size; ++i)
	    new (tmp++) A(i);  		//3次 ctor 

	cout << "buf=" << buf << "  tmp=" << tmp << endl;
		    
	delete [] buf;    //dtor three times (次序逆反, [2]先於[1]先於[0])	
}

}	
```
### placement new
- placement new允许我们将object构建于allocated memory中，在指定位置构建对象。
- 没有所谓的placement delete，因为placement new根本没有分配memory，也可以说与placement new对应的operator delete为palcement delete。
```cpp
//placement new典型用法
char* buf = new char[1024];
Object* object = new (buf) Object();
```
## cpp典型内存分配过程
- 在这个过程中，套了一层operator new，方便重载的new自行调用。
```cpp
Foo* p =  new Foo(x);
delete p;
//is equvalent to
Foo* p = (Foo*)malloc(sizeof(Foo));
new (p)Foo(x);
p->~Foo();
free(p);
```
## 重载::operator new / ::operator delete
- 注意：这个是全局操作符，重载影响到当前程序的方方面面，如果不是特别需求，不推荐重载。
```cpp
inline void* operator new(size_t size)
{//do something
}
inline void* operator new[](size_t size)
{//do something
}
inline void operator delete(void* ptr)
{//do something
}
inline void operator delete[](void* ptr)
{//do something
}
```
## 重载operator new
```cpp
class Foo{
public:
    void* operator new(size_t);
    //size_t是可选的参数，可写可不写
    void operator delete(void*,size_t);
};

Foo* p = new Foo;
//===>
void* mem = operator new(sizeof(Foo));
p = static_cast<Foo*>(mem);
p->Foo::Foo(1,2);

delete p;
//===>
p->~Foo();
operator delete(p);
```
## 强制使用global scope operator
```cpp
Foo* p = ::new Foo();
::delete p;
```
## placement new
- 可以重载class member operator new()，写出多个版本，前提是每个版本必须有独特的参数列，其中第一个参数必须是size_t，其余参数以new所指定的placement arguments为初值。
- 也可以重载class member operator delete()，写出多个版本，但是它们绝对不会被delete调用。只有当new所调用的ctor抛出exception，才会调用重载的operator delete()。之所以会这样设计，主要用来归还未能完全创建成功的object所占用的memory。
```cpp
class Foo{
public:
    //1 一般的operator new()的重载
    void* operator new(size_t size){
        return malloc(size);
    }
    //2 自定义参数个数的operator new()的重载
    void* operator new(size_t size,long extra){
        return malloc(size + extra);
    }
    //3 一般意义上的operator delete(),对应上面的1
    void operator delete(void*,size_t)
    {}
    //4 对应上面的2重载operator new()
    void operator delete(void*,long)
    {} 
};

```

## static allocator
- 将一个总是分配特定尺寸区块的memory allocator给封装起来，使其可以被重复使用。
```cpp
class allocator
{
private:
    struct obj{
        struct obj* next;   //embedded pointer
    };
public:
    void* allocate(size_t);
    void deallocate(void*,size_t);
private:
    obj* freeStore = nullptr;
    const int CHUNK = 5;
};
void* allocator::allocate(size_t size)
{
    obj* p;
    if(!freeStore)
    {
        //linklist为空，于是申请一大块内存
        size_t chunk = CHUNK * size;
        freeStore = p = (obj*)malloc(chunk);

        //将分配得到的一大块内存，分成一块块小的linked list并且将它们串起来
        for(int i = 0;i < (CHUNK - 1) ; ++i)
        {
            p->next = (obj*)((char*)p + size);
            p = p->next;
        }
        p->next = nullptr;
    }
    p = freeStore;
    freeStore = freeStore->next;
    return p;
}
void allocator::deallocate(void* p,size_t)
{
    ((obj*)p)->next = freeStore;
    freeStore = (obj*)p;
}

class Foo
{
public:
    long L;
    string str;
    static allocator myAlloc;
public:
    Foo(long l):L(l){}
    static void* operator new(size_t size)
    {
        return myAlloc.allocate(size);
    }
    static void operator delete(void* pdead,size_t size)
    {
        return myAlloc.dealloc(pdead,size);
    }
};
allocatot Foo::myAlloc;
```

## new handler
- 当operator new没能申请分配到新的内存空间时，会抛出一个std::bad_alloc exception。
- 在抛出exception之前会先（不止一次的）调用一个可由client指定的handler
    - 以下是new handler的形式和设定方法
    ```cpp
    typedef void(*new_handler)();
    new_handler set_new_handler(new_handler p)throw();
    ```
    - 设计良好的new handler应该具备以下两个特性
      - 让更多的memory可用
      - 调用abort()或exit()

## VC6 malloc()
- 存在着巨大的内存浪费
- VC6、BC5和G2.9的allocator以及G4.9的new_allocator都只是以::operator new和::operator delete完成allocate()和deallocate()，并没有特殊设计，使其适配各大容器。
- ![](VC6内存块布局.png)

## std::alloc
- 由free_list[16]和free_list每个元素所指向的单向链表组成。本质上是自己指向自己，自己管理自己。
- 由alloc::allocate()和alloc::deallocate()进行申请和归还。
- 每次申请内存时，都会检查free_list上对应大小的位置，有没有空的还未使用的内存，如果没有，那么就申请当前需要大小的字节的20*2的内存块，比如如果申请96字节内存块大小，那么就申请96*20*2=3840字节的内存，其中20块96字节的内存直接挂在对应位置上，剩下的一半内存放回内存池内以供日后使用。
- free_list自己管理有一个内存池，每次分配之前都会检查pool的空闲内存是否够分配，如果够就取一部分下来挂到free_list对应位置上，如果不够就用malloc从系统申请新的内存挂到内存池上，不够的情况指的是连一个申请对象大小的块都分配不出来。在pool余量，大于等于一个申请对象大小的情况下，都尽可能的分配出去。
- 新申请的内存块大小，遵循所需对象内存大小*20*2+RoundUp(已经申请内存大小>>4)，RoundUp（）是将除以16的额外大小进行调整到8的倍数的一个接口，方便内存对其设计。
- 从pool中切割出来准备挂上free_list的区块，数量永远在1~20之间。
- 当所申请的内存，在pool里连一个对象都分配不出来，但是存在内存碎块的时候，除了直接malloc一块新的内存块予以分配之外，还需要对剩下的内存碎块进行特殊处理，将剩下的内存块大小直接挂到free_list对应位置上，交由对应指针进行管理。因为申请的时候就进行了字节对其处理，所以不存在会落空、没有合适存放位置的内存块。
- 如果malloc分配失败之后，就会从申请内存块大小最近的free_list位置上去找，是否有空闲的、还未分配给客户使用的内存，如果有，就取下来，放回pool中，再进行分配给新的free_list[i]进行管理。它会一直扫描下去，直到扫描完整个free_list.
- 如果malloc分配失败，会一直折半，直到拿到内存为止。

### embedded pointers
- 嵌入式指针，在分配的每一个内存块上，内含有一个4字节指针，方便alloc对内存块进行链表链接管理，但是给用户之后，该指针会被覆盖，直到alloc重新接管该内存块，才重新对该指针进行置值。
```cpp
//可以表现为如下形式
struct obj{
    struct obj* free_list_link;
};
```

### std::alloc源码分析
```cpp
#define __THROW_BAD_ALLOC  cerr << "out of memory" << endl; exit(1)
//----------------------------------------------
// 第1級配置器。
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
    (*my_malloc_handler)();    //呼叫處理常式，企圖釋放記憶體
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
    (*my_malloc_handler)();    //呼叫處理常式，企圖釋放記憶體。
    result = realloc(p, n);    //再次嘗試配置記憶體。
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

  //若bytes为13,则（13+7）&～（7），即10110&11000得10000即16
  //上取整，保证是8的倍数
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
  //计算是由哪个链表进行管理的，找到链表上对应位置的指针
  static size_t FREELIST_INDEX(size_t bytes) {
    return (((bytes) + __ALIGN-1)/__ALIGN - 1);
  }

  // Returns an object of size n, and optionally adds to size n free list.
  //充值，从内存池中拿一块内存给客户使用，同时把同样大小的内存块链接到链表上，提供给以后使用
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
    //指针的指针，要去找到free_list的指针上管理的链表的元素
    obj* volatile *my_free_list;    //obj** my_free_list;
    obj* result;

    //转去调用第一级分配器
    //也就是malloc
    if (n > (size_t)__MAX_BYTES) {
        return(malloc_alloc::allocate(n));
    }

    //找到应该由第几号链表进行管理
    my_free_list = free_list + FREELIST_INDEX(n);
    //找到对应链表之后，检查一下是否为空
    result = *my_free_list;
    if (result == 0) {
        //如果为空，就从内存池拿一块下来，充值到对应链表上
        void* r = refill(ROUND_UP(n));
        return r;
    }
    //如果不为空，则调整指向，free_list指向下一块空闲区块
    //返回原位置的result给用户使用
    *my_free_list = result->free_list_link;
    return (result);
  }

  //如果传入的p并不是从alloc取得，会带来未定义行为
  //如果p的大小不是8的倍数，会带来毁灭性的灾难
  static void deallocate(void *p, size_t n)  //p may not be 0
  {
    //暂存目的指针
    obj* q = (obj*)p;
    //指向指针的指针，进行修改链表的指向
    obj* volatile *my_free_list;   //obj** my_free_list;

    //如果超过大小，那么交由一级分配器进行处理
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
//找到第n号链表之后，发现对应链表是空的，于是调用这个接口，获取一大块内存
//其中一半插入到链表上，给链表进行管理，剩下一半放入内存池中
template <bool threads, int inst>
char*
__default_alloc_template<threads, inst>::
chunk_alloc(size_t size, int& nobjs)
{
  char* result;
  //一次申请中，总计需要的内存大小
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
  } else {//连一块都无法满足，剩下来的内存碎块处理
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
          //这段注释是针对别的程序，而不是我们写的程序
          for (i = size; i <= __MAX_BYTES; i += __ALIGN) {
              //从申请内存块的大小，向链表右端不断搜索,直到搜索到有空闲块的链表
              my_free_list = free_list + FREELIST_INDEX(i);
              p = *my_free_list;
              if (0 != p) {//如果该链表内有可用区块，拿出一块给内存池使用
                  *my_free_list = p -> free_list_link;
                  start_free = (char*)p;
                  end_free = start_free + i;
                  //递归调用检查内存池
                  return(chunk_alloc(size, nobjs));
                  //Any leftover piece will eventually make it to the
                  //right free list.
              }
          }
          //至此，memory已经山穷水尽
          end_free = 0;       //In case of exception.
          //尝试调用第一级分配器，看看能不能拿到一点什么
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

    //检查拿到了几个区块，如果只有一个直接返回给客户
    if (1 == nobjs) return(chunk);
    //如果有很多个，那么需要将其挂到free_list上
    my_free_list = free_list + FREELIST_INDEX(n);

    //Build free list in chunk
    //在chunk内建fre_list链表结构
    result = (obj*)chunk;
    *my_free_list = next_obj = (obj*)(chunk + n);
    //第一个要给回用户，所以从1开始
    for (i=1;  ; ++i) {
      current_obj = next_obj;
      //先转型为char进行字节跳转，再转型成obj再链接起来
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
//返回一个函数指针，而且参数是一个函数指针的接口
static H set_malloc_handler(H f);

//易混点3
//deallcate()完全没有调用free() or delete
```

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