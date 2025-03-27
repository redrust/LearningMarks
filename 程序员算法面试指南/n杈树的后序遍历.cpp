/*
使用两个栈，将根节点入栈1，然后依次将根节点的子节点入栈1，
然后依次将栈1的元素出栈，并入栈2，最后将栈2的元素出栈，即为后序遍历的结果

比如：
    1
   / \
  3   2
 / \
 4   5

S1:3,2,1
S2:1,2,3

后序遍历结果为：4 5 3 2 1
*/

class Solution {
public:
    vector<int> postorder(Node* root) {
        vector<int> result;
        if(root==nullptr){
            return result;
        }
        stack<Node*> s1;
        stack<Node*> s2;
        Node* cur=nullptr;
        s1.push(root);
        while(s1.empty()==false){
            cur=s1.top();
            s1.pop();
            s2.push(cur);
            for(auto& item:cur->children){
                s1.push(item);
            }
        }
        while(s2.empty()==false){
            cur=s2.top();
            s2.pop();
            result.push_back(cur->val);
        }
        return result;
    }
};