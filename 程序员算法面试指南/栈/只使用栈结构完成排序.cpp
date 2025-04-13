'''
本质上是使用栈模拟了选择排序
当前处理的元素 cur 与 help 栈中已排序的元素进行比较
如果 help 栈顶元素小于 cur，就将 help 栈顶元素暂时移回 main
重复这个过程，直到找到 help 中第一个不小于 cur 的位置
此时将 cur 放入 help，保证了大小关系

通过示例说明
假设我们有一个已排序的栈 main = [5,3,1]（左侧为栈底，右侧为栈顶），现在要插入元素 2：
main = [5,3,1,2]（插入新元素）
创建空的 help 栈
开始排序:
取出 2: main = [5,3,1], cur = 2, help = []
直接将 2 放入 help: help = [2]
取出 1: main = [5,3], cur = 1, help = [2]
help 顶部是 2，大于 1，不执行内部循环
将 1 放入 help: help = [2,1]
取出 3: main = [5], cur = 3, help = [2,1]
help 顶部是 1，小于 3，将 1 移回 main: main = [5,1], help = [2]
help 顶部是 2，小于 3，将 2 移回 main: main = [5,1,2], help = []
将 3 放入 help: help = [3]
...继续处理...
最终结果: main = [5,3,2,1]（有序，最小元素在栈顶）
通过这种方法，虽然只使用了栈的基本操作（push、pop、top），但成功实现了元素的排序，保证每次插入新元素后，栈都保持有序。
'''

class SortedStack {
public:
    stack<int> main;
    SortedStack() {
        
    }
    
    void push(int val) {
        main.push(val);
        stack<int> help;
        while (!main.empty()) {
            int cur = main.top();
            main.pop();
            while(!help.empty() && help.top() < cur) {
                main.push(help.top());
                help.pop();
            }
            help.push(cur);
        }
        main = help;
    }
    
    void pop() {
        if (main.empty()) {
            return;
        }
        main.pop();
    }
    
    int peek() {
        if (main.empty()) {
            return -1;
        }
        return main.top();
    }
    
    bool isEmpty() {
        return main.empty();
    }
};

/**
 * Your SortedStack object will be instantiated and called as such:
 * SortedStack* obj = new SortedStack();
 * obj->push(val);
 * obj->pop();
 * int param_3 = obj->peek();
 * bool param_4 = obj->isEmpty();
 */



'''
加入dirty标志位，避免重复排序
'''
class SortedStack {
public:
    stack<int> main;
    bool isDirty = false;
    SortedStack() {
        
    }

    void sort() {
        if (!isDirty) {
            return;
        }
        stack<int> help;
        while (!main.empty()) {
            int cur = main.top();
            main.pop();
            while(!help.empty() && help.top() < cur) {
                main.push(help.top());
                help.pop();
            }
            help.push(cur);
        }
        main = help;
        isDirty = false;
    }
    
    void push(int val) {
        main.push(val);
        isDirty = true;
    }
    
    void pop() {
        if (main.empty()) {
            return;
        }
        sort();
        main.pop();
    }
    
    int peek() {
        if (main.empty()) {
            return -1;
        }
        sort();
        return main.top();
    }
    
    bool isEmpty() {
        return main.empty();
    }
};

/**
 * Your SortedStack object will be instantiated and called as such:
 * SortedStack* obj = new SortedStack();
 * obj->push(val);
 * obj->pop();
 * int param_3 = obj->peek();
 * bool param_4 = obj->isEmpty();
 */