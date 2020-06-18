# 9.C语言API
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

### 2.查询栈
- C API使用索引来引用栈中的元素．第一个入栈的元素索引为１，第二个入栈的元素索引为２．还可以使用负数，来进行索引，比如－１是栈顶元素，－２是在栈顶入栈之前入栈的元素
- 1.检查栈中元素是否为特定的类型
  - 比如lua_isnil,lua_isnumber,lua_isstring和lua_istable等
  - int lua_is*(lua_State *L,int index)
  - 这些函数不会检查元素是否为特定类型，而是检查元素是否能转换撑特定类型
- 2.lua_type返回栈中元素类型
  - 返回的类型使用常量表示，包括LUA_TNIL,LUA_TBOOLEAN,LUA_TNUMBER,LUA_TSTRING
- 3.lua_to*从栈中获取一个值
  - int lua_toboolean(lua_State *L,int index)
    - nil和false转换成0,其他Lua值转换成1
  - const char *lua_tolstring(lua_State *L,int index,size_t *len)
    - 返回一个指向该字符串内部副本的指针，并将字符串的长度存入到对应参数len指定的位置
    - 类型不正确时返回null
  - lua_State *lua_tothread(lua_State *L,int index)
    - 类型不正确时返回null
  - lua_Number lua_tonumber(lua_State *L,int index)
    - 类型不正确时返回0
  - lua_Integer lua_tointeger(lua_State *L,int index)
    - 类型不正确时返回0
  - lua_Number lua_tonumber(lua_State *L,int index,int *isnum) & lua_Integer lua_tointeger(lua_State *L,int index)
    - isnum返回bool值，表示Lua值是否被强制转换为期望的类型

### 3.其他栈操作
- 1.`int lua_gettop(lua_State *L)`:返回栈中元素的个数，也就是栈顶元素的索引
- 2.`void lua_settop(lua_State *L,int index)`:将栈顶设置成指定的值，即修改栈中元素的数量
  - 如果之前的栈顶比新设的高，那么高出来的这些元素就会被丢弃，否则该函数会向栈中压入nil来补足大小
  - `lua_settop(L,0)`:用于清空栈
- 3.`void lua_pushvalue(lua_State *L,int index)`:将指定索引上的元素的副本压入栈
- 4.`void lua_rotate(lua_State *L,int index,int n)`:将指定索引的元素向栈顶转动n个位置,如果索引为负数表示向相反的方向转动
- 5.`void lua_remove(lua_State *L,int index)`:用于删除指定索引的元素，并将该位置上的所有元素下移以填补空缺
- 6.`void lua_insert(lua_State *L,int index)`:用于将栈顶元素移动到指定位置，并上移制定位置之上的所有元素以开辟出一个元素的空间
- 7.`void lua_replace(lua_State *L,int index)`:弹出一个值，并将栈顶设置为指定索引上的值，而且不移动任何元素
- 8.`void lua_copy(lua_State *L,int index)`:将一个索引上的值复制到另一个索引上，并且原值不受影响

```cpp
#include <stdio.h>
#include <string.h>

extern "C"{
    #include <lua5.3/lua.h>
    #include <lua5.3/lauxlib.h>
    #include <lua5.3/lualib.h>
}

static void stackDump(lua_State *L)
{
    int top = lua_gettop(L); // 获取栈的深度
    for (int i = 1; i <= top; ++i)
    {
        int t = lua_type(L,i); // 获取栈元素类型
        switch (t)
        {
            case LUA_TSTRING: // 字符串类型
            {
                printf("'%s'",lua_tostring(L,i));
                break;
            }
            case LUA_TBOOLEAN: // 布尔类型
            {
                printf(lua_toboolean(L,i) ? "true":"false");
                break;
            }
            case LUA_TNUMBER: // 数值类型
            {
                if(lua_isinteger(L,i)) // 整形?
                {
                    printf("%lld",lua_tointeger(L,i));
                }
                else // 浮点型
                {
                    printf("%g",lua_tonumber(L,i));
                }
                break;
            }
            default : // 其他类型
            {
                printf("%s",lua_typename(L,t));
            }
        }
        printf(" ");
    }
    printf("\n");
}
int main(int argc, char const *argv[])
{
    lua_State *L = luaL_newstate(); //打开lua句柄
    luaL_openlibs(L); //在这个lua句柄上打开所有的标准库

    lua_pushboolean(L,1);
    lua_pushnumber(L,10);
    lua_pushnil(L);
    lua_pushstring(L,"hello");

    stackDump(L);
    /* 输出： true 10 nil 'hello' */

    lua_pushvalue(L,-4);
    stackDump(L);
    /* 输出: true 10 nil 'hello' true */

    lua_replace(L,3);
    stackDump(L); // 将nil设置为栈顶的true,同时将true出栈
    /* 输出: true 10 true 'hello' */

    lua_settop(L,6);
    stackDump(L); // 增大栈的大小,空的位置补nil
    /* 输出: true 10 true 'hello' nil nil */

    lua_rotate(L,3,1);
    stackDump(L);// 将索引为3的元素true向上移动一个位置,并且用nil补足空缺,栈不会自动扩增，栈顶元素自动出栈
    /* 输出: true 10 nil true 'hello' nil */

    lua_remove(L,-3);
    stackDump(L);// 弹出从栈顶往回数第三个索引的元素
    /* 输出: true 10 nil 'hello' nil */

    lua_settop(L,-5);
    stackDump(L); // 将从栈顶往回数第五个索引的元素设为栈顶，原栈顶到新栈顶之间的元素全部出栈
    /* 输出: true */
    
    lua_close(L); // 关闭lua句柄
    return 0;
}

```