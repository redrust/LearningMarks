# 1、调用顺序
## 启动端口监听
- watchdog.start()->gate.open()->gateserver.open()
```lua
-- watchdog.lua
function CMD.start(conf)
	skynet.call(gate, "lua", "open" , conf)
end

-- gateserver.lua
function CMD.open( source, conf )
    assert(not socket)
    local address = conf.address or "0.0.0.0"
    local port = assert(conf.port)
    maxclient = conf.maxclient or 1024
    nodelay = conf.nodelay
    skynet.error(string.format("Listen on %s:%d", address, port))
    socket = socketdriver.listen(address, port)
    socketdriver.start(socket)
    if handler.open then
        return handler.open(source, conf)
    end
end
```

## socket消息接管处理
gateserver直接使用c lib将C层的socket和消息到达等事件封装起来，使其在Lua层能表现出Lua层的行为。
- (1)gateserver注册socket消息类型
    ```lua
    -- 接管socket类型消息
    skynet.register_protocol {
        name = "socket",
        id = skynet.PTYPE_SOCKET,	-- PTYPE_SOCKET = 6
        -- 将传入的消息进行解析：msg是消息的lightdata指针，sz是消息的长度
        unpack = function ( msg, sz )
            return netpack.filter( queue, msg, sz)
        end,
        -- unpack的返回值传入dispatch进行处理，对消息进行分发
        -- 这里的dispatch是skynet.dispatch的内联定义
        -- 参数和作用与skynet.dispatch相同
        dispatch = function (_, _, q, type, ...)
            queue = q
            if type then
                MSG[type](...)
            end
        end
    }
    ```
    所有在socket上到达的消息，都会经过这里定义的消息类型进行处理。消息通过unpack解包，由netpack.filter解析消息，转换成其他消息类型，之后传入dispatch函数中进行分发。  

    ```lua
    function MSG.open(fd, msg)
		handler.connect(fd, msg)
	end

	function MSG.close(fd)
		handler.disconnect(fd)
	end

	function MSG.error(fd, msg)
		handler.error(fd, msg)
	end

	function MSG.warning(fd, size)
		handler.warning(fd, size)
	end
    ```
    上述```MSG[type](...)```也就是上面所罗列的四个MSG函数，负责处理socket上连接、关闭、异常和警告四种特殊事件。
- (2)gateserver处理socket上数据包到达  
    当netpack.filter返回的消息类型是data和more时，进入MSG.data和MSG.more消息处理函数，如下所示：
    ```lua
        -- 分包完成之后恰好有一条完整消息
    	local function dispatch_msg(fd, msg, sz)
		if connection[fd] then
			handler.message(fd, msg, sz)
		else
			skynet.error(string.format("Drop message from fd (%d) : %s", fd, netpack.tostring(msg,sz)))
		end
	end

	MSG.data = dispatch_msg

    -- 分包之后不止一条消息，数据依次压入queue中，之后通过netpack.pop弹出queue中的一条消息
	local function dispatch_queue()
		local fd, msg, sz = netpack.pop(queue)
		if fd then
			-- may dispatch even the handler.message blocked
			-- If the handler.message never block, the queue should be empty, so only fork once and then exit.
			skynet.fork(dispatch_queue)
			dispatch_msg(fd, msg, sz)

			for fd, msg, sz in netpack.pop, queue do
				dispatch_msg(fd, msg, sz)
			end
		end
	end

	MSG.more = dispatch_queue
    ```
    MSG.data和MSG.more调用进入handler.message接口，区别是MSG.data只进入一次，而MSG.more进入多次。

## gate中handler的回调处理
gate作为连接gateserver和watchdog的纽带，起到桥接的作用，通过注册handler的处理函数，将从gateserver上到达的消息，转发给系统内部。
- (1)注册处理函数接收gateserver的回调  
    可以看到注册到gateserver中handler的函数集合，内部实现经常需要再次发送消息给watchdog服务，gate起到沟通连接gateserver和watchdog两个服务，通过网关桥接起了服务器内外部通信。
    ```lua
    --- 当新连接被accept之后，handler的connect会被调用。fd是socket句柄，而不是文件描述符
    ---@param fd socket句柄
    ---@param addr ip地址加端口
    function handler.connect(fd, addr)
        local c = {
            fd = fd,
            ip = addr,
        }
        connection[fd] = c
        skynet.send(watchdog, "lua", "socket", "open", fd, addr)
    end

    local function close_fd(fd)
        local c = connection[fd]
        if c then
            unforward(c)
            connection[fd] = nil
        end
    end

    --- 连接中断时调用
    ---@param fd socket句柄
    function handler.disconnect(fd)
        close_fd(fd)
        skynet.send(watchdog, "lua", "socket", "close", fd)
    end

    --- 连接上出现错误时调用
    ---@param fd socket句柄
    ---@param msg 错误消息
    function handler.error(fd, msg)
        close_fd(fd)
        skynet.send(watchdog, "lua", "socket", "error", fd, msg)
    end

    --- 当fd上待发送的数据积累超过1M时，调用该函数
    ---@param fd socket句柄
    ---@param size integer积累数据长度
    function handler.warning(fd, size)
        skynet.send(watchdog, "lua", "socket", "warning", fd, size)
    end

    ```
- (2)gate上新数据包到达处理  
    gate注册handler.message到gateserver中对socket上新到数据包进行处理回调，全部数据包通过skynet.redirect转发给agent进行解析。
    ```lua
    --- socket上新数据到达
    ---@param fd 文件描述符
    ---@param msg 消息数据内容
    ---@param sz 消息长度
    function handler.message(fd, msg, sz)
        -- recv a package, forward it
        local c = connection[fd]
        local agent = c.agent
        if agent then
            -- It's safe to redirect msg directly , gateserver framework will not free msg.
            skynet.redirect(agent, c.client, "client", fd, msg, sz)
        else
            skynet.send(watchdog, "lua", "socket", "data", fd, skynet.tostring(msg, sz))
            -- skynet.tostring will copy msg to a string, so we must free msg here.
            skynet.trash(msg,sz)
        end
    end
    ```
## watchdog注册接收gate传递的事件消息处理函数
- watchdog是和gate服务器内部直接和gate沟通的第一个服务，需要直接管理socket的状态，在socket状态改变的时候，如果有必要的话要通知关心异常事件的服务，以及确保socket的行为正确以及资源的正确回收。
    ```lua
    -- 关闭连接，清理连接资源，通知gate关闭socket的读事件
    local function close_agent(fd)
        local a = agent[fd]
        agent[fd] = nil
        if a then
            skynet.call(gate, "lua", "kick", fd)
            -- disconnect never return
            skynet.send(a, "lua", "disconnect")
        end
    end

    -- 新连接到时，对应gate的connect
    function SOCKET.open(fd, addr)
        skynet.error("New client from : " .. addr)
        agent[fd] = skynet.newservice("agent")
        skynet.call(agent[fd], "lua", "start", { gate = gate, client = fd, watchdog = skynet.self() })
    end

    -- 连接关闭时，对应gate的disconnect
    function SOCKET.close(fd)
        print("socket close",fd)
        close_agent(fd)
    end

    -- 连接异常时，对应gate的error
    function SOCKET.error(fd, msg)
        print("socket error",fd, msg)
        close_agent(fd)
    end

    -- 连接发送缓冲区满时，对应gate的warning
    function SOCKET.warning(fd, size)
        -- size K bytes havn't send out in fd
        print("socket warning", fd, size)
    end

    -- 连接数据处理，对应gate的message
    function SOCKET.data(fd, msg)
    end
    ```
- watchdog接处理当agent没有定义的消息类型  
    这个函数处理了所有没有在agent中注册的消息类型，可以进行一些自定义的处理行为。
    ```lua
    function SOCKET.data(fd, msg)
    end
    ```

## agent处理gate中传递的消息
- (1)新到连接，创建agent，连接通过agent调用框架内定义的服务  
    在watchdog的SOCKET.open中，进行了skynet.newservice新建了每个连接对应的agent服务，同时skynet.call调用了agent service的初始化函数。在agent初始化函数中，进行了通信协议的加载，注册了心跳检测。
    ```lua
    -- watchdog.lua
    function SOCKET.open(fd, addr)
        skynet.error("New client from : " .. addr)
        agent[fd] = skynet.newservice("agent")
        skynet.call(agent[fd], "lua", "start", { gate = gate, client = fd, watchdog = skynet.self() })
    end

    -- agnet.lua
    function CMD.start(conf)
	local fd = conf.client
	local gate = conf.gate
	WATCHDOG = conf.watchdog
	-- slot 1,2 set at main.lua
	host = sprotoloader.load(1):host "package"
	-- 绑定发送协议，服务器到客户端通信
	send_request = host:attach(sprotoloader.load(2))
	-- 使用上面绑定的发送函数，对客户连接进行心跳检测
	skynet.fork(function()
		while true do
			send_package(send_request "heartbeat")
			skynet.sleep(500)
		end
	end)

	client_fd = fd
	skynet.call(gate, "lua", "forward", fd)
    end
    ```
- (2)agent注册协议和处理函数  
    在agent中注册了client类型的协议，自定义的unpack和dispatch函数。  
    - unpack函数针对redirect传入的消息进行解析，拆包成Lua能够识别的数据格式，这里使用了host:dispatch进行处理，host:dispatch将字符串数据，解析返回，存在两种返回可能，REQUEST和RESPONSE，由第一个返回值决定。
      - REQUEST请求包中，第2个返回值是请求协议函数名，第3个返回值是args，也就是请求函数的参数，如果存在session字段，那么第4个返回值是个用于生成回复包的函数。
      - REPONSE回复包中，第2个返回值是session，第3个返回值是参数列表，也有可能不存在参数列表。
    - dispatch函数将unpack解析好的数据进行转发，调用处理函数获取返回结果。
      - fd:也就是session，这里为了使用上的方便，直接就是fd本身。
      - _:source地址，也就是gate的地址。
      - type:也就是unpack第一个返回值，这个数据包的类型
    - request函数参数列表
      - name:请求函数名
      - args:请求函数参数列表
      - response:生成回复包的函数
    ```lua
    skynet.register_protocol {
	name = "client",
	id = skynet.PTYPE_CLIENT,
	unpack = function (msg, sz)
		return host:dispatch(msg, sz)
	end,
	dispatch = function (fd, _, type, ...)
		assert(fd == client_fd)	-- You can use fd to reply message
		skynet.ignoreret()	-- session is fd, don't call skynet.ret
		skynet.trace()
		if type == "REQUEST" then
			local ok, result  = pcall(request, ...) -- 调用请求处理分发函数，获取返回值
			if ok then
				if result then
					send_package(result)
				end
			else
				skynet.error(result)
			end
		else
			assert(type == "RESPONSE")
			error "This example doesn't support request client"
		end
	end
    }

    -- 请求处理分发函数，三个参数对应REQUEST请求包的三个返回值
    local function request(name, args, response)
        local f = assert(REQUEST[name])
        local r = f(args)
        if response then
            return response(r)
        end
    end

    -- 协议定义的请求处理函数之一
    function REQUEST:get()
        print("get", self.what)
        local r = skynet.call("SIMPLEDB", "lua", "get", self.what)
        return { result = r }
    end
    ```


    