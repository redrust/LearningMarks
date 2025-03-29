/*
暴力解法
遍历树，记录每个层级的节点值
最后返回哈希表中最后一层的第一个节点值
*/
class Solution {
public:
    int maxLayer=0;
    int findBottomLeftValue(TreeNode* root) {
        if(root==nullptr){
            return 0;
        }
        unordered_map<int,vector<int>> m;
        helper(root, 0, m);
        return m[maxLayer][0];
    }

    void helper(TreeNode* root, int layer, unordered_map<int,vector<int>>& m){
        if(root==nullptr){
            return;
        }
        if(m.find(layer)==m.end()){
            m[layer]=vector<int>();
        }
        maxLayer=max(layer, maxLayer);
        m[layer].push_back(root->val);
        helper(root->left, layer+1, m);
        helper(root->right, layer+1, m);
    }
};