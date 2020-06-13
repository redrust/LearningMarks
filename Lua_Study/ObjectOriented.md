## 3.面向对象编程
- 因为Lua没有类的语法概念，所以通过创建一个专门被用作其他对象的原型对象来创建类．

### 1.创建类
```lua
-- 元类
Shape = {area = 0}

-- 基础类方法 new
function Shape:new (o,side)
  o = o or {}
  setmetatable(o, self) -- 将o设置成调用当前函数的表的元表
  self.__index = self -- 查询元表方法
  side = side or 0
  self.area = side*side;
  return o
end

-- 基础类方法 printArea
function Shape:printArea ()
  print("面积为 ",self.area)
end

-- 创建对象
myshape = Shape:new(nil,10) -- 调用上可以看成myshape = Shape.new(myshape,nil,10)

myshape:printArea() -- getmetatable(myshape).__index.printArea(myshape)
```

### 2.继承
- 继承了一个简单的类，来扩展派生类的方法，派生类中保留了继承类的成员变量和方法
```lua
Square = Shape:new()
-- 派生类方法 new
function Square:new (o,side)
  o = o or Shape:new(o,side) -- 如果元表存在，则使用元表，否则，从基类创建一个新的元表
  setmetatable(o, self)
  self.__index = self
  return o
end

-- 派生类方法 printArea
function Square:printArea ()
  print("正方形面积为 ",self.area)
  self:printSide()
end

function Square:printSide ()
    return self.printSide or 0
end

-- 创建对象
mysquare = Square:new(nil,10)

-- 定制派生类对象的专属函数实现
function mysquare:printSide()
    print("正方形边长为 ",math.sqrt(self.area))
end

mysquare:printArea()
```

### 3.多继承
- 子类拥有多个父类对象,在Lua中则是拥有两个元表
```lua
-- 在表plist的列表中查找k
local funcion search(k,plist)
  for i = 1,#plist do
    local v = plist[i][k]
    if v then return v end
  end
end

function createClass(...)
  local c = {} -- 新类
  local parents = {...} -- 父类列表

  -- 在父类列表中查找类缺失的方法
  setmetatable(c,{__index = function(t,k)
                  return search(k,parents)
                  end})

  -- 将c作为其实例的元表
  c.__index = c

  -- 为新类定义一个新的构造函数
  function c:new(o)
    o = o or {}
    setmetatable(o,c)
    return o
  end
  
  return c -- 返回新类
end

Named = {}

function Named:getname()
  return self.name
end

function Named:setname(n)
  self.name = n
end

-- 同时继承两个父类
NamedAccount = createClass(Account,Named)

account = NamedAccount:new{name = "Paul"}
print(account:getname()) --> Paul
```

### 4.私有性
- 一个表用来保存对象的状态，另一个表用于保存对象的操作(或接口)
```lua
function newAccount(iniialBalance)
  -- 私有属性
  local self = {balance = initialBalance}

  -- 公开方法
  local withdraw = function(v)
                    self.balance = self.balance - v
                   end

  local deposit = function(v)
                    self.balance = self.balance + v
                  end

  local getBalance = function () return self.balance end

  -- 打包成一个表，提供给外部进行操作
  return {
    withdraw = withdraw,
    deposit = deposit,
    getBalance = getBalance
  }
end

-- 使用
acc1 = newAccount(100.00)
acc1.withdraw(40)
print(acc1.getBalance()) --> 60
```

### 5.两种特殊的实现
- 单方法对象：将单独的方法以对象的表现形式返回
- 对偶表示：把表当作键,同时又把这个对象本身当作这个表的键

## 4.环境
- 全局变量_G:保存有Lua所有全局变量的一张表
```lua
-- 使用全局表来保存动态名称变量
-- 获取保存的动态变量
function getfield(f)
  local v = _G
  for w in string.gmatch(f,"[%a_][%w_]*") do
    v = v[w]
  end
  return v
end

-- 设置动态变量
function setfield(f,v)
  local t = _G -- 从全局表开始
  for w,d in string.gmatch(f,"([%a_][%w_]*)(%.?)") do
    if d == "." then -- 不是最后一个名字?
      t[w] = t[w] or {} -- 如果不存在则创建表
      t = t[w]
    else
      t[w] = v -- 进行赋值
    end
  end
end

-- 使用
setfield("t.x.y",10)

print(t.x.y) -->10
print(getfield("t.x.y")) -->10
```

### 1.限制全局变量的访问
- 设置元表，当访问不存在的全局变量时提示错误
```lua
setmetatable(_G,{
  __newindex = function(_,n)
    error("attempt to write undeclared variable" .. n,2)
  end,
  __index = function(_,n)
    error("attempt to write undeclared variable" .. n,2)
  end,
})

-- 设置了元表之后，要使用定制的方法进行新变量的声明
function declare(name,initval)
  rawset(_G,name,initval or false)
end
```

### 2._ENV变量
- 一个预定义上值(外部的局部变量)存在的情况下编译所有的代码段的
- _ENV与_G相似，但是不相同．_ENV指向当前的环境，_G指向全局环境
- `_ENV=nil`会使后续语句都不能直接访问全局变量

## 5.垃圾收集机制
### 1.弱引用表
- 允许收集Lua语言中还可以被程序访问的对象
- 弱引用与强引用对立，弱引用只引用对象，而不持有对象的生命周期，强引用就像铁丝绑定，而弱引用是棉花丝绑定
- 只有对象可以从弱引用表中被移除
- 一个表是否为弱引用表是由其元表中__mode字段所决定的,该字段的值为k,则是键为弱引用，字段值为v，则为值弱引用，字段值为kv，则为键和值都是弱引用的
```lua
a = {}
mt = {__mode = "k"}
setmetatable(a,mt) -- 现在a的键是弱引用的
key = {}
a[key] = 1
key = {}
a[key] = 2
collectgarbage() -- 强制进行垃圾回收
for k,v in pairs(a) do print(v) end -->2
```
- 1．记忆函数
  - 通过记忆函数的执行结果，在后续使用相同参数再次调用该函数直接返回之前记忆的结果，来加快函数的运行速度
  ```lua
  local results = {}
  setmetatable(results,{__mode = "v"}) -- 让值成为弱引用的
  function mem_loadstring(s)
    local res = results[s]
    if res == nil then -- 已经有结果吗
      res = assert(load(s)) -- 计算新结果
      results[s] = res
    end
    return res
  end
  ```
- 2.记忆函数和默认值表
  - 使用弱引用表来映射每一个表和它的默认值
  ```lua
  local default = {}
  setmetatable(default,{__mode = "k"}) -- 让键成为弱引用的
  local mt = {__index = function(t) return defaults[t] end}
  function setDefault(t,d)
    defaults[t] = d
    setmetatable(t,mt)
  end
  ```
- 3.瞬表
  - 具有弱引用的键的表中的值又引用了对应的键
  ```lua
  do
    local mem = {} -- 记忆表
    setmetatable(mem,{__mode = "k"})
    function factory(o)
      local res = mem[o]
      if not res then
        res = (function () return o end)
        mem[o] = res
      end
      return res
    end
  end
  ```
### 2.析构器
- 允许收集不在垃圾收集器直接控制下的外部对象
- 析构器是一个与对象关联的函数，当该对象即将被回收时该函数会被调用
- 通过元方法__gc实现析构器
```lua
o = {x = "hi"}
setmetatable(o,{__gc = function(o) print(o.x) end})
o = nil
collectgarbage() -->hi
```
### 3.函数cllectgarbafe
- 允许控制垃圾收集器的步长
- 两个参数：第一个参数是一个可选的字符串，用来明可以进行何种操作．有些选项使用一个整形作为第二参数，来对操作进行定制

### 4.垃圾收集器
- Lua会时不时地停止主程序运行来执行一次完整的垃圾回收周期
- 1.标记
  - 把根节点结合标记为活跃，根节点集合由Lua语言可以直接访问的对象组成
- 2.清理
  - 处理析构器和弱引用表
  - 遍历需要进行析构，而且不活跃的对象,将其标记为活跃，放入析构列表中
  - 然后遍历弱引用表，移除键或值未被标记的元素
- 3.清除
  - 遍历所有对象，如果一个对象没有标记为活跃，Lua就将其回收，否则清理标记．
- 4.析构
  - 调用清理阶段被分离出的对象的析构器

- Lua5.1引入增量式垃圾收集器，不主动停止程序运行，当分配到一定数量内存时，收集器也执行一小步
- Lua5.2引入紧急垃圾收集，当内存分配失败时，强制进行一次完整的垃圾收集
