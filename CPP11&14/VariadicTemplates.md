## Variadic Templates
- 可变参数模板:可以接收任意参数个数，且每个参数类型也可以是任意的。
- 如果要知道包(args)里有多少个参数，可以使用`sizeof...(args)`直接获取其个数。
```cpp
//可变参模板实现递推
//递推注意事项：一定要实现一个出口函数，处理最后状态，否则会编译出错。
void printX() //出口函数 版本１
{
}

template<typename T,typename... Types>
void printX(const T& firstArg,const Types&... args) //版本２
{
    std::cout << firstArg << std::endl;
    printX(args...);
}

template<typename... Types>
void printX(const Types&... args) //版本３
{
    //do something
}
```
### 思考：版本２和版本３可以共存吗？如果可以，谁更特化？
- [答]：可以共存，版本２比版本３更特化。

### recursive function call
```cpp
template<typename... Types>
inline size_t hash_val(const Types&... args) //版本１
{
    size_t seed = 0;
    hash_val(seed,args...);
    return seed;
}
template<typename T,typename... Types>
inline void hash_val(size_t& seed,const T& val,const Types&... args) //版本２
{
    hash_combine(seed,val);
    hash_val(seed,args...);
}
template<typename T>
inline void hash_val(size_t& seed,const T& val) //版本３
{
    hash_combine(seed,val);
}
inline void hash_combine(size_t& seed,const T& val)
{
    seed ^= sed::hash<T>()(val) + 0x9e3779b9 + (seed<<6) + (seed >> 2);
}
class CustomerHash
{
public:
    std::size_t operator()(const Customer& c)const
    {
        return hash_val(c.fname,c.lname,c.no);
    }
};
```
调用顺序：类方法入口调用，调用版本１，版本１做初始化操作。接着调用版本２，版本２做拆包操作，不断调用自身（递归），直到包里只剩下最后一个参数。最后调用版本３，版本３作为出口函数，做最后状态处理。

### tuple也是这个实现思路。关键点在于私有继承包对象。
``` cpp
template<typename... Values> class tuple;
template<>class tuple<>{} //最后的出口处理

template<typename Head,typename... Tail>
class tuple<Head,Tail...>:private tuple<Tail...>
{
    typedef tuple<Tail...> inherited;
public:
    tuple(){}
    tuple(Head v,Tail... vtail):m_head(v),inherited(vtail...){}
    typename Head::type head(){ return m_head; }
    inherited& tail() { return *this; }
protected:
    Head m_head;
}

//测试案例
tuple<int,float,string> t(41,6.3,"nico");
t.head(); //获得"41"
t.tail(); //获得" "
t.tail().head(); //获得6.3
```
测试案例中继承结构如下:  
tuple<>  
&emsp;&emsp;&emsp;$\uparrow$  
tuple<string>  
string m_head("nico");  
&emsp;&emsp;&emsp;$\uparrow$  
tuple<float,string>  
float m_head(6.3);  
&emsp;&emsp;&emsp;$\uparrow$  
tuple<int,float,string>  
int m_head(41);
