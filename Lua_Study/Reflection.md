## 7.反射
- 反射是程序用来检查和修改其自身某些部分的能力

### 1.自省机制
- 1.debug.getinfo:返回一个包含该函数有关的一些数据的表
```lua
-- 打印活跃栈的栈回溯
function traceback()
    for level = 1,math.huge do
        local info = debug.getinfo(level,"Sl")
        if not info then break end
        if info.what == "C" then
            print(string.format("%d\tC function",level))
        else
            print(string.format("%d\t[%s]:%d",level,info.short_src,info.currentline))
        end
    end
end
```
- 2.debug.getlocal:检查任意活跃函数的局部变量
  - 参数:查询函数的栈层次,变量的索引
  - 返回值:变量名,变量当前值
- 3.debug.setlocal:改变局部由变量的值
  - 参数:前两个与getlocal相同,第三个参数为该局部变量的新值
- 4.debug.getupvalue:访问一个全局变量
  - 参数:第一个参数为函数,第二个参数是变量的索引
- 5.debug.setupvalue:设置一个全局变量的新值
```lua
-- 获取变量的值
function getvarvalue(name,level,isenv)
    local value
    local found = false

    level = (level or 1) + 1

    -- 尝试局部变量
    for i = 1,math.huge do
        local n,v = debug.getlocal(level,i)
        if not n then break end
        if n == name then
            value = v
            found = true
        end
    end
    if found then return "local",value end

    --尝试非局部变量
    local func = debug.getinfo(level,"f").func
    for i = 1,math.huge do
        local n,v = debug.getlocal(func,i)
        if not n then break end
        if n == name then
            value = v
            found = true
        end
    end
    if found then return "upvalue",value end

    if isenv then return "noenv" end -- 避免循环

    -- 没找到,从环境中获取值
    local _,env = getvarvalue("_ENV",level,true)
    if env then
        return "glbal",env[name]
    else -- 没有找到有效的env
        return "noenv"
    end
end

-- 用法如下
local a = 4
print(getvarvalue("a"))
```

### 2.钩子
- 钩子函数会在程序运行中某个特定事件发生时被调用
- 四种事件能够触发一个钩子
  - 1.调用一个函数时产生call事件
  - 2.函数返回时产生return事件
  - 3.开始执行一行新代码时产生的line事件
  - 4.执行完制定数量的指令后产生的count事件
- debug.sethook:注册一个钩子函数
  - 参数:第一个参数是钩子函数,第二个参数是选项字符串,第三个参数是count事件的频次
```lua
-- 用于计算调用次数的钩子
local Counters = {}
local Names = {}
local function hook()
    local f = debug.getinfo(2,"f").func
    local count = Counters[f]
    if count == nil then -- 'f'第一次被调用
        Counters[f] = 1
        Names[f] = debug.getinfo(2,"Sn")
    else -- 多次到调用
        Counters[f] = count + 1
    end
end
-- 使用
local f = assert(loadfile(arg[1]))
debug.sethook(hook,"c") -- 设置call事件的钩子
f() -- 运行主程序
debug.sethook() -- 关闭钩子
```

### 3.沙盒
- Lua语言通过库函数完成所有与外部世界的通信,因此一旦移除了这些函数也就排除一个脚本能够影响外部环境的可能
```lua
local debug = require "debug"

-- 最大能执行的steps
local steplimit = 1000
-- 最大能够使用的内存(kb)
local memlimit = 1000

local count = 0 -- 计数器

local function checkmem()
    if collectgarbage("count") > memlimit then
        error("script uses too much memory")
    end
end

end
local function step()
    checkmem()
    count = count + 1
    if count > setplimit then
        error("script uses too much CPU")
    end
end

-- 加载
local f = assert(loadfile(arg[1],"t",{}))

debug.sethook(setp,"",100) -- 设置钩子
f() -- 运行文件

```