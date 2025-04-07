/*
给定一个链表，返回链表开始入环的第一个节点。 如果链表无环，则返回 null。

使用快慢指针，快指针每次走两步，慢指针每次走一步。
如果链表有环，那么快指针和慢指针一定会相遇。
如果链表无环，那么快指针会走到链表的末尾。

当快慢指针相遇时，让快指针回到链表的头节点，然后快慢指针每次都走一步，当快慢指针再次相遇时，相遇的节点就是链表开始入环的第一个节点。

*/
class Solution {
public:
    ListNode *detectCycle(ListNode *head) {
        if(head==nullptr||head->next==nullptr||head->next->next==nullptr){
            return nullptr;
        }
        ListNode* slow=head;
        ListNode* fast=head;
        while(fast!=nullptr){
            if(fast->next==nullptr){
                return nullptr;
            }
            slow=slow->next;
            fast=fast->next->next;
            if(slow==fast){
                fast=head;
                while(slow!=fast){
                    slow=slow->next;
                    fast=fast->next;
                }
                return slow;
            }
        }
        return nullptr;
    }
};