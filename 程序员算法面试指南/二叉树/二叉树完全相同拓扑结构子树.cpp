/*
给定两棵二叉树的头节点head1和head2，
判断head1中是否存在一棵子树的拓扑结构和head2完全相同

暴力解法：
1. 以head1的每个节点为根节点，判断是否存在一棵子树的拓扑结构和head2完全相同
2. 如果存在，返回true
3. 如果遍历完head1的每个节点，都没有找到和head2完全相同的子树，返回false

时间复杂度：O(N*M)，N是head1的节点数，M是head2的节点数
空间复杂度：O(N)，递归栈的空间
*/
class Solution {
public:
    bool isSubtree(TreeNode* root, TreeNode* subRoot) {
        if(root==nullptr){
            return false;
        }
        return helper(root, subRoot) || isSubtree(root->left, subRoot) || isSubtree(root->right, subRoot);
    }
    bool helper(TreeNode* root, TreeNode* subRoot){
        if(subRoot==nullptr&&root==nullptr){
            return true;
        }
        if(subRoot==nullptr||root==nullptr||root->val!=subRoot->val){
            return false;
        }
        return helper(root->left, subRoot->left)&&helper(root->right, subRoot->right);
    }
};