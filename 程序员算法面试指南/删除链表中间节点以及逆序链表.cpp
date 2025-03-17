/*
    删除链表中间节点
    使用快慢指针，快指针每次走两步，慢指针每次走一步，当快指针到达链表末尾时，慢指针正好到达链表中间节点
    因为快指针每次走两步，所以当链表长度为奇数时，慢指针正好到达中间节点，当链表长度为偶数时，慢指针会到达中间两个节点中的前一个
    所以需要使用一个pre指针记录慢指针的前一个节点，当快指针到达链表末尾时，删除慢指针所在的节点
*/
class Solution {
public:
    ListNode* deleteMiddle(ListNode* head) {
        if(head->next==nullptr){
            return head->next;
        }
        ListNode* slow=head;
        ListNode* fast=head;
        ListNode* pre=nullptr;
        while(fast!=nullptr&&fast->next!=nullptr){
            fast=fast->next->next;
            pre=slow;
            slow=slow->next;
        }
        pre->next=pre->next->next;
        return head;
    }
};

/*
    逆序链表
    使用一个pre指针记录当前节点的前一个节点，一个next指针记录当前节点的下一个节点
    然后让当前节点的next指针指向pre指针，然后pre指针和head指针都向后移动一个节点
    直到head指针为nullptr，此时pre指针正好指向逆序链表的头节点
*/
class Solution {
public:
    ListNode* reverseList(ListNode* head) {
        if(head==nullptr){
            return head;
        }
        ListNode* pre;
        ListNode* next;
        while(head!=nullptr){
            next=head->next;
            head->next=pre;
            pre=head;
            head=next;
        }
        return pre;
    }
};