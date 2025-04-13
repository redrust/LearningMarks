/*
获取完全二叉树节点数量
1. 首先获取完全二叉树的最左节点的层数
2. 然后使用二分查找的思想，查找最后一个节点
3. 如果最后一个节点的最左节点的层数等于最左节点的层数，则说明最后一个节点在右子树，那么左子树是满二叉树，可以使用公式2^n-1计算节点数量
4. 如果最后一个节点的最左节点的层数不等于最左节点的层数，则说明最后一个节点在左子树，那么右子树是满二叉树，节点数量为2^(n-l-1)-1(注意这里是n-l-1)
5. 递归查找最后一个节点

*/
class Solution {
    public:
        int countNodes(TreeNode* root) {
            if(root==nullptr){
                return 0;
            }
            return bsp(root, 1, getMostLeftLevel(root, 1));
        }
        int bsp(TreeNode* root, int curLevel, int maxLeftLevel){
            if(curLevel==maxLeftLevel){
                return 1;
            }
            if(getMostLeftLevel(root->right, curLevel+1)==maxLeftLevel){
                return (1<<(maxLeftLevel-curLevel))+bsp(root->right, curLevel+1, maxLeftLevel);
            }
            else{
                return (1<<(maxLeftLevel-curLevel-1))+bsp(root->left, curLevel+1, maxLeftLevel);
            }
        }
    
        int getMostLeftLevel(TreeNode* root, int curLevel){
            while(root!=nullptr){
                ++curLevel;
                root=root->left;
            }
            return curLevel-1;
        }
    };