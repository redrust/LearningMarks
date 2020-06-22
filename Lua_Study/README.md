# Lua学习笔记
## 资源和内容全部来源于Roberto lerusalimschy的《Programming in Lua(fourth edition)》

## 1.[基础语法](基础语法.md)
- Lua语言的基础语法

## 2.[语言特性](语言特性.md)
- Lua语言的一些语法特性,元表与元方法的使用

## 3.[编程实操](编程实操.md)
- 闭包．模式匹配.日期和时间．位和字节的函数使用

## 4.[面向对象编程](ObjectOriented.md)
- Lua语言中使用元表实现对象的封装和继承等面向对象的特性

## 5.[数据结构](DataStructure.md)
- 使用表结构实现各种高级复杂的数据结构

## 6.[协程](Coroutine.md)
- 重要的实现异步并发的语言特性之一

## 7.[反射](Reflection.md)
- 用于Lua程序自我检查的一种机制，包括自省．钩子和沙盒

## 8.[协程模拟多线程操作](coroutine_to_thread.md)
- 使用协程进行异步IO操作达成并发下载文件的目的

## 9.[C语言与Lua协作基础](C_TO_Lua.md)
- C语言与Lua语言进行协作配合，着重介绍了栈的相关操作

## 10.[使用Lua进行程序配置](Configuration.md)
- 将配置文件写成Lua语言支持的格式，就可以直接进行代码编译，使其支持C语言使用
  
## 11.[调用Lua函数](Call_Lua_Function.md)
- C语言调用Lua语言函数，并且获取函数调用结果

## 12.[调用C函数](Call_C_Function.md)
- Lua语言调用C语言函数

## 13.[编写C语言函数案例](C_Function_Example.md)
- 包括统一处理数组元素．字符串处理．注册表．上值四个典型案例实现

## 14.[编译．执行和错误](Compile&Package.md)
- Lua语言中错误处理的手段，以及包编写的案例
