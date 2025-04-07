
// 暴力解法
class Solution {
public:
    long long subArrayRanges(vector<int>& nums) {
        long long result=0;
        for(int i=0;i<nums.size();i++){
            int iMax=nums[i];
            int iMin=nums[i];
            for(int j=i;j<nums.size();j++){
                iMax=max(iMax, nums[j]);
                iMin=min(iMin, nums[j]);
                result+=iMax-iMin;
            }
        }
        return result;
    }
};


'''
单调栈解法
求解i位置的元素成为了多少个区间的最大值，多少个区间的最小值，前者是i位置元素累计被加的次数，后者是i位置元素累计被减的次数
使用单调栈，可以获取到i位置左边第一个比它小的元素，和右边第一个比它小的元素
针对位置i,左边第一个比它小的元素是k,右边第一个比它小的元素是j,那么i位置的元素成为了k~j区间的最大值，被加了(i-k)*(j-i)次
同理可以推及到i位置左边第一个比它大的元素，和右边第一个比它大的元素，那么i位置的元素成为了k~j区间的最小值，被减了(i-k)*(j-i)次
需要注意的一点是，数组会有重复元素，为了避免重复计算，区间上采用左闭右开
'''

class Solution {
public:
    long long subArrayRanges(vector<int>& nums) {
        int n=nums.size();
        vector<int> lsmall(n,0), rsmall(n,0), llarger(n,0), rlarger(n,0);
        stack<int> s;
        
        s.push(-1);
        for(int i=0;i<n;++i){
            while(s.top()!=-1&&nums[s.top()]>=nums[i]){
                s.pop();
            }
            lsmall[i]=s.top();
            s.push(i);
        }

        s = stack<int>();
        s.push(n);
        for(int i=n-1;i>=0;--i){
            while(s.top()!=n&&nums[s.top()]>nums[i]){
                s.pop();
            }
            rsmall[i]=s.top();
            s.push(i);
        }

        s = stack<int>();
        s.push(-1);
        for(int i=0;i<n;++i){
            while(s.top()!=-1&&nums[s.top()]<=nums[i]){
                s.pop();
            }
            llarger[i]=s.top();
            s.push(i);
        }

        s = stack<int>();
        s.push(n);
        for(int i=n-1;i>=0;--i){
            while(s.top()!=n&&nums[s.top()]<nums[i]){
                s.pop();
            }
            rlarger[i]=s.top();
            s.push(i);
        }

        long long result=0;
        for(int i=0;i<n;++i){
            result+=1L*(i-llarger[i])*(rlarger[i]-i)*nums[i];
            result-=1L*(i-lsmall[i])*(rsmall[i]-i)*nums[i];
        }
        return result;
    }
};