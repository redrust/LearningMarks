# 12.编写C函数的案例

## 1.对数组中的所有元素调用一个指定函数
- `void lua_geti(lua_State *L,int index,int key)`
- `void lua_seti(lua_State *L,int index,int key)`
  - index表示表在栈中的位置,key表示元素在表中的位置
- 调用lua_geti等价于
    ```cpp
    lua_pushnumber(L,key);
    lua_gettable(L,index);
    ```
- 调用lua_seti等价于
    ```cpp
    lua_pushnumber(L,key);
    lua_insert(L,-2); // 把key放在value的下面,因为是键值对
    lua_settable(L,index);
    ```

```cpp
int l_map(lua_State *L)
{
    //第一个参数必须是一张表
    luaL_checktype(L,1,LUA_TTABLE);

    //第二个参数必须是一个函数
    luaL_checktype(L,2,LUA_TFUNCTION);

    //获取表的大小
    int n = luaL_len(L,1);

    for(int i = 1; i <= n ; i++)
    {
        //压入函数f
        lua_pushvalue(L,2);
        //压入t[i]
        lua_geti(L,1,i);
        //调用f(t[i])
        lua_call(L,1,1);
        //t[i] = result
        lua_seti(L,1,i);
    }
    //没有返回值
    return 0;
}
```

## 2.字符串操作
### 1.分割字符串
```cpp
static int l_split(lua_State *L)
{
    const char *s = luaL_checkstring(L,1); // 目标字符串
    const char *sep = luaL_checkstring(L,2); // 分隔符
    
    lua_newtable(L); // 新建结果表
    
    // 依次处理每个分隔符
    const char *e = nullptr;
    while((e = strchr(s,*sep)) != nullptr)
    {
        lua_pushlstring(L,s,e - s); // 压入字串
        s = e + 1; // 跳过分隔符 
    }

    // 插入最后一个字串
    lua_pushstring(L,s);
    lua_rawseti(L,-2,1);
    return 1;// 将结果返回
}
```
### 2.`const char *lua_pushfstring(lua_State *L,const char *fmt,...)`
- 格式化创建字符串,与sprintf类似,但是不需要提供缓冲区,Lua会自动创建
- 将结果压入栈中并返回一个指向它的指针
### 3.使用缓冲区
```cpp
// 使用任意大小的缓冲区,将字符串转成大写
static int str_upper(lua_State *L)
{
    size_t l = 0;
    const char *s = luaL_checkstring(L,1,&l);
    //声明一个buffer对象
    luaL_Buffer b{};
    //获取指向指定大小缓冲区的指针
    char *p = luaL_buffinitsize(L,&b,l);
    for (size_t i = 0; i < l; i++)
    {
        p[i] = toupper(uchar(s[i]));
    }

    //将缓冲区的内容转换成一个新的Lua字符串
    luaL_pushresultsize(&b,l);
    return 1;
}
```
### 4.常用字符串函数
- `void luaL_buffinit(lua_State *L,luaL_Buffer *B)`:初始化缓冲区
- `void luaL_addvalue(luaL_Buffer *B)`:在栈顶增加一个字符串
- `void luaL_addlstringt(lua_State *L,const char *s,size_t l)`:增加一个长度明确的字符串
- `void luaL_addstringt(lua_State *L,const char *s)`:增加一个以\0结尾的字符串
- `void luaL_addchar(lua_State *L,char s)`:增加单个字符
- `void luaL_pushresult(luaL_Buffer *B)`:将缓冲区中的内容转换成Lua字符串,并且将其压栈
### 5.table.concat一个简化的实现
```cpp
static int tconcat(lua_State* L)
{
    luaL_Buffer b{};
    luaL_checktype(L,1,LUA_TTABLE);
    int n = luaL_len(L,1);
    luaL_buffinit(L,&b);
    for (size_t i = 0; i <= n; i++)
    {
        lua_geti(L,1,i); // 从表中获取字符串
        luaL_addvalue(b); // 将其放入缓冲区
    }
    luaL_pushresult(&b);
    return 1;
}
```

## 3.注册表
- 注册表是一张只能被C代码访问的全局表．通常情况下,我们使用注册表存储多个模块间共享的数据．
- 注册表总是位与LUA_REGISTRYINDEX中
- 1.获取注册表中键为"key"的值
  - `lua_getfield(L,LUA_REGISTRYINDEX,"key");`
- 2.使用引用
  - 1.创建新的引用:`int ref  = luaL_ref(L,LUA_REGISTRYINDEX);`
  - 2.将与引用相关联的值压入栈中:`lua_rawgeti(L,LUA_REGISTRYINDEX,ref);`
  - 3.释放值和引用:`lua_unref(L,LUA_REGISTRYINDEX,ref);`
- 3.预定义引用
  - LUA_RIDX_MAINTHREAD:指向Lua状态本身,也就是其主线程
  - LUA_RIDX_GLOBALS:指向全局变量
- 4.使用C语言指针(转换为轻量级用户数据)作为键
  ```cpp
    static char Key = 'k';

    //保存字符串
    lua_pushstring(L,myStr);
    lua_rawsetp(L,LUA_REGISTRYINDEX,(void*)&Key);// 将键值绑定

    //获取字符串
    lua_rawgetp(L,LUA_REGISTRYINDEX,(void*)&Key);
    myStr = lua_tostring(L,-1);
  ``` 

## 4.上值
- 上值实现了类似于C语言静态变量(只在特定的函数中可见)的机制
- C函数与其上值的关联称为闭包
```cpp
static int counter(lua_State *L)
{
    //lua_upvalueindex:生成上值的伪索引,给出当前运行函数的第一个上值的伪索引
    int val = lua_tointeger(L,lua_upvalueindex(1));
    lua_pushinteger(L,++val); // 新值
    lua_copy(L,-1,lua_upvalueindex(1)); // 更新上值
    return 1; // 返回新值
}

int newCounter(lua_State *L)
{
    lua_pushinteger(L,0);
    //创建一个新的闭包,第二个参数是一个基础函数,第三个参数是上值的数量
    lua_pushcclosure(L,&counter,1);
    return 1;
}
```