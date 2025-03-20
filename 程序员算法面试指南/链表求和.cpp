/*
链表求和
使用栈保存链表的值，然后从栈顶开始相加，如果相加结果大于10，则进位，否则不进位

如果两个链表长度不一致，则需要将较长的链表的剩余部分直接添加到结果链表中

如果最后有进位，则需要将进位添加到结果链表中

最后需要将结果链表反转
*/
class Solution {
public:
    ListNode* addTwoNumbers(ListNode* l1, ListNode* l2) {
        stack<int> s1;
        stack<int> s2;
        while(l1!=nullptr){
            s1.push(l1->val);
            l1=l1->next;
        }
        while(l2!=nullptr){
            s2.push(l2->val);
            l2=l2->next;
        }
        ListNode* head=new ListNode();
        ListNode* cur=head;
        int plus=0;
        while(!s1.empty()&&!s2.empty()){
            int s1TopVal=s1.top();
            s1.pop();
            int s2TopVal=s2.top();
            s2.pop();
            int result=s1TopVal+s2TopVal+plus;
            int val=result%10;
            plus=result>=10?1:0;
            cur->next=new ListNode(val);
            cur=cur->next;
        }
        while(!s1.empty()){
            int s1TopVal=s1.top();
            s1.pop();
            int result=s1TopVal+plus;
            int val=result%10;
            plus=result>=10?1:0;
            cur->next=new ListNode(val);
            cur=cur->next;
        }
        while(!s2.empty()){
            int s2TopVal=s2.top();
            s2.pop();
            int result=s2TopVal+plus;
            int val=result%10;
            plus=result>=10?1:0;
            cur->next=new ListNode(val);
            cur=cur->next;
        }
        if(plus!=0){
            cur->next=new ListNode(1);
            cur=cur->next;
        }
        cur=head->next;
        ListNode* pre=nullptr;
        ListNode* next=nullptr;
        while(cur!=nullptr){
            next=cur->next;
            cur->next=pre;
            pre=cur;
            cur=next;
        }
        return pre;
    }
};