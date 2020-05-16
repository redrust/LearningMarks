- weak_ptr和shared_ptr进行的引用计数测试。测试shared_ptr的生命周期，以及weak_ptr的能否正确的提升为对应的shared_ptr对象。
- 还增加有placement new和shared_ptr对象结合的问题，实测shared_ptr可以和placement new结合的很好，但是如果试图用这个方式来做内存池的话，可能内存池本身需要持有shared_ptr对象的引用才能很好的管理shared_ptr对象的生命周期，否则就会成为流浪在外的对象，如果内存池提前析构，极有可能导致core dump。
```cpp
class A
{
public:
    A(){}
    A(int){}
};
int main()
{
    //placement new和shared_ptr测试
    {
        A* a = new A[10];
        A* t = a;
        for (int i = 0; i < 10; i++)
        {
            std::shared_ptr<A> s(new(t)A(i),[](A* ptr){ ptr->~A();});
            t++;
        }
        delete [] a;
    }
    //weak_ptr和shared_ptr对象联合使用
    std::weak_ptr<A> w ;
    {
        std::shared_ptr<A> t(new A(1));
        w = t;
    }
    std::shared_ptr<A> t1 = w.lock();
    if(t1 != nullptr)
    {
        std::cout << t1.use_count() << std::endl;;
    }
    std::cout << w.use_count() << std::endl;
    return 0;
}
``｀