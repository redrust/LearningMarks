'''
使用单调栈获取数组中每个元素的下一个更大元素
单调栈：栈中元素保持单调性，单调递增或单调递减
算法的核心是：
1. 如果栈顶元素小于当前元素，则将栈顶元素弹出，直到栈顶元素大于当前元素，保持栈的单调性
2. 将当前元素压入栈中
3. 遍历完数组之后，如果栈中还残留有元素，那么这些元素没有下一个更大元素，设置为-1
4. 遍历目标数组，查找生成好的map，获取下一个更大元素
5. 返回结果
'''

class Solution {
public:
    vector<int> nextGreaterElement(vector<int>& nums1, vector<int>& nums2) {
        stack<int> s;
        map<int,int> m;
        for (int i=0;i<nums2.size();i++){
            int cur=nums2[i];
            while(!s.empty()&&s.top()<cur){
                m[s.top()]=cur;
                s.pop();
            }
            s.push(cur);
        }
        while(!s.empty()){
            int t=s.top();
            s.pop();
            m[t]=-1;
        }
        vector<int> result(nums1.size());
        for(int i=0;i<nums1.size();i++){
            auto iter=m.find(nums1[i]);
            if(iter!=m.end()){
                result[i]=iter->second;
            }
            else{
                result[i]=-1;
            }
        }
        return result;
    }
};