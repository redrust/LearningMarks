## skynet.start(func)
- 用func函数初始化服务，并将消息处理函数注册到C层，让该服务可以工作。
- 服务初始化时将相应的处理函数注册到skynet框架中。

## skynet.fork(func)
- 启动一个新的任务去执行函数func

## skynet.call(address, type, ...)
- 用type类型发送一个消息到address，并等待对方回应。

## skynet.send(address, type, ...)
- 用type类型发送一个消息到address，非阻塞调用。

## skynet.dispatch(type, function(session, source, ..))
- 进行注册特定类消息的处理函数。

## skynet.redict(addr, sourcec, type, ...)
- 伪装成source地址向addr地址发送一个消息，没有返回值。

## skynet.exit()
- 用于退出当前服务，skynet.exit()之后的代码都不会被运行，而且当前服务被阻塞的coroutine也会立刻中断退出。由于每个Service在的执行都是通过coroutine进行恢复调用处理的，所以skynet.exit()也可以理解成Service退出当前coroutine，并且绝对不会再次重新申请调用coroutine。

## skynet.register_protocol
```lua
skynet.register_protocol{ 
  name = "text",
  id = skynet.PTYPE_TEXT,
  pack = function(m) return tostring(m) end,
  unpack = skynet.tostring, 
  dispatch = function (_, _, q, type, ...)end
  }
```
- name:新的类别名
- id:新的消息类别
- pack:用于消息的编码，必须返回string或者userdata和size
- unpack:用于消息解码，参数为lightuserdata和一个整数，也就是上面提到的消息指针和消息长度，skynet.tostring将消息指针和长度翻译成Lua的string
- dispatch:将unpack解码的消息进行转发，这里的dispatch是skynet.dispatch的内联定义，参数和作用与skynet.dispatch相同

## socketdriver.start(fd)
- 当一个服务想读写fd前，需要调用socketdriver.start(fd)。
- 该文件描述符在单一skynet节点上独立唯一存在，全局Service可以直接通过消息传递使用。
- 当一个Service对fd进行start调用之后，以后在该fd上到达的数据会优先传递到该Service上，如果有多个Service先后Start了同一个fd，最后的Service会获得到达数据回调通知，原先的Service失去对fd的控制权。

## netpack.filter(queue, msg, size)
- 返回一个type(“data”, “more”, “error”, “open”, “close”)代表具体IO事件，其后返回每个事件所需参数。
- 包格式是2字节头+数据内容，字节头采用大端编码。
- 该函数也进行分包的操作，针对data的数据包，会尽可能的分包。
- 该函数内部使用哈希表记录每个套接字对应的粘包，在下次数据到达时，取出上次留下的粘包数据，重新分包。