- 此测试针对unique_ptr进行测试。
- 不是标准unique_ptr的使用测试.
- 通过自定义删除器、以及暴露unique_ptr的原始指针,使unique_ptr所持有的元素,可以同时被两个容器持有,是十分危险的行为,破坏了unique_ptr的特性,而且得不偿失.但是值得思考、有参考价值,故保留下来,以此警醒自己.

```cpp
#include <memory>
#include <set>
#include <vector>
#include <iostream>
int main()
{
    //声明自定义删除器,为了能够通过编译,必须持有对应对象的指针,但是不做任何事情,因为资源的最终清理是由unique_ptr完成的
    auto Deleter = [](int*){};
    //持有unique_ptr的第一个容器
    std::set<std::pair<int,std::unique_ptr<int>>> t;
    t.emplace(std::pair<int,std::unique_ptr<int>>(1,std::make_unique<int>(2)));

    //持有unique_ptr的第二个容器
    std::vector<std::pair<int,std::unique_ptr<int,decltype(Deleter)>>> v;
    for(auto it = t.begin() ; it != t.end() ; ++it)
    {
        //十分长而且没有任何特殊意义的插入代码,只是为了通过编译器编译
        v.emplace_back(std::pair<int,std::unique_ptr<int,decltype(Deleter)>>(it->first,std::unique_ptr<int,decltype(Deleter)>(it->second.get(),Deleter)));
    }
    std::cout << t.size() << std::endl;
    //t.erase(t.begin(),t.end());
    t.clear();
    std::cout << t.size() << " " << v.size() << std::endl;
    return 0;
}
```
程序输出如下:  
1  
0 1  
程序通过编译也没有崩溃,可喜可贺可喜可贺.
