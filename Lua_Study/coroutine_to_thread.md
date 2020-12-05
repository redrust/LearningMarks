## 8.使用协程实现多线程
- 多线程并发下载文件
```lua
-- 线程函数
local socket = require "socket"
function download(host,file)
    local c = assert(socket.connect(host,80))
    local count = 0 -- 计算读取的字节数
    local request = string.format(
        "GET %s HTTP/1.0\r\nhost: %s\r\n\r\n",file,host
    )
    c:send(request)
    while true do
        local s,status = receive(c)
        count = count + #s
        if status == "closed" then break end
    end
    c:close()
    print(file,count)
end
```
```lua
-- 非阻塞多线程接收函数
function receive(connection)
    connection:settimeout(0)
    -- receive读取成功则返回字符串,不成功则返回nil外加错误码以及出错前读取到的内容z2
    local s,status,partial = connection:receive(2^10)
    if status == "timeout" then -- 没有足够的数据可以读取时,协程挂起
        cotoutine.yield(connection)
    end
    return s or partial,status
end
```
```lua
-- 调度器实现
-- 可以看成线程池
tasks = {} -- 所有活跃任务的列表

function get(host,file)
    -- 为任务创建协程
    local co = coroutine.wrap(function ()
                                download(host,file)
                              end)
    -- 将其插入列表
    table.insert(tasks,co)
end

function dispatch()
    local i = 1
    local timeout = {}
    while true do
        if tasks[i] == nil then -- 没有其他任务了?
            if tasks[1] == nil then -- 列表为空
                break -- 从循环中退出
            end
        i = 1 -- 否则继续循环
        timeout = {}
        end
        local res = tasks[i]() -- 运行一个任务
        if not res then -- 任务结束?
            table.remove(tasks,i)
        else -- 超时处理
            i = i + 1
            timeout[#timeout + 1] = res
            if #timeout == #tasks then -- 所有任务都被阻塞了？
                socket.select(timeout) -- 等待
        end
    end
end
```
```lua
-- 调用示例
get("www.lua.org","/ftp/lua-5.3.2.tar.gz")
get("www.lua.org","/ftp/lua-5.3.1.tar.gz")
dispatch() -- 主循环
```