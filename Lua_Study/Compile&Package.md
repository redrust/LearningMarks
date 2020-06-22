## 14.编译．执行和错误
### 1.编译
- 1.loadfile
  - 从文件中加载Lua代码段，但是不运行代码，只是编译代码然后将编译后的代码段作为一个函数返回．
- 2.load
  - 从字符串中加载代码段

### 2.预编译
- 使用luac指令可以将lua源码编译成二进制文件
- `luac -o prog.lc prog.lua`

### 3.错误处理
- 1.error:传入错误信息作为参数来引发一个错误．第二个可选参数level，用于指出向函数带哦用层次中哪层函数报告错误．
- 2.assert:传入两个参数，第一个参数为真则返回该参数，如果为假则引发一个错误．第二个参数是一个可选的错误信息
- 3.pcall
  - pcall执行完成之后，如果没有错误发生，返回true和包裹函数的返回值，否则返回false和错误信息．
  ```lua
  local ok,msg = pcall(function()
                        some code
                        if unexpected_condition then error() end
                        some code
                      end)
  if ok then -- 执行被保护的代码时没有错误发生
    regular code
  else -- 执行被保护的代码时有错误发生：进行恰当的处理
    error-handling code
  end
  ```
#### 4.xpcall
- 第二个参数是一个消息处理函数，当发生错误时，Lua会先调用这个函数来获取有关错误的更多信息
- 消息处理函数
  - 1.debug.debug
    - 提供一个Lua提示符来让用户检查错误发生的原因
  - 2.debug.traceback
    - 使用调用栈来构造详细的错误信息

## 8.模块和包
- 一个模块就是一些代码，这些代码可以通过函数require加载，然后创建和返回一个表,其本质是一个预先定义好的表
- 可以很轻松的实现包管理和命名空间定义
```lua
-- 可以这样使用math库
local m = require "math";
print(m.sin(3.14));
```
### 1.require
- 传入模块名作参数，加载对应模块．只有一个括号时，可以省略括号
#### 加载操作过程
  - 1.在package.loaded中检查模块是否被加载．如果被加载，就返回相应的值，否则执行2.
  - 2.在package.path指定的路径中搜索具有指定模块名的Lua文件．如果找到了对应文件，那么就使用loadfile将其进行加载，结果返回一个加载器(loader)的函数.如果找不到对应文件，那么搜索相应名称的C标准库，使用package.loadlib进行加载C标准库．
  - 3.如果正常加载成功，那么require返回加载函数，同时将其保存在package.loaded中，以便将来加载同一个模块时返回相同的值.

### 2.搜索路径
- 使用模板路径，其中每个模板都指定了将模块名转换为文件名的方式
- 每一个模板都是一个包含可选问好的文件名
- 函数require会使用模块名来替换每一个问号
```lua
-- 有如下路径
?;?.lua;c:\windows\?;/usr/local/lua/?/?.lua
-- 当有这个调用时
require "sql"
-- 会被翻译成如下路径
sql
sql.lua
c:\windows\sql
/usr/local/lua/sql/sql.lua
```

### 3.编写模块
- 第一种创建方法：创建一个表并将其所有需要导出的函数放入其中，最后返回这个表
- 第二种创建方法：把所有函数定义为局部变量，然后在最后构造返回的表
- 一个用于复数的简单模块.
```lua
local M = {}; -- 模块名

-- 创建一个新的复数
local function new(r,i)
  return { r = r,i = i};
end

M.new = new; -- 把'new'加到模块中

-- constant 'i'
M.i = new(0,1);

function M.add(c1,c2)
  return new(c1.r + c2.r , c1.i + c2.i);
end

function M.sub(c1,c2)
  return new(c1.r - c2.r , c1.i - c2.i);
end

function M.mul(c1,c2)
  return new(c1.r * c2.r - c1.i * c2.i , c1.r*c2.i + c1.i*c2.r );
end

local function inv(c)
  local n = c.r^2 + c.i^2;
  return new(c.r/n,-c.i/n);
end

function M.div(c1,c2)
  return M.mul(c1,inv(c2));
end

function M.tostring(c)
  return string.format("(%g,%g)",c.r,c.i);
end

return M;
```