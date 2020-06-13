## 6.协程
- 协程是一系列的可执行语句，拥有自己的栈．局部变量和指令指针，同时协程又与其他协程共享了全局变量和其他几乎一切资源．
- 与线程不同的是，协程却需要彼此协作地运行，即在任意指定的时刻只能有一个协程运行，且只有当正在运行的协程显式地要求被挂起时其执行才会暂停

### 1.协程基础
- 协程相关的所有函数都被放在表coroutine中
- 一个协程有四种状态：挂起．运行．正常．死亡
```lua
-- 创建协程
co = coroutine.create(function() print("hi") end)
print(type(co)) --> thread

-- 检查协程状态
print(coroutine.status(co)) --> suspended

-- 唤醒协程
coroutine.resume(co)

-- 挂起协程
co = coroutine.create(function()
                        for i = 1,10 do
                          print("co",i)
                          coroutine.yield() -- 协程主动将自己挂起
                        end
                      end)
coroutine.resume(co) --> co 1 主动唤醒协程

-- 通过resume-yeild来传递参数
co = coroutine.create(function (a,b,c)
                        print("co",a,b,c + 2)
                      end)
coroutine.resume(co,1,2,3) --> co 1 2 5

co = coroutine.create(function (a,b)
                        coroutine.yield(a + b,a - b)
                      end)
-- resume返回值中，第一个返回值为true时表示没有错误,之后返回对应的yield的参数
print(coroutine.resume(co,20,10)) -->true 30 10

-- 当一个协程结束时，主函数所返回的值都将编程对应函数resume的返回值
co = coroutine.create(function ()
                        return 6,7
                      end)
print(coroutine.resume(co)) --> true 6 7

-- 使用wrap来创建协程，与create不同的是它返回一个函数，调用这个函数即唤醒协程
function wrap(a)
  return coroutine.wrap(function () print("hi") end)
end
```

### 2.异步ＩＯ库
```lua
local cmdQueue = {} -- 挂起操作的队列

local lib = {}

function lib.readline(stream,callback)
  local nextCmd = function()
                    callback(stream:read())
                  end
  table.insert(cmdQueue,nextCmd)
end

function lib.writeline(stream,line,callback)
  local nextCmd = function()
                    callback(stream:write(line))
                  end
  table.insert(cmdQueue,nextCmd)
end

function lib.stop()
  table.insert(cmdQueue,"stop")
end

function lib.runloop()
  while true do
    local nextCmd = table.remove(cmdQueue,1)
    if nextCmd == "stop" then
      break
    else
      nextCmd() -- 进行事件回调
    end
  end
end
```

### 3.使用异步库运行同步代码
```lua
local lib = require "async-lib"

function run(code)
  local co = coroutine.wrap(function()
                              code()
                              lib.stop() -- 结束时停止事件循环
                            end)
  co() -- 启动协程
  lib.runloop() -- 启动事件循环
end

function getline(stream,line)
  local co = coroutine.running() -- 调用协程
  local callback = (function (l) coroutine.resume(co,l) end) -- 事件回调里，唤醒协程
  lib.readline(stream,callback) -- 将回调函数打包成事件注册到loop中
  local line = coroutine.yield()　-- 主动挂起等待loop回调函数完成,同步事件
  return line
end

function putline(stream,line)
  local co = coroutine.running() -- 调用协程,返回正在运行的coroutine
  local callback = (function () coroutine.resume(co) end) -- 事件回调里，唤醒协程
  lib.writeline(stream,line,callback)
  coroutine.yield()
end


-- 异步库的同步调用
run(function()
      local t = {}
      local inp = io.input()
      local out = io.output()

      while true do
        local line = getline(inp)
        if not line then break end
        t[#t + 1] = line
      end
      
      for i = #t,1,-1 do
        putline(out,t[i] .. "\n")
      end
    end
)
```