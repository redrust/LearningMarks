## 5.数组结构
### 1.数组
- 使用整数来索引表即可实现数组
- 一般使用１作为数组的起始索引
```lua
local a = {};
for i = 1,1000 do
  a[i] = 0;
end
```

### 2.多维数组
- 显式的创建每一行元素
```lua
local mt = {};
for i = 1,N do
  local row = {};
  mt[i] = row;
  for j = 1,M do
    row[j] = 0;
  end
end
```
- 将两个索引合并成一个来创建多维数组
```lua
local mt = {};
for i = 1,N do
  local aux = (i - 1 ) * M;
  for j = 1,M do
    mt[aux + j] = 0;
  end
end
```

### 3.链表
```lua
-- 头节点
list = nil;
-- 子节点
list = {next = list,value = v};
-- 遍历链表
local l = list;
while l do
  visit l.value;
  l = l.next;
end
```

### 4.队列及双端队列
```lua
-- 返回一个初始化的list
-- 链表结构
--[[
 first                  last
{  1  ,  2  , 3  ,  4  ,  5  }
]]--
function listNew()
  return {first = 0,last = -1};
end

-- 从头部插入
function pushFirst(list,value)
  local first = list.first - 1;
  list.first = first;
  list[first] = value;
end

-- 从尾部插入
function pushLast(list,value)
  local last = list.last + 1;
  list.last = last;
  list[last] = value;
end

-- 从头部删除数据
function popFirst(list)
  local first = list.first;
  if first > list.last then
    error("list is empty");
  end
  local value = list[first];
  list[first] = nil; --使得元素能够被垃圾回收
  list.first = first + 1;
  return value;
end

-- 从尾部删除数据
function popLast
  local last = list.last;
  if list.first > last then
    error("list is empty");
  end
  local value = list[last];
  list[last] = nil;
  list.last = last - 1;
  return value;
end
```

### 5.反向表
- 索引与值恰好反过来存储的表
```lua
function reverse(origin)
reverse_table = {};
for k,v in pairs(origin) do
  reverse_table[v] = k;
end
```

### 6.包
- 使用反向表可以制作一种结构变形
- 其中元素可以出现多次，每个键有一个计数器
```lua
-- 向包中插入元素，如果存在增加其计数器
function insert(bag,element)
  bag[element] = (bag[element] or 0 ) + 1;
end

-- 从包中删除元素，计数器减为０则清除元素
function remove(bag,element)
  local count = bag[element];
  bag[element] = (count and count > 1) and count - 1 or nil;
end
```

## 6.数据文件和序列化
### 1.数据文件
- 将数据结构化成Lua构造器能够直接处理的型别
```lua
--以下数据文件就可以转变成Entry进行处理
Donald E. Knuth,Literate Programming,CSLT,1992
-->>>>
Entry{"Donald E. Knuth",
  "Literate Programming",
  "CSLT",
  1992
}
```
### 2.序列化
- 将序列化后的数据表示为Lua代码，当这些代码运行时，被序列化的数据就可以在读取程序中得到重建．
```lua
function basicSerialize(o)
  --假设'o'是一个数字或者字符串
  return string.format("%q",o);
end

function save(name,value,saved)
  saved = saved or {};--初始值
  io.write(name," = ");
  if type(value) == "number" or type(value) == "string" then
    io.write(basicSerialize(value,"\n"));
  elseif type(value) == "table" then
    if saved[value] then --值是否已被保存？
      io.write(saved[value],"\n");--使用之前的名称
    else
      saved[value] = name;--保存名称以供后续使用
      io.write("{}\n"); --创建新表
      for k,v in pairs(value) do -- 保存表的字段
        k = basicSerialize(k);
        local fname = string.format("%s[%s]",name,k);
        save(fname,v,saved)
      end
    end
  end
  else
    error("connnot save a " .. type(value));
  end
end
end
```