## lambda
- 函数对象，或者说仿函数，可以被当成参数或者对象进行处理。
- 是定义一个类型，后面加()可以直接调用。
- 一组可以被定义在语句中或者表达式中的函数定义。
- 由`[]()mutable throwSpec -> returnType {}`组成定义。
- `[]`:捕获列表,捕获当前作用域内的变量。
- `()`:参数列表　
- `{}`:函数体　
- `mutable`:增加这个关键字可以在函数体内改变捕获列表中参数的值。
- 捕获列表中:[=]传值　[&]传引用　并且`=`号也可以作为占位符，可以匹配外部任何变量。
```cpp
//基础示例
int id = 0;
auto f = [id] mutable {
    std::cout << "id:" << id << std::endl;
    ++id;
};
id = 42;
f();
f();
f();
std::cout << id << std::endl;
//示例输出：id:0 id:1 id:2 42
//equvalent to
class Functor{
private:
    int id;
public:
    void operator()(){
        std::cout << "id:" << id << std::endl;
        ++id;
    }
};
Functor f;
//传引用示例
int id = 0;
auto f = [&id] (int param) {
    std::cout << "id:" << id << std::endl;
    ++id;++param;
};
id = 42;
f(7);
f(7);
f(7);
std::cout << id << std::endl;
//传引用示例输出:id:42 id:43 id:44 45
```
```cpp
//更复杂的lambda编译器生成代码示例
int tobefound = 5;
auto lambda1 = [tobefound](int val){ return val == tobefound; };
//equvalent to
class UnNameLocalFunction
{
    int localVar;
public:
    UnNameLocalFunction(int var):localVar(var){}
    bool operator()(int val)
    { return val == tobefound; }
};
bool b1 = lambda1(5);
bool b2 = lambda1(5);
```
```cpp
//lambda和容器结合使用示例
vector<int> vi{5,28,50,83,70,590};
int x = 30;
int y = 100;
vi.erase( 
    remove_if(vi.begin(),
                        vi.end(),
                        [x,y](int n){ return x < n && n < y ; }
                        ),
    vi.end()
    );
for(auto i : vi)
    cout << i << " "; //输出结果:5 28 590
```