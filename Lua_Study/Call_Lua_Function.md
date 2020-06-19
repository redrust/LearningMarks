# 11.调用Lua函数
- 首先，将待调用的函数压栈
- 然后，压入函数的参数
- 接着，使用lua_pcall进行实际的调用
- 最后，从栈中取出结果

```lua
-- 待调用的lua函数
function f(x,y)
    return (x^2 * math.sin(y)) / (1 - x);
end
```

- lua_pcall(L,n,r,f)
  - L:lua_State
  - n:函数参数个数
  - r:返回结果个数
  - f:错误处理函数,0表示没有错误处理函数
  - 在压入结果之前，lua_pcall会将函数和函数参数出栈
  - 第一个结果先入栈，最后一个结果最后入栈
```cpp
// 调用Lua语言中定义的函数f
double f(lua_State* L,double x,double y)
{
    // 函数和参数压栈
    lua_getglobal(L,"f"); // 要调用的函数
    lua_pushnumber(L,x); // 压入第一个参数
    lua_pushnumber(L,y); // 压入第二个参数

    // 进行函数调用(两个参数，一个结果)
    if(lua_pcall(L,2,1,0) != LUA_OK)
    {
        error(L,"error running function 'f':%s",lua_tostring(L,-1));
    }

    // 获取结果
    int isnum = 0;
    double z = lua_tonumberx(L,-1,&isnum);
    if(!isnum)
    {
        error(L,"function 'f' should return a number");
    }
    lua_pop(L,1); // 弹出返回值
    return z;
}
```