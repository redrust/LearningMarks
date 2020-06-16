# C语言API
- Lua是一种嵌入式语言,作为一个库链接到其他应用程序，将Lua的功能融入这些应用
- Lua也可以作为扩展式语言，在宿主语言中可以注册新的Lua函数,从而增加一些无法直接用Lua语言编写的功能

## 1.一个简单的独立解释器
```cpp
#include <stdio.h>
#include <string.h>

extern "C"{
    #include <lua5.3/lua.h>
    #include <lua5.3/lauxlib.h>
    #include <lua5.3/lualib.h>
}

int main(int argc, char const *argv[])
{
    char buff[256];
    int error;
    lua_State *L = luaL_newstate(); //打开lua句柄
    luaL_openlibs(L); //在这个lua句柄上打开所有的标准库

    while(fgets(buff,sizeof buff,stdin) != nullptr)
    {
        //luaL_loadstring将用户输入编译，并将编译好的函数压入lua栈中
        //然后调用lua_pcall从栈中弹出编译后的函数，并以保护模式运行
        //如果没有错误返回０，如果有错误两个函数都像栈中压入一条错误信息
        error  = luaL_loadstring(L,buff) || lua_pcall(L,0,0,0);
        if(error)
        {
            fprintf(stderr,"$s\n",lua_tostring(L,-1)); //从栈中读取错误信息
            lua_pop(L,1); // 从lua栈中弹出错误信息
        }
    }
    lua_close(L); // 关闭lua句柄
    return 0;
}
```

## 2.栈
- 一个C语言与Lua之间进行通信的虚拟栈
- 一般的操作是C压栈Lua出栈，或者与之相反

### 1.压栈
- 压栈函数(C语言方向):
  - 1.lua_pushnil(lua_State *L):压入一个常量nil
  - 2.lua_pushboolean(lua_State *L,int bool):压入一个布尔值(在c语言中是整形)
  - 3.lua_pushnumber(lua_State *L,lua_Number n):压入一个双精度浮点型
  - 4.lua_pushinteger(lua_State *L,lua_Integer n):压入一个整形
  - 5.lua_pushlstring(lua_State *L,const char *s,size_t len):压入一个任意字符串(一个指向char的指针，外加一个长度)
  - 6.lua_pushstring(lua_State *L,const char *s):压入一个以\0终止的字符串

- Lua启动时栈中至少会有20个位置，具体要看lua.h中的常量LUA_MINSTACK
- 使用`int lua_checkstack(lua_State *L,int sz)`来检查栈中是否有足够的空间，sz是额外栈位置的数量，如果可能，此函数会增大栈的大小，否则会返回0
- `void luaL_checkstack(lua_State *L,int sz,const char *msg)`此函数类似于lua_checkstack，但是当栈空间不满足需求时，使用指定的错误信息抛出异常，而不是返回错误码
