/*
给定一个二叉树, 找到该树中两个指定节点的最近公共祖先。

递归法：
1. 如果当前节点为空，或者当前节点为p或q，则返回当前节点
2. 递归遍历左子树和右子树，如果左子树和右子树都存在，则返回当前节点
3. 如果左子树存在，则返回左子树
4. 如果右子树存在，则返回右子树
*/
class Solution {
public:
    TreeNode* lowestCommonAncestor(TreeNode* root, TreeNode* p, TreeNode* q) {
        if(root==NULL||root==p||root==q){
            return root;
        }
        TreeNode* left=lowestCommonAncestor(root->left, p, q);
        TreeNode* right=lowestCommonAncestor(root->right, p, q);
        if(left!=NULL&&right!=NULL){
            return root;
        }
        return left!=NULL?left:right;
    }
};

/*
使用一个map记录每个节点的父节点，然后从p节点开始，向上遍历，记录每个访问过的节点，然后从q节点开始，向上遍历，如果遇到已经访问过的节点，则返回该节点
*/
class Solution {
public:
    TreeNode* lowestCommonAncestor(TreeNode* root, TreeNode* p, TreeNode* q) {
        if(root==nullptr){
            return nullptr;
        }
        unordered_map<TreeNode*,TreeNode*> m;
        m[root]=nullptr;
        buildMap(root->left, root, m);
        buildMap(root->right, root, m);

        unordered_set<TreeNode*> s;
        TreeNode* cur=p;
        while(cur!=nullptr){
            s.insert(cur);
            cur=m[cur];
        }

        cur=q;
        while(cur!=nullptr){
            auto parent=s.find(cur);
            if(parent!=s.end()){
                return *parent;
            }
            cur=m[cur];
        }
        return nullptr;
    }
    void buildMap(TreeNode* cur, TreeNode* parent, unordered_map<TreeNode*,TreeNode*>& m){
        if(cur==nullptr){
            return;
        }
        m[cur]=parent;
        buildMap(cur->left, cur, m);
        buildMap(cur->right, cur, m);
    }
};