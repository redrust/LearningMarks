/*
给定一个排序链表，删除所有重复的元素，使得每个元素只出现一次。

暴力解法：
1. 使用一个哈希表来记录每个元素出现的次数
2. 遍历链表，将每个元素出现的次数记录到哈希表中
3. 遍历哈希表，将出现次数为1的元素连接起来
4. 返回新的链表


也可以使用双指针来实现，一个指针遍历链表，一个指针记录当前不重复的元素
*/
class Solution {
public:
    ListNode* deleteDuplicates(ListNode* head) {
        if(head==nullptr){
            return head;
        }
        unordered_map<int, vector<ListNode*>> m;
        vector<int> vec;
        ListNode* cur=head;
        while(cur!=nullptr){
            int val=cur->val;
            vec.push_back(val);
            m[val].push_back(cur);
            cur=cur->next;
        }
        ListNode* newHead=new ListNode();
        cur=newHead;
        for(auto& v:vec){
            auto& item=m[v];
            if(item.size()==1){
                cur->next=item[0];
                cur=cur->next;
                cur->next=nullptr;
            }
        }
        return newHead->next;
    }
};