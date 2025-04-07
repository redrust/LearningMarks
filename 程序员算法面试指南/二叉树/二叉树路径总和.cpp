/**
 * 给定一个二叉树，返回所有任意路径，使得每个路径上节点值的和为给定的目标值。
 * 路径不需要从根节点开始，也不需要在叶子节点结束，但是路径方向必须是向下的（只能从父节点到子节点）。
 * 
 * 这道题也可以看作是求解二叉树中任意两个节点之间的路径和为targetSum的数量，也可以抽象的看作每条路径都是一个数组，求解这个数组中和为targetSum的子数组数量
 * 
 * 使用一个哈希表，记录每个前缀和出现的次数
 * 遍历数组，计算当前前缀和，如果前缀和减去targetSum在哈希表中存在，则说明存在一个子数组，其和为targetSum
 * 
 * 当以当前节点为根节点的子树遍历完时，需要将其sum的出现次数减去1，否则会影响其他路径的计算
 * 并且当sum==targetSum时，result需要加1，这里处理的是从根节点到当前节点的路径和为targetSum的情况
 */
class Solution {
public:
    int pathSum(TreeNode* root, int targetSum) {
        int result=0;
        unordered_map<long int,int> m;
        long int sum=0;
        helper(root, targetSum, result, sum, m);
        return result;
    }

    void helper(TreeNode* root, int targetSum, int& result, long int sum, unordered_map<long int,int>& m){
        if(root==nullptr){
            return;
        }
        sum+=root->val;
        if(m.find(sum-targetSum)!=m.end()){
            result+=m[sum-targetSum];
        }
        if(sum==targetSum){
            result+=1;
        }
        m[sum]+=1;   
        helper(root->left, targetSum, result, sum, m);
        helper(root->right, targetSum, result, sum, m);
        m[sum]-=1;
    }
};