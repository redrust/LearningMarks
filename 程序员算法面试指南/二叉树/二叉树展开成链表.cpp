/*
给定一个二叉树，原地将它展开为一个链表。

解法1：
1. 先序遍历，将每个节点加入到队列中
2. 遍历队列，将每个节点连接到右子树上
3. 将左子树置为nullptr
4. 返回新的链表

解法2：
1. 基于递归，先序遍历
2. 将左子树展开成链表
3. 将右子树展开成链表
4. 将左子树连接到当前根节点
5. 将右子树连接到当前根节点
6. 返回新的链表
*/
class Solution {
public:
    void flatten(TreeNode* root) {
        if(root==nullptr){
            return;
        }
        deque<TreeNode*> q;
        helper(root->left, q);
        helper(root->right, q);
        TreeNode* cur=nullptr;
        TreeNode* pre=root;
        while(q.empty()==false){
            cur=q.front();
            q.pop_front();
            pre->left=nullptr;
            pre->right=cur;
            pre=cur;
        }
    }
    void helper(TreeNode* root, deque<TreeNode*>& q){
        if(root==nullptr){
            return;
        }
        q.push_back(root);
        helper(root->left, q);
        helper(root->right, q);
    }
};