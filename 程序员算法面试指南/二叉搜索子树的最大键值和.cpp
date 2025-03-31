/*
给定一个二叉树，请计算它的所有子树中，键值和最大的二叉搜索子树的键值和

求解思路：
1. 判断当前节点是否是二叉搜索树
2. 如果是二叉搜索树，则计算当前节点的键值和
3. 如果不是二叉搜索树，则递归计算左右子树的键值和
4. 返回左右子树中键值和最大的一个

需要注意：
1. 当前节点是二叉搜索树，需要满足当前节点的左子树和右子树都是二叉搜索树
2. 求出左右子树的最大子树，如果左右子树的最大节点恰好是当前节点的左右子节点，则当前节点是二叉搜索树，需要将当前节点加入形成一颗更大的二叉搜索树
3. 需要求出左子树的最大值和右子树的最小值，才能判断左右子树+当前节点是否是二叉搜索树
*/
class Solution {
public:
    class NodeInfo{
    public:
        int max;
        int min;
        int sum=0;
        TreeNode* maxHead;
        NodeInfo(int max_, int min_, int sum_, TreeNode* maxHead_):max(max_),min(min_),sum(sum_),maxHead(maxHead_){}
    };
    int maxSumBST(TreeNode* root) {
        NodeInfo info=helper(root);
        return info.sum>0?info.sum:0;
    }
    NodeInfo helper(TreeNode* root){
        if(root==nullptr){
            return NodeInfo(INT_MIN, INT_MAX, 0, nullptr);
        }
        NodeInfo leftInfo=helper(root->left);
        NodeInfo rightInfo=helper(root->right);
        int maxNum=max(root->val, max(leftInfo.max, rightInfo.max));
        int minNum=min(root->val, min(leftInfo.min, rightInfo.min));
        int sum=max(leftInfo.sum, rightInfo.sum);
        TreeNode* maxHead=leftInfo.sum>rightInfo.sum?leftInfo.maxHead:rightInfo.maxHead;
        if(leftInfo.maxHead==root->left&&rightInfo.maxHead==root->right&&root->val>leftInfo.max&&root->val<rightInfo.min){
            sum=max(leftInfo.sum+rightInfo.sum+root->val, sum);
            maxHead=root;
        }
        return NodeInfo(maxNum,minNum,sum,maxHead);
    }
};