## CPP内存管理入门初探
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

{
    //case 3	
    //掌握崩潰原因, 再次模擬 memory pool作法, array new + placement new. 	
    //不, 不做了, 因為 memory pool 只是供應 memory, 它並不管 construction, 
    //也不管 destruction. 它只負責回收 memory. 
    //所以它是以 void* 或 char* 取得 memory, 釋放 (刪除)的也是 void* or char*.  
    //不像本例 case 1 釋放 (刪除) 的是 A*. 
    //
    //事實上 memory pool 形式如 jj04::test 
}

}	
```
### placement new