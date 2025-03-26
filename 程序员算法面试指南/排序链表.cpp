/* 
给定一个链表，将其排序。

解法：
1. 使用一个vector来存储每个节点
2. 将vector中的节点按照值排序
3. 将vector中的节点连接起来
4. 返回新的链表
*/
class Solution {
public:
    ListNode* sortList(ListNode* head) {
        if(head==nullptr||head->next==nullptr){
            return head;
        }
        vector<pair<int,ListNode*>> vec;
        ListNode* cur=head;
        while(cur!=nullptr){
            vec.push_back(make_pair(cur->val, cur));
            cur=cur->next;
        }
        sort(vec.begin(), vec.end());
        ListNode* newHead=new ListNode();
        ListNode* pre=newHead;
        for(auto& item:vec){
            pre->next=item.second;
            item.second->next=nullptr;
            pre=pre->next;
        }
        return newHead->next;
    }
};