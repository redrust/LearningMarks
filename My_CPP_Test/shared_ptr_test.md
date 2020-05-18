## 测试１：shared_ptr和bind导致shared_ptr对象生命周期延长的问题,以及placement new与shared_ptr对象的结合性测试

```cpp
//测试对象
class A:public std::enable_shared_from_this<A>
{
public:
    void test()
    {
        //use_count-1本身是为了减少临时shared对象带来的干扰
        std::cout << "test use_count:" << shared_from_this().use_count() - 1<< std::endl;
    }
};

//测试接口函数，参数为bind对象
void threadFunc(std::function<void()> t)
{
    t();
}

int main()
{
    char* mem = new char[1024];
    std::thread t;
    std::weak_ptr<A> wa;
    {
        //placement new和shared_ptr对象结合，需要自定义Deleter，否则会析构mem内存，而且析构不成功，导致程序core dump的问题
        std::shared_ptr<A> a(new(mem)A(),[](A* a){ a->~A();});
        //函数绑定对象,内部会进行隐性拷贝一次，延长了shared_ptr对象的生命周期
        t = std::thread(threadFunc,std::bind(&A::test,a));
        wa = a;
    }
    //主线程，延时和不延时会出两种结果
    sleep(1);
    //测试shared_ptr对象是否活着
    if(wa.lock())
    {
        std::cout << "shared_ptr a is alive." << std::endl;
        std::cout << wa.use_count() << std::endl;
    }
    if(t.joinable())
    {
        t.join();
    }
    //线程结束之后，再次进行测试
    if(wa.lock())
    {
        std::cout << "shared_ptr a is alive." << std::endl;
        std::cout << wa.use_count() << std::endl;
    }
    delete mem;
    return 0;
}
```
- ### 输出结果１  
```cpp
shared_ptr a is alive.
1
test use_count:1
```
- 输出结果１思考：如果并发运行程序，那么在主线程申请的shared_ptr对象，bind到子线程处理，会导致shared_ptr对象生命周期延长，生命周期延长到子线程处理完毕，释放目标对象为止．哪怕目标bind的函数参数，本身是const references的，也会导致一次隐性的拷贝操作．
- 如果目标对象长期持有bind的函数对象的话，那么shared_ptr的生命周期就不可控了．
- ### 输出结果２
```cpp
test use_count:1
```
- 输出结果２思考：主线程主动sleep一段时间．等待子线程处理完毕返回之后，再继续执行，则可以将shared_ptr对象生命周期给控制住，但是sleep是糟糕的设计．任何一个线程都是重要的资源，不应该为了同步而主动睡眠，如果用互斥锁则略显小题大作．总的来说，还是尽量避免bind导致shared_ptr对象生命周期延长的问题．
- 像上面的设计，使用一个内存池来管理智能指针对象生命周期会好很多，可以保证智能指针对象生命周期在可控制的范围内．