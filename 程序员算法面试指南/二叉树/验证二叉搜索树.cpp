/**
 * 验证二叉搜索树
 * 
 * 二叉搜索树，左子树所有节点小于根节点，右子树所有节点大于根节点
 * 
 * 中序遍历，左子树->根节点->右子树
 * 中序遍历的顺序是升序的
 * 如果遍历过程中，发现当前节点小于前一个节点，则不是二叉搜索树
 * 
 */
class Solution {
public:
    int pre=INT_MIN;
    bool isInit=false;
    bool isValidBST(TreeNode* root) {
        if(root==nullptr){
            return false;
        }
        return helper(root);
    }

    bool helper(TreeNode* root){
        if(root==nullptr){
            return true;
        }
        bool result=helper(root->left);
        if(root->val <= pre && isInit){
            return false;
        }
        pre=root->val;
        isInit=true;
        result&=helper(root->right);
        return result;
    }
};