/*
分隔链表
使用两个链表，一个链表保存小于x的节点，一个链表保存大于等于x的节点
最后将两个链表拼接起来

需要注意的是：
1. 需要将每个节点的next指针设置为nullptr，否则会出现环形链表
2. 需要将两个链表拼接起来，最后返回的是第一个链表的头节点
*/
class Solution {
public:
    ListNode* partition(ListNode* head, int x) {
        ListNode* lowerHead=new ListNode();
        ListNode* lowerCur=lowerHead;
        ListNode* upperHead=new ListNode();
        ListNode* upperCur=upperHead;
        ListNode* t=nullptr;
        while(head!=nullptr){
            if(head->val<x){
                lowerCur->next=head;
                lowerCur=lowerCur->next;
            }
            else{
                upperCur->next=head;
                upperCur=upperCur->next;
            }
            t=head;
            head=head->next;
            t->next=nullptr;
        }
        lowerCur->next=upperHead->next;
        return lowerHead->next;
    }
};