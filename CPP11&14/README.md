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
## 6.explicit for ctors taking more than one argument
- cpp11之前，只适用于单一实参。cpp11之后，支持多实参构造函数。
- explicit关键字：主要应用于构造函数.
- 禁止隐式转换，禁止隐式调用对应构造函数。

