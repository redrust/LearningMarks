/*
一颗二叉搜索树，被调换两个节点，返回被调换的两个节点值

被调换的节点，存在两种情况
1. 相邻节点被调换，中序遍历的时候，存在两个连续的节点，值是递减的
2. 不相邻节点被调换，中序遍历的时候，存在两次递减序列

综合以上两种情况，
第一次递减序列的第一个节点，是第一个被调换的节点
第二次递减序列的第二个节点，是第二个被调换的节点
*/
class Solution {
public:
    vector<TreeNode*> getTwoSwappedNodes(TreeNode* root) {
        vector<TreeNode*> result{nullptr, nullptr};
        if(root==nullptr){
            return result;
        }
        stack<TreeNode*> s;
        TreeNode* cur=root;
        TreeNode* pre=nullptr;
        while(cur!=nullptr || !s.empty()){
            while(cur!=nullptr){
                s.push(cur);
                cur=cur->left; 
            }
            cur=s.top();
            s.pop();
            if(pre!=nullptr && pre->val>cur->val){
                result[0]=result[0]==nullptr?pre:result[0];
                result[1]=cur;
            }
            pre=cur;
            cur=cur->right;
        }
        return result;
    }
};