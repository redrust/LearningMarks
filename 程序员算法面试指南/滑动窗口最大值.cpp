'''
核心是使用双端队列，维护一个单调递减的队列，队列中存储的是数组的下标
有两个点需要注意：
1.当队列头部下标等于i-k时，说明队列头部下标已经不在窗口内，需要弹出
2.当i>=k-1时，说明窗口已经形成，需要将队列头部的元素加入结果集
3.while循环保证当前窗口内最大值在队列头部
'''

class Solution {
public:
    vector<int> maxSlidingWindow(vector<int>& nums, int k) {
        deque<int> queueMax;
        vector<int> result;
        for (int i = 0; i < nums.size(); i++) {
            while (!queueMax.empty() && nums[queueMax.back()] <= nums[i]) {
                queueMax.pop_back();
            }
            queueMax.push_back(i);
            if (queueMax.front() == i - k) {
                queueMax.pop_front();
            }
            if (i >= k - 1) {
                result.push_back(nums[queueMax.front()]);
            }
        }
        return result;
    }
};