/*
给定两个整数 n 和 k，返回 1 ... n 中所有可能的 k 个数的组合。
示例:
输入: n = 4, k = 2
输出:
[
  [2,4],
  [3,4],
  [2,3], 
]

使用递归进行深度优先搜索
需要注意的是，在每次深度搜索结束的时候，要将状态恢复，以及达到深度搜索的结果的条件时要记录下来
*/
class Solution {
    public:
        vector<vector<int>> combine(int n, int k) {
            vector<vector<int>> result;
            vector<int> path;
            helper(n, k, 1, path, result);
            return result;
        }
        void helper(int n, int k, int j, vector<int>& path, vector<vector<int>>& result){
            for(int i=j;i<=n;++i){
                path.push_back(i);
                if(path.size()==k){
                    result.push_back(path);
                }
                helper(n, k, i+1, path, result);
                path.pop_back();
            }
        }
    };