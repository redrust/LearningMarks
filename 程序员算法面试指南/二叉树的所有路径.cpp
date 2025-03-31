/*
返回所有从根节点到叶子节点的路径

输入:

   1
 /   \
-5    4
 \     \
  -3     6
*/
class Solution {
public:
    vector<string> binaryTreePaths(TreeNode* root) {
        vector<string> result;
        string path;
        helper(root, result, path);
        return result;
    }
    void helper(TreeNode* root, vector<string>& result, string path){
        if(root==nullptr){
            return;
        }
        if(root->left==nullptr&&root->right==nullptr){
            path+=to_string(root->val);
            result.push_back(path);
            return;
        }
        path+=to_string(root->val)+"->";
        helper(root->left, result, path);
        helper(root->right, result, path);
    }
};