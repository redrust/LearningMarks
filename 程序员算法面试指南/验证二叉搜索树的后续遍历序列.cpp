
/**
 * 验证二叉搜索树的后续遍历序列
 * 
 * 给定一个整数数组，判断该数组是否是二叉搜索树的后续遍历序列
 * 
 * 后序遍历，根节点在最后
 * 
 * 二叉搜索树，左子树所有节点小于根节点，右子树所有节点大于根节点
 * 
 * 如果是后序遍历的数组，那么最后一个元素是根节点，数组前面的元素可以分成两部分，
 * 第一部分是左子树，第二部分是右子树
 * 左子树的元素都小于根节点，右子树的元素都大于根节点
 * 
 * 在递归遍历中，以根节点为标准，划分左右子树
 * 
 * 有三种情况：
 * 1. 左子树为空，或者右子树为空，直接递归遍历子树
 * 2. 左子树的范围不在右子树旁边，中间有其他的异常值出现，返回false
 * 3. 以当前根节点看是标准的二叉搜索树，需要判断左右子树是否符合标准，将数组划分成左右子树，递归遍历左右子树
 */
class Solution {
public:
    bool verifyTreeOrder(vector<int>& postorder) {
        if(postorder.size()==0){
            return false;
        }
        return helper(postorder, 0, postorder.size()-1);
    }
    bool helper(vector<int>& postorder, int start, int end){
        if(start==end){
            return true;
        }
        int leftChild=-1;
        int rightChild=end;
        for(int i=start;i<end;++i){
            if(postorder[end]>postorder[i]){
                leftChild=i;
            }
            else{
                rightChild=rightChild==end?i:rightChild;
            }
        }
        if(leftChild==-1||rightChild==end){
            return helper(postorder, start, end-1);
        }
        if(leftChild!=rightChild-1){
            return false;
        }
        return helper(postorder, start, leftChild) && helper(postorder, rightChild, end-1);
    }
};