/*
返回两个链表的第一个公共节点

先遍历两个链表，获取到两个链表的长度，然后让长的链表先走长度差值，然后两个链表一起走，直到找到公共节点
*/
class Solution {
public:
    ListNode *getIntersectionNode(ListNode *headA, ListNode *headB) {
        int lenA=0;
        int lenB=0;
        ListNode* curA=headA;
        ListNode* curB=headB;
        ListNode* tailA=nullptr;
        ListNode* tailB=nullptr;
        while(curA){
            ++lenA;
            if(curA->next==nullptr){
                tailA=curA;
            }
            curA=curA->next;
        }
        while(curB){
            ++lenB;
            if(curB->next==nullptr){
                tailB=curB;
            }
            curB=curB->next;
        }
        if(tailA!=tailB){
            return nullptr;
        }
        curA=headA;
        curB=headB;
        int jumpTo=0;
        if(lenA>lenB){
            jumpTo=lenA-lenB;
            while(jumpTo>0){
                curA=curA->next;
                --jumpTo;
            }
        }
        else{
            jumpTo=lenB-lenA;
            while(jumpTo>0){
                curB=curB->next;
                --jumpTo;
            }
        }
        while(curA!=curB){
            curA=curA->next;
            curB=curB->next;
        }
        return curA;
    }
};