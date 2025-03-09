// 递归实现栈逆序
void reverse(stack<int> &stack) {
    if (stack.empty()) {
        return;
    }
    int i = getAndRemoveLastElement(stack);
    reverse(stack);
    stack.push(i);
}

// 获取栈底元素并移除
int getAndRemoveLastElement(stack<int> &stack) {
    int result = stack.top();
    stack.pop();
    if (stack.empty()) {
        return result;
    } else {
        int last = getAndRemoveLastElement(stack);
        stack.push(result);
        return last;
    }
}
