## 1.[Variadic Templates](VariadicTemplates.md)
## 2.Spaces in Template Expressions
- vector<list\<int>> 双尖括号时，可以直接写，不用带空格  
## 3.nuppltr and std::nullptr_t
- 可以使用nullptr代替0或者NULL.
## 4.Automatic Type Deduction with auto
- auto:自动类型推导，建议只使用在type十分长或者复杂的情况下。
## 5.Uniform Initialization
- 任何初始化都可以使用`{}`进行初始化。
- such as :```cppint values[]{ 1,2,3 }; vector<int> v{2,3,5,7,9};```
- 编译器读取到{}时，生成一个initializer_list\<T>(初始化列表)，关联至一个array<Type,TypeNum>。然后调用处理函数将元素逐一分解传给ctor进行构建。
    - 有两种情况，如果目标类型构造函数，存在一个对应的初始化列表，那么会将整个{}号内的元素打包传过去，否则会拆散单独传送。
## 6.Initializer Lists
- 初始化列表:如果为空，且目标对象为系统定义类型，可以给其一个默认的初值。
- int j{};//j被置为0
## 7.initializer_list<>
- 这个模板类型可以支持对一系列的值进行处理。
- 内部持有array对象，对参数包进行逐一构建处理。
- 初始化列表对象只持有目标对象的指针，而不持有目标对象本身，如果拷贝一个初始化列表对象，那么只能获得目标对象的引用，而不是一份新的拷贝（浅拷贝）。
- 如今所有容器都接受指定任意数量的值用于构建或者赋值或者insert()或者assign(),以及max()和min()也支持任意个数参数。
```cpp
void print(std::initializer_list<int> vals)
{
    for(auto p = vals.begin() ; p != vals.end() ; ++p)
    {
        std::cout << *p  << std::endl;
    }
}
print({1,3,5,7,9});
```
## 8.explicit for ctors taking more than one argument
- cpp11之前，只适用于单一实参。cpp11之后，支持多实参构造函数。
- explicit关键字：主要应用于构造函数.
- 禁止隐式转换，禁止隐式调用对应构造函数。

## 9.range-based for statement
- 按顺序遍历容器中的每个元素。
- explicit可以禁止隐式转换，可以在for-loop中禁止容器A转换成类型B。换句话来说，可以禁止用新式for-loop进行遍历容器中的所有元素。
```cpp
for(decl : coll)
{
    statement
}
//可以近似的看成
for(auto _pos=coll.begin(),_end=coll.end();_pos!=_end;++_pos)
{
    decl = *_pos;
    statement;
}
```
## 10.=default,=delete
- 如果自行定义有ctor，那么编译器将不再会提供default ctor，但是如果强制加上=default，就可以重新获得并且使用default ctor。
    - 一般函数不能使用default。
    - 如果实现有拷贝构造和拷贝赋值函数，那么不能使用default。
- =delete经常用于拷贝构造和拷贝赋值函数，用于禁止拷贝构造或者拷贝赋值的操作。
  - 析构函数不能使用delete。

## 11.NoCpoy and Private-Copy
```cpp
struct NoCopy{
    NoCopy() = default;
    NoCopy(const NoCopy&) = delete;
    NoCopy& operator=(const NoCopy&) = delete;
    ~NoCopy() = default;
};

struct NoDtor{
    NoDtor() = default;
    ~NoDtor() = delete;
};
NoDtor nd; //error:NoDtor destructor is deleted
NOdtor *p = new NoDtor(); //ok
delete p; //error:NoDtor destructor is deleted

//此class不允许被ordinary user code copy,但仍可被friends和members copy.
class PrivateCopy{
private:
    PrivateCopy(const PrivateCopy&);
    PrivateCopy& operator=(const PrivateCopy&);
public:
    PrivateCopy() = default;
    ~PrivateCopy();
};
```
- class noncopyable:使用了类似上述PrivateCopy的实现手段，所有继承它的子类，普通用户都不能直接代码拷贝其对象。

## 12.Alias Template(template typedef)
- 模板别名。
- 注意：无法对模板别名进行偏特化。
```cpp
//such as
template<typename T>
using Vec = std::vector<T,MyAlloc<T>>

Vec<int> coll;
//is equivalent to
std::vector<int,MyAlloc<int>> coll;
```

## 13.Template template parameter
- 模板作为另一个模板的参数。
```cpp
template<typename T,template<class> class Container>
class XCIs
{
private:
    Container<T> c;
};

//这个不能在函数体内部声明
template<typename T>
using Vec = std::vector<T,allocator<T>>;

XCIs<std::string,Vec> c1;
```

## 14.Type Alias(similar to typedef)
- example1
```cpp
//type alias,identical to
//typedef void (*func)(int,int);
using func = void(*)(int,int);
void example(int,int){}
func fn = example;
```
- example2
```cpp
//type alias can introduce a member typedef name
template<typename T>
struct Container{
    using value_type = T;
};
//which can be used in generic programming
template<typename Cntr>
void fn2(const Cntr& c)
{
    typename Cntr::value_type n;
}
```
- example3
```cpp
//type alias used to hide a template parameter
template<class CharT> using mystring = std::basic_string<CharT,std::char_traites<CharT>>;
mystring<char> str;
```
- using还可以用来做using-directives和using-declarations.比如`using namespace std;`和`using std::count;`.

## 15.noexcept
- 在函数后面写，保证该函数不抛出异常。
- 注意：move语意要正常调用，不仅要写出移动构造函数，还需要在移动构造函数后面加上noexcept，否则编译器不予以调用。
- 注意，growable containers（会发生memory reallcation）只有两种：vector和deque

## 15.overide
- 复写、改写，应用于虚函数上。声明该函数是从父类上继承过来，但是被重写了。如果不是重写的函数，那么编译器会报错。

## 16.final
- 用于类和虚函数。用于类的时候该类无法被继承。用于虚函数的时候该函数不能被重写。