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



