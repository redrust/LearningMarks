/*
最小路径和
给定一个包含非负整数的 m x n 网格，请找出一条从左上角到右下角的路径，使得路径上的数字总和为最小。
说明：每次只能向下或者向右移动一步。

使用动态规划
构建一个二维数组mat，mat[i][j]表示从左上角到grid[i][j]的最小路径和
状态转移公式：mat[i][j]=min(mat[i-1][j], mat[i][j-1])+grid[i][j]
边界条件：
mat[0][0]=grid[0][0]
*/

class Solution {
    public:
        int minPathSum(vector<vector<int>>& grid) {
            if(grid.size()==0){
                return 0;
            }
            int rowSize=grid.size();
            int colSize=grid[0].size();
            vector<vector<int>> mat(rowSize, vector<int>(colSize));
            mat[0][0]=grid[0][0];
            for(int i=1;i<rowSize;++i){
                mat[i][0]=mat[i-1][0]+grid[i][0];
            }
            for(int i=1;i<colSize;++i){
                mat[0][i]=mat[0][i-1]+grid[0][i];
            }
            for(int i=1;i<rowSize;++i){
                for(int j=1;j<colSize;++j){
                    mat[i][j]=min(mat[i-1][j], mat[i][j-1])+grid[i][j];
                }
            }
            return mat[mat.size()-1][mat[0].size()-1];
        }
    };


/*
压缩矩阵的动态规划
因为每次只需要用到上一行或者上一列的信息，所以可以压缩矩阵，只需要用一个一维数组即可
*/
class Solution {
    public:
        int minPathSum(vector<vector<int>>& grid) {
            if(grid.size()==0){
                return 0;
            }
            int rowSize=grid.size();
            int colSize=grid[0].size();
            int more=max(rowSize, colSize);
            int less=min(rowSize, colSize);
            bool rowMore=more==rowSize;
            vector<int> arr(less);
            arr[0]=grid[0][0];
            for(int i=1;i<less;++i){
                arr[i]=arr[i-1]+(rowMore?grid[0][i]:grid[i][0]);
            }
            for(int i=1;i<more;++i){
                arr[0]=arr[0]+(rowMore?grid[i][0]:grid[0][i]);
                for(int j=1;j<less;++j){
                    arr[j]=min(arr[j-1], arr[j])+(rowMore?grid[i][j]:grid[j][i]);
                }
            }
            return arr[arr.size()-1];
        }
    };
