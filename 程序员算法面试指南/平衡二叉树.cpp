/*
判断一棵二叉树是否是平衡二叉树

使用递归遍历每个节点，判断每个节点的左右子树是否是平衡二叉树，并记录每个节点的左右子树的高度

时间复杂度：O(N)，N是二叉树的节点数
空间复杂度：O(N)，递归栈的空间
*/
class Solution {
public:
    class Return{
    public:
        bool isBalance=false;
        int height=0;
    };
    bool isBalanced(TreeNode* root) {
        return helper(root).isBalance;
    }
    Return helper(TreeNode* root){
        if(root==nullptr){
            return Return{true,0};
        }
        Return leftData=helper(root->left);
        Return rightData=helper(root->right);
        bool isBalance=leftData.isBalance&&rightData.isBalance&&abs(leftData.height-rightData.height)<=1;
        int height=leftData.height>rightData.height?leftData.height:rightData.height;
        height+=1;
        return Return{isBalance,height};
    }
};