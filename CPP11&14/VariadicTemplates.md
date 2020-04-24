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
    auto head()->decltype(m_head) head(){ return m_head; }
    inherited& tail() { return *this; }
protected:
    Head m_head;
}

//测试案例
tuple<int,float,string> t(41,6.3,"nico");
t.head(); //获得"41"
t.tail(); //获得一个内存地址,指向t对象的地址首部。
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

### 例１
- 可以将参数类型和值进行打包，之后通过模板预算推导，来实现拆包逐一处理元素。
```cpp
void printX()
{
}

template<typename T,typename... Types>
void printX(const T& firstArg,const Types&... args)
{
    std::cout << firstArg << std::endl;
    printX(args...);
}
```
### 例２
- 重写printf
```cpp
//出口处理，参数不匹配的异常情况
void printf(const char* s)
{
    while(*s)
    {
        if(*s == '%' && *(++s) != '%')
            throw std::runtime_error("invalid format string:missing argument");
        std::cout << *s++;
    }
}
template<typename T,typename... Args>
void printf(const char* s,T value,Args... args)
{
    while(*s)
    {
        if(*s == '%' && *(++s) != '%')
        {
            std::cout << value;
            printf(++s,args);
            return;
        }
        std::cout << *s++;
    }
    throw std::logic_error("extra arguments provided to printf");
}
```
### 例３
- 如果参数types相同，不需要动用variadic templates，使用initializer_list\<T>即可。
- 案例可以参考C++标准库的max算法实现。

### 例４
- 不使用initializer_list的max算法实现。
```cpp
int maximum(int n)
{
    return n;
}
template<typename... Args>
int maximum(int n,Args... args)
{
    return std::max(n,maximum(args...));
}
```

### 例５
- 特殊处理first元素和last元素
```cpp
cout << make_tuple(7.5,string("hello"),bitset<16>(377),42);
//输出结果[7.5,hello,0000000101111001,42]
//调用顺序－从上到下
//入口函数,给首部元素加上[]
template<typename... Args>
ostream& operator<<(ostream& os,const tuple<Args...>& t)
{
    os << "[";
    PRINT_TUPLE<0,sizeof...(Args),Args...>::print(os,t);
    return os <<"]";
}
//处理中间的元素
template<int IDX,int MAX,typename... Args>
struct PRINT_TUPLE{
    static void print(ostream& os,const tuple<Args...>& t)
    {
        os << get<IDX>(t) << (IDX + 1 == MAX ? "":",");
        PRINT_TUPLE<IDX+1,MAX,Args...>::print(os,t);
    }
};
//处理尾部元素
//偏特化处理尾部元素
template<int MAX,typename... Args>
struct PRINT_TUPLE<MAX,MAX,Args...>
{
    static void print(ostream& os,const tuple<Args...>& t){}
};
```

### 例６
- 用于递归继承，recursive inheritation
- 参考tuple实现。

### 例７
- 用于递归复合，recursive composition
- 递归调用，处理的都是参数，使用function template
- 递归复合，处理的都是类型，使用class template
- 类对象与类对象之间的关系变成了组合，就不再是继承了
```cpp
template<typename... Values> class tup;
template<> class tup<>{};
template<typename Head,typename... Tail>
class tup<Head,Tail...>
{
    typedef tup<Tail...> composited;
protected:
    composited m_tail;
    Head m_head;
public:
    tup(){}
    tup(Head v,Tail... vtail):m_tail(vtail),m_head(v){}
    Head head(){ return m_head; }
    //传引用，否则会是一份拷贝
    composited& tail(){ return m_tail; }
};

//测试案例
tup<int,float,std::string> it1(41,6.3,"nico");
std::cout << sizeof(it1) << std::endl;
std::cout << it1.head() << std::endl;
std::cout << it1.tail().head() << std::endl;
std::cout << it1.tail().tail().head() << std::endl;
/*输出结果如下：
*56
*41
*6.3
*nico
*/
```
