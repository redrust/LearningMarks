/*
给定一棵二叉树的根节点，请找出该二叉树中每一行最大的值。

输入：
[1,3,2,5,3,null,9]

输出：
[1,3,9]

使用一个队列，一个last指针，一个nLast指针，一个maxNum变量
last指针指向当前行最后一个节点，nLast指针指向下一行最后一个节点，maxNum变量指向当前行最大值

层序遍历，将每一行的节点加入队列，然后比较当前行的最大值，如果当前行的最大值大于maxNum，则更新maxNum
如果当前行遍历完了，则将maxNum加入result，然后更新last指针为nLast指针，maxNum为INT_MIN
*/
class Solution {
public:
    vector<int> largestValues(TreeNode* root) {
        vector<int> result;
        if(root==nullptr){
            return result;
        }
        queue<TreeNode*> q;
        TreeNode* cur=nullptr;
        TreeNode* last=root;
        TreeNode* nLast=nullptr;
        int maxNum=INT_MIN;
        q.push(root);
        while(q.empty()==false){
            cur=q.front();
            q.pop();
            if(cur->left!=nullptr){
                q.push(cur->left);
                nLast=cur->left;
            }
            if(cur->right!=nullptr){
                q.push(cur->right);
                nLast=cur->right;
            }
            if(cur->val>maxNum){
                maxNum=cur->val;
            }
            if(cur==last){
                result.push_back(maxNum);
                last=nLast;
                maxNum=INT_MIN;
            }
        }
        return result;
    }
};