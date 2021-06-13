# 1.HelloWorld
- 快速入门的例子，进行LuaEnv类创建和基础使用。  
    ```c#
    LuaEnv luaenv = new LuaEnv(); // 创建C#和Lua进行通信的句柄，一般推荐一个unity程序只有一个
    luaenv.DoString("CS.UnityEngine.Debug.Log('hello world')"); // 进行脚本执行，符合Lua语法规范的文本字符串都可以通过该接口进行执行
    luaenv.Dispose(); // 清除Lua的资源，释放掉句柄，全局Lua句柄唯一的情况下在程序退出时才执行
    ```

# 2.U3DScripting
- 展示怎么用lua来写MonoBehaviour。以下C#代码均来自Examples/02_U3DScriptiong/LuaBehaviour.cs，Lua代码来自Examples/02_U3DScriptiong/LuaTestScript.lua.text  
- 编写C#调用的Lua脚本需要三个步骤。
  - 一：定义注册到Lua脚本中的GameObject对象列表。
    ```c#
    [System.Serializable]
    public class Injection
    {
        public string name;
        public GameObject value;
    }
    ```
  - 二：创建当前脚本关联的环境，注册GameObject对象。
    ```c#
    private LuaTable scriptEnv; // 当前Lua脚本关联的对象
    
    LuaTable meta = luaEnv.NewTable(); // 创建对象

    /*设置元表，绑定全局公共索引*/
    meta.Set("__index", luaEnv.Global);
    scriptEnv.SetMetaTable(meta);
    meta.Dispose();

    /*设置self索引，注册GameObject对象引用*/
    scriptEnv.Set("self", this);
    foreach (var injection in injections)
    {
        scriptEnv.Set(injection.name, injection.value);
    }

    luaEnv.DoString(luaScript.text, "LuaTestScript", scriptEnv); // 执行Lua脚本，注册脚本内置函数
    ```
  - 三：注册Lua函数引用。
    ```c#
    /* C#中对Lua函数的引用都以Action的形式体现 */
    private Action luaStart;
    private Action luaUpdate;
    private Action luaOnDestroy;

    /* 通过Get接口获取Lua脚本中注册的函数 */
    Action luaAwake = scriptEnv.Get<Action>("awake");
    scriptEnv.Get("start", out luaStart);
    scriptEnv.Get("update", out luaUpdate);
    scriptEnv.Get("ondestroy", out luaOnDestroy);
    
    /* 执行Lua中函数的调用 */
    if (luaAwake != null)
    {
        luaAwake();
    }
    ```
    以下是Lua脚本中定义的函数，通过脚本函数调用映射，可以在Lua脚本中实现类似于unity中c#脚本的编写逻辑。
    ```lua
    function start()
        print("lua start...")
        print("injected object", lightObject)
        lightCpnt= lightObject:GetComponent(typeof(CS.UnityEngine.Light))
    end

    function update()
        local r = CS.UnityEngine.Vector3.up * CS.UnityEngine.Time.deltaTime * speed
        self.transform:Rotate(r)
        lightCpnt.color = CS.UnityEngine.Color(CS.UnityEngine.Mathf.Sin(CS.UnityEngine.Time.time) / 2 + 0.5, 0, 0, 1)
    end

    function ondestroy()
        print("lua destroy")
    end
    ```
  - 额外Tips：  
    可以在Update函数中，控制Lua虚拟机的GC时机。
    ```c#
    /* 通过Tick调用主动进行LuaGC操作 */
    if (Time.time - LuaBehaviour.lastGCTime > GCInterval)
    {
        luaEnv.Tick();
        LuaBehaviour.lastGCTime = Time.time;
    }
    ```
    在程序运行结束时，需要主动进行Lua虚拟机的资源的释放，避免内存泄露。
    ```c#
    void OnDestroy()
    {
        if (luaOnDestroy != null)
        {
            luaOnDestroy();
        }
        luaOnDestroy = null;
        luaUpdate = null;
        luaStart = null;
        scriptEnv.Dispose();
        injections = null;
    }
    ```

# 3.UIEvent
- 展示怎么用lua来写UI逻辑。
- 这个例子中的C#代码来自于Examples/02_U3DScripting/LuaBehaviour.cs，不同之处在于这个C#脚本文件被挂载在UI控件上,而且它关联度的GameObject对象换成了canvas中实际存在的Input组件。
    ```lua
    -- 在脚本Start的时候，获取Button组件并为其添加监听函数，每次点击该Button就在Console上输出Input里的内容 
    function start()
        print("lua start...")

        self:GetComponent("Button").onClick:AddListener(function()
            print("clicked, you input is '" ..input:GetComponent("InputField").text .."'")
        end)
    end
    ```
# 4.LuaObjectOrented
- 展示lua面向对象和C#的配合。
- 进行lua面向对象和C#的配合，本节主要讨论C#调用Lua，需要在C#层进行映射绑定Lua的函数，才能执行符合预期的调用。
- C#绑定Lua函数，有以下三种情况：(1)绑定event事件。(2)绑定实体属性和函数。(3)绑定类方法。以下代码均来自Examples/04_LuaObjectOrented/invokeLua.cs：
  - 一：注册event事件参数列表
    ```c#
    /* 注册键值对的event事件参数 */
    public class PropertyChangedEventArgs : EventArgs
    {
        public string name;
        public object value;
    }
    ```
  - 二：注册和Lua面向对象绑定的接口和属性。
    ```c#
    [CSharpCallLua] // invoke lua函数的C#接口都需要该修饰符
    /* 和Lua面向对象表关联的数据结构 */
    public interface ICalc
    {
        event EventHandler<PropertyChangedEventArgs> PropertyChanged;  // 绑定事件修改接口

        int Add(int a, int b); // 实体函数绑定
        int Mult { get; set; } // 实体属性绑定

        object this[int index] { get; set; } // 类属性绑定
    }

    /* 创建对应数据结构的接口 */
    [CSharpCallLua]
    public delegate ICalc CalcNew(int mult, params string[] args);
    ```
  - 三：Lua脚本中注册相关接口函数变量。
    ```lua
    -- 面向对象声明元表 
    local calc_mt = {
        __index = {
            -- Add方法，对应上面ICalc中的Add方法
            Add = function(self, a, b)
                return (a + b) * self.Mult
            end,
            
            --[[
            get_Item和set_Item对应类属性绑定方法
            ]]--
            get_Item = function(self, index)
                return self.list[index + 1]
            end,

            set_Item = function(self, index, value)
                self.list[index + 1] = value
                self:notify({name = index, value = value})
            end,
            
            --[[
            add_PropertyChanged和remove_PropertyChanged对应绑定C#脚本中的PropertyChanged事件列表
            ]]--
            add_PropertyChanged = function(self, delegate)
                if self.notifylist == nil then
                    self.notifylist = {}
                end
                table.insert(self.notifylist, delegate)
                print('add',delegate)
            end,
                                    
            remove_PropertyChanged = function(self, delegate)
                for i=1, #self.notifylist do
                    if CS.System.Object.Equals(self.notifylist[i], delegate) then
                        table.remove(self.notifylist, i)
                        break
                    end
                end
                print('remove', delegate)
            end,

            notify = function(self, evt)
                if self.notifylist ~= nil then
                    for i=1, #self.notifylist do
                        self.notifylist[i](self, evt)
                    end
                end	
            end,
        }
    }

    --[[
        创建Calc对象的接口，设置元表属性和类属性
    ]]--
    Calc = {
        New = function (mult, ...)
            print(...)
            return setmetatable({Mult = mult, list = {'aaaa','bbbb','cccc'}}, calc_mt)
        end
    }
    ```
  - 四：C#脚本中使用。
    ```c#
    luaenv.DoString(script);
    CalcNew calc_new = luaenv.Global.GetInPath<CalcNew>("Calc.New"); // 绑定函数接口
    ICalc calc = calc_new(10, "hi", "john"); //constructor
    Debug.Log("sum(*10) =" + calc.Add(1, 2)); // 进行函数调用
    calc.Mult = 100; // 直接修改对象属性
    Debug.Log("sum(*100)=" + calc.Add(1, 2));

    Debug.Log("list[0]=" + calc[0]); // 获取类属性
    Debug.Log("list[1]=" + calc[1]);

    calc.PropertyChanged += Notify; // 增加event事件监听
    calc[1] = "dddd";
    Debug.Log("list[1]=" + calc[1]);

    calc.PropertyChanged -= Notify; // 注销event事件监听

    calc[1] = "eeee";
    Debug.Log("list[1]=" + calc[1]);
    ```
# 5.NoGC
- 展示怎么去避免值类型的GC。
- 进行Lua调用C#函数时，需要传入当前脚本对象，通过Lua函数调用的方式进行调用C#的函数。
- 进行C#调用Lua函数，为了避免值类型的GC，需要严格定义delegate函数指针，以及在C#脚本中生成值类型，再传入Lua函数中进行函数调用处理。
  - (1)Lua调用C#函数参数中复杂类型
    ```c#
    /* 在C#中对复杂类型进行定义，需要两个修饰符 */
    [GCOptimize] // 值类型GC优化
    [LuaCallCSharp] // Lua能够进行调用修饰
    public struct MyStruct
    {
        public MyStruct(int p1, int p2)
        {
            a = p1;
            b = p2;
            c = p2;
            e.c = (byte)p1;
        }
        public int a;
        public int b;
        public decimal c;
        public Pedding e;
    }
    /* Lua中调用C#接口声明 */
    [CSharpCallLua]
    public delegate MyStruct CustomValueTypeParam(MyStruct p);
    ```
    Lua中进行C#函数调用，传入复杂值类型。
    ```lua
    --[[
        传入的复杂类型以表进行构建，每个属性均以属性名=值进行初始化
    ]]--
    r = monoBehaviour:StructParamMethod({a = 1, b = rnd, e = {c = rnd}})
    assert(r.b == rnd and r.e.c == rnd)
    ```
  - (2)C#中调用Lua函数参数中复杂类型
    ```c#
    /* 1.在C#中声明Lua的函数接口*/
    [CSharpCallLua]
    public delegate int IntParam(int p);

    /* 2.创建函数接口对象 */
    IntParam f1;

    /* 3.进行函数接口绑定，将Lua函数地址绑定到C#函数接口对象上 */
    luaenv.Global.Get("id", out f1);

    /* 4.进行函数调用 */
    f1(1); // primitive type
    ```
    从上面的代码中可以看到，避免LuaGC的调用，需要在C#脚本中使用delegate绑定函数，之后通过在C#中实例化值类型，将实例化的值类型传入，那么值类型就只在C#中进行GC，而不会在Lua中引起第二次GC绑定，因为通过delegate传输的参数，是引用传递，在Lua中不会生成GC标识。

# 6.Coroutine
- 展示lua协程怎么和Unity协程相配合。
- 在Lua中调用coroutine需要导入xlua.util包。
    ```lua
    local util = require 'xlua.util'
    -- 通过调用cs_generator进行创建协程
    util.cs_generator(...)

    -- 创建协程成功之后可以通过对应C#对象进行执行协程，对应的控制调用接口是StartCoroutine和StopCoroutine
    cs_coroutine_runner:StartCoroutine(util.cs_generator(...))
    cs_coroutine_runner:StopCoroutine(coroutine)
    ```   
- 在C#中进行静态列表标签声明
    ```c#
    /* 声明一个静态类，将对应静态属性加上LuaCallCSharp标签 */
    public static class CoroutineConfig
    {
        [LuaCallCSharp]
        public static List<Type> LuaCallCSharp
        {
            /* 设置属性，返回类型列表，给unity内置的类打标签，让Lua语言可以高效的访问 */
            get
            {
                return new List<Type>()
            {
                typeof(WaitForSeconds),
                typeof(WWW)
            };
            }
        }
    }
    ```
- cs_coroutine.lua.txt
    ```lua
    --[[
        创建GameObject对象，给该对象增加Coroutine_Runner脚本组件
    ]]--
    local gameobject = CS.UnityEngine.GameObject('Coroutine_Runner')
    CS.UnityEngine.Object.DontDestroyOnLoad(gameobject)
    local cs_coroutine_runner = gameobject:AddComponent(typeof(CS.XLuaTest.Coroutine_Runner))

    --[[
        返回一个表，声明两个接口函数，提供给外部调用，进行协程的创建和停止
    ]]--
    return {
        start = function(...)
            return cs_coroutine_runner:StartCoroutine(util.cs_generator(...))
        end;

        stop = function(coroutine)
            cs_coroutine_runner:StopCoroutine(coroutine)
        end
    }
    ```
- coroutine_test.lua.txt
```lua
--[[
    创建协程，协程内联执行和测试C#脚本中内置类型的使用
]]--
local a = cs_coroutine.start(function()
    print('coroutine a started')

	coroutine.yield(cs_coroutine.start(function() 
		print('coroutine b stated inside cotoutine a')
		coroutine.yield(CS.UnityEngine.WaitForSeconds(1))
		print('i am coroutine b')
	end))
	print('coroutine b finish')

	while true do
		coroutine.yield(CS.UnityEngine.WaitForSeconds(1))
		print('i am coroutine a')
	end
end)

--[[
    测试携程终止协程运行
]]--
cs_coroutine.start(function()
    print('stop coroutine a after 5 seconds')
	coroutine.yield(CS.UnityEngine.WaitForSeconds(5))
	cs_coroutine.stop(a)
    print('coroutine a stoped')
end)
```

# 7.AsyncTest
- 展示怎么用lua协程来把异步逻辑同步化。
- `util.async_to_sync`:该示例中新使用的接口，通过该接口包裹C#脚本的异步函数，异步操作开始后挂起执行的协程，在异步操作完成之后调用回调函数恢复协程，达到异步调用转换成同步调用的效果。
  - 该函数将两个C#异步函数转换成同步调用
    ```lua
    local sync_alert = util.async_to_sync(CS.XLuaTest.MessageBox.ShowAlertBox)
    local sync_confirm = util.async_to_sync(CS.XLuaTest.MessageBox.ShowConfirmBox) 
    ```
  - 被async_to_sync转换的异步函数，最后一个参数默认为Action，通过Action内的类型数量进行返回返回值。
    ```c#
    public static void ShowAlertBox(string message, string title, Action onFinished = null)
    public static void ShowConfirmBox(string message, string title, Action<bool> onFinished = null)
    ```
  - 进行C#和Lua之间的函数调用时，都需要在白名单中添加声明，才可以正常进行调用。这里添加了Action、Action\<bool>和UnityAction三个函数接口声明，以适配async_to_sync的回调函数。
    ```c#
    [CSharpCallLua]
    public static List<Type> CSharpCallLua = new List<Type>()
    {
        typeof(Action),
        typeof(Action<bool>),
        typeof(UnityAction),
    };
    ```
- 通过上面的async_to_sync进行异步转同步调用，执行同步逻辑。构造同步调用函数，封装异步调用细节，外部调用就像同步调用一样处理。
    ```lua
    --构造alert和confirm函数
    return {
        alert = function(message, title)
            sync_alert(message, title)
        end;
        
        confirm = function(message, title)
            local ret = sync_confirm(message, title)
            return ret == true
        end;
    }
    ```

# 8.Hotfix
- 热补丁的示例，需要开启热补丁特性，详情可以参考文档。


# 9.GenericMethod
- 泛化函数支持的演示。
- 在该示例中，展示了Lua调用C#泛化函数的方法，同时例举出了支持和不支持的泛化函数声明方式。
    ```c#
    // 定义父类
    [LuaCallCSharp]
    public class Foo1Parent
    {
    }
    // 定义子类
    [LuaCallCSharp]
    public class Foo1Child : Foo1Parent
    {
    }
    // 泛型函数声明
    public static T Extension1<T>(this T a) where T : Foo1Parent
    {
        Debug.Log(string.Format("Extension1<{0}>", typeof(T)));
        return a;
    }
    ```
    ```lua
    -- 创建子类对象
    local foo1 = CS.XLuaTest.Foo1Child()
    -- 泛型函数调用
    foo1:Extension1()
    ```
- 动态类类型的成员函数调用上略有不同，examples代码列举的十分详细，对照着看即可。

# 10.SignatureLoader
- 展示如何读取经数字签名的lua脚本，也就是保证传输过程中代码的完整性和可靠性，让脚本不被第三方窃取。
- 具体使用可以参考官方文档signature.md。

# 11.RawObject
- 当C#参数是object时，如何把一个lua number指定以boxing后的int传递过去。
- 代码很简单，只是声明了一个参数为Object的函数，然后往里面传输，Lua脚本可以自动匹配参数，并且会选择最大精度来进行传输。
    ```c#
    public static void PrintType(object o)
    {
        Debug.Log("type:" + o.GetType() + ", value:" + o);
    }

    // Use this for initialization
    void Start()
    {
        LuaEnv luaenv = new LuaEnv();
        //直接传1234到一个object参数，xLua将选择能保留最大精度的long来传递
        luaenv.DoString("CS.XLuaTest.RawObjectTest.PrintType(1234)");
        //通过一个继承RawObject的类，能实现指明以一个int来传递
        luaenv.DoString("CS.XLuaTest.RawObjectTest.PrintType(CS.XLua.Cast.Int32(1234))");
        luaenv.Dispose();
    }
    ```

# 12.ReImplementInLua
- 展示如何将复杂值类型改为lua实现。
- 暂留。