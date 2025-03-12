'''
本质上是求矩形i往左最近的比i小的矩形，往右离i最近且比i小的矩形

算法的核心是：
1. 使用单调栈获取每个元素的下一个更大元素
2. 使用单调栈获取每个元素的下一个更小元素
3. 遍历每个元素，计算以该元素为高度的矩形面积
4. 返回最大矩形面积

当遍历到i时，如果栈顶元素比i大，则弹出栈顶元素，计算以栈顶元素为高度的矩形面积
设当前弹出栈顶元素为j,因为单调栈保持单调性，所以j往左最多能扩展的位置是k,往右最多能扩展到i-1
所以j位置的矩形面积为(i-k-1)*heights[j]

当遍历完heights后，如果栈中还残留有元素，则这些元素没有下一个更小元素，所以可以扩展到heights.size()
其中还需要注意的是，当栈为空时，k=-1

'''

class Solution {
public:
    int largestRectangleArea(vector<int>& heights) {
        
        if(heights.size()==0){
            return 0;
        }
        int result=0;
        stack<int> s;
        for(int i=0;i<heights.size();i++){
            while(!s.empty()&&heights[i]<=heights[s.top()]){
                int j=s.top();
                s.pop();
                int k=s.empty()?-1:s.top();
                int t=(i-k-1)*heights[j];
                result=max(result, t);
            }
            s.push(i);
        }
        while(!s.empty()){
            int j=s.top();
            s.pop();
            int k=s.empty()?-1:s.top();
            int t=(heights.size()-k-1)*heights[j];
            result=max(result, t);
        }
        return result;
    }
};