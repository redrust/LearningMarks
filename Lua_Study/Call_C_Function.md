# 12.在Lua中调用C语言
- 所有在Lua中注册的C函数都必须使用一个相同的原型:`typedef int (*lua_CFunction)(lua_State *L)`
- 此原型声明，参数为lua句柄，返回值为压入栈中的返回值个数
- 在调用C语言函数之前，要通过lua_pushcfuntion注册c函数
```lua
lua_pushcfuntion(L,func) -- 注册名字为func的C函数
lua_setglobal(L,"func") -- 将函数命名为func，之后Lua中就可以通过func进行调用对应的C函数
```
```cpp
static int l_dir(lua_State* L)
{
    struct dirent *entry;
    const char *path = luaL_checkstring(L,1); // 检查参数是否为字符串

    // 打开目录
    DIR* dir = opendir(path);
    if(dir == nullptr) // 打开目录失败
    {
        lua_pushnil(L); // 返回nil值
        lua_pushstring(L,strerror(errno)); // 返回错误信息
        return 2; // 返回值的数量
    }

    // 创建结果表
    lua_newtable(L);
    int i = 1;
    while((entry = readdir(dir)) != nullptr) // 对于目录中的每一个元素
    {
        lua_pushinteger(L,i++); // 压入键
        lua_pushstring(L,entry->d_name); // 压入值
        lua_settable(L,-3); // 插入表中,table[i] = 元素名
    }
    closedir(dir);
    return 1; // 表本身就在栈顶
}

```

## 1.在Lua中注册C模块
```cpp
static const struct lua_Reg mylib[] = {
    {"dir",l_dir},
    {nullptr,nullptr} // 哨兵
};

//luaL_newlib会新创建一个表，并使用由数组mylib指定的"函数名-函数指针"填充这个新创建的表
//luaL_newlib返回时将新创建的表留在栈中，在表中它打开了这个库
//luaopen_mylib返回１,表示将这个表返回给Lua
int luaopen_mylib(lua_State *L){
    luaL_newlib(L,mylib);
    return 1;
}
```
- 做完上面的步骤之后，还需要将其链接到解释器