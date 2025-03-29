/*
使用一个哈希表，记录每个前缀和出现的次数
遍历数组，计算当前前缀和，如果前缀和减去k在哈希表中存在，则说明存在一个子数组，其和为k

类似题也可以用相似的思路解决，比如和为k的最长子数组长度
哈希表里记录的是每个前缀和出现的最早位置
当前缀和减去k在哈希表中存在，那么将当前位置减去最早位置大于max，则更新max
如果不存在，则将当前前缀和和位置记录到哈希表中
*/


class Solution {
public:
    int subarraySum(vector<int>& nums, int k) {
        unordered_map<int,int> m;
        int sum=0;
        int result=0;
        m[0]=1;
        for(int i=0;i<nums.size();++i){
            sum+=nums[i];
            if(m.find(sum-k)!=m.end()){
                result+=m[sum-k];
            }
            m[sum]+=1;
        }
        return result;
    }
};