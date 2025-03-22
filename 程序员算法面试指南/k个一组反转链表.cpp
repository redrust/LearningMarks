/*
给定一个链表，每 k 个节点一组进行翻转，请你返回翻转后的链表。
k 是一个正整数，它的值小于或等于链表的长度。
如果节点总数不是 k 的整数倍，那么请将最后剩余的节点保持原有顺序。

第一种方法：
使用栈来实现，每次将k个节点压入栈中，然后弹出栈中的节点，并将其连接起来。

需要注意以下几点：
1. 需要记录下每次反转后的尾节点，方便下次反转时连接。
2. 需要记录第一次反转的头节点，方便返回。
3. 当遍历完链表后，如果栈中还有节点，则将栈中的节点依次弹出，直到栈底，然后连接到尾节点。
*/
class Solution {
public:
    ListNode* reverseKGroup(ListNode* head, int k) {
        if(head==nullptr||k==1){
            return head;
        }
        stack<ListNode*> s;
        ListNode* newHead=nullptr;
        ListNode* top=nullptr;
        ListNode* tail=nullptr;
        bool isFirstReverse=false;
        while(head!=nullptr){
            s.push(head);
            head=head->next;
            if(s.size()==k){
                if(isFirstReverse==false){
                    newHead=s.top();
                    isFirstReverse=true;
                }
                bool subFirst=false;
                while(s.empty()==false){
                    if(subFirst==false){
                        if(tail!=nullptr){
                            tail->next=s.top();
                        }
                        subFirst=true;
                    }
                    top=s.top();
                    s.pop();
                    if(s.empty()==true){
                        top->next=nullptr;
                        tail=top;
                    }
                    else{
                        top->next=s.top();
                    }
                }

            }
        }
        while(s.empty()==false){
            if(s.size()==1){
                top->next=s.top();
            }
            s.pop();
        }
        return newHead;
    }
};



/*
第二种方法：

使用多个指针来实现，每次将k个节点反转，然后连接到一起。

需要注意以下几点：
1. 每次反转后，需要记录下反转后的头节点和尾节点，方便下次反转时连接。
2. 当已有节点不足k个时，直接返回头节点。
3. 一组节点内反转时，需要保持链表的完整性，在获取pre节点时，需要获取到tail节点的下一个节点，否则会破坏链表
4. 每次反转前，需要保存下tail节点的下一个节点，在反转完成之后要将反转链表和原链表连接起来
5. pre指针需要移动到每次反转后的tail节点,head指针需要移动到tail节点的下一个节点,也就是下一组的头节点
*/

class Solution {
public:
    pair<ListNode*, ListNode*> reverse(ListNode* head, ListNode* tail){
        ListNode* next=nullptr;
        ListNode* pre=tail->next;
        ListNode* cur=head;
        while(pre!=tail){
            next=cur->next;
            cur->next=pre;
            pre=cur;
            cur=next;
        }
        return make_pair(tail, head);
    }
    ListNode* reverseKGroup(ListNode* head, int k) {
        if(head==nullptr||k==1){
            return head;
        }
        ListNode* newHead=new ListNode();
        newHead->next=head;
        ListNode* tail=nullptr;
        ListNode* pre=newHead;
        ListNode* newNext=nullptr;
        while(head!=nullptr){
            tail=pre;
            for(int i=0;i<k;++i){
                if(tail->next==nullptr){
                    return newHead->next;
                }
                tail=tail->next;
            }
            newNext=tail->next;
            pair<ListNode*, ListNode*> headTail=reverse(head, tail);
            head=headTail.first;
            tail=headTail.second;
            pre->next=head;
            tail->next=newNext;
            pre=tail;
            head=tail->next;
        }
        return newHead->next;
    }
};