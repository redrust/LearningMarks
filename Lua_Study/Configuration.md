# 10.使用Lua进行程序配置

## 1.读取配置文件
```cpp
/*
配置文件:
width = 200
height = 300
*/
int getglobint(lua_State *L,const char *var)
{
    lua_getglobal(L,var);
    int isnum = 0;
    int result = (int)lua_tointegerx(L,-1,&isnum);
    if(!isnum)
    {
        error(L,"'%s' should br a number\n",var);
    }
    lua_pop(L,1); // 从栈中移除结果
    return result;
}

void load(lua_State* L,const char *fname,int *w,int *h)
{
    //加载编译文件中的代码段，并且运行编译后的代码段
    if(luaL_loadfile(L,fname) || lua_pcall(L,0,0,0))
    {
        error(L,"cannot run config.file:%s\n",lua_tostring(L,-1));
    }
    *w = getglobint(L,"width");
    *h = getglobint(L,"height");
}

int main(int argc, char const *argv[])
{
    lua_State* L = luaL_newstate();
    luaL_openlibs(L);
    int w = 0,h = 0;
    load(L,"config.txt",&w,&h);
    lua_close(L);
    return 0;
}
```

## 2.操作表
```cpp
#define MAX_COLOR 255

struct ColorTable{
    const char *name;
    unsigned char red,green,blue;
}colortable[] = {
    {"WHITE",MAX_COLOR,MAX_COLOR,MAX_COLOR},
    {"RED",MAX_COLOR,0,0},
    {"GREEN",0,MAX_COLOR,0},
    {"BLUE",0,0,MAX_COLOR},
    {nullptr,0,0,0},
};

//错误处理函数，打印错误信息并关闭lua_State
void error(lua_State* L,const char *fmt,...)
{
    va_list argp;
    va_start(argp,fmt);
    vfprintf(stderr,fmt,argp);
    va_end(argp);
    lua_close(L);
    exit(EXIT_FAILURE);
}

//假设表位于栈顶
//获取表中对应键的值
int getcolorfield(lua_State *L,const char *key)
{
    //lua_gettable使用键来索引表中元素的值,所以需要先压入键，才能正确的在表中索引
    lua_pushstring(L,key); // 压入键
    // lua_gettable使用键索引，获取到值之后将值入栈
    lua_gettable(L,-2); // 获取background[key]
    int isnum = 0;
    //索引到之后，lua将值压栈，所以在栈顶读取对应键的值
    int result = (int)(lua_tonumberx(L,-1,&isnum) * MAX_COLOR);
    if(!isnum)
    {
        error(L,"invalid component '%s' in color",key);
    }
    lua_pop(L,1); // 移除数值
    return result;
}

int easygetcolorfield(lua_State *L,const char *key)
{
    if(lua_getfield(L,-1,key) != LUA_TNUMBER)// 获取background[key]
    {
        error(L,"invalid component in background color");
    }
    return (lua_tonumber(L,-1) * MAX_COLOR);
}

//假设表位与栈顶
//将键值对插入到表中
void setcolorfield(lua_State *L,const char *key,int value)
{
    lua_pushstring(L,key); // 将键入栈
    lua_pushnumber(L,(double)value / MAX_COLOR); // 将值入栈
    lua_settable(L,-3); // 将键值对插入到表中,同时lua_settable这个函数会将键值出栈,完成这个函数调用之后，栈顶指向表
}

void easysetcolorfield(lua_State *L,const char *key,int value)
{
    lua_pushnumber(L,(double)value / MAX_COLOR); // 将值入栈
    lua_setfield(L,-2,key);
}

//设置颜色表
void setcolor(lua_State* L,struct ColorTable *ct)
{
    //创建表并为元素预分配空间
    lua_createtable(L,0,3); // 创建表
    setcolorfield(L,"red",ct->red);
    setcolorfield(L,"green",ct->green);
    setcolorfield(L,"blue",ct->blue);
    lua_setglobal(L,ct->name); // 'name' = table
}
int main(int argc, char const *argv[])
{
    lua_State* L = luaL_newstate();
    int i = 0;
    while(colortable[i].name != nullptr)
    {
        setcolor(L,&colortable[i++]);
    }
    lua_close(L);
    return 0;
}
```

