/*
回文链表简单解法
使用栈保存链表的值，然后遍历链表，比较栈顶元素和链表当前元素是否相等
*/
class Solution {
public:
    bool isPalindrome(ListNode* head) {
        stack<int> s;
        ListNode* cur=head;
        while(cur!=nullptr){
            s.push(cur->val);
            cur=cur->next;
        }
        cur=head;
        while(s.empty()==false){
            if(s.top()!=cur->val){
                return false;
            }
            s.pop();
            cur=cur->next;
        }
        return true;
    }
};

/*
回文链表进阶解法
使用快慢指针找到链表中点，然后反转后半部分链表，然后比较前半部分和后半部分是否相等
*/
class Solution {
public:
    bool isPalindrome(ListNode* head) {
        if(head==nullptr||head->next==nullptr){
            return true;
        }
        ListNode* slow=head;
        ListNode* fast=head;
        while(fast!=nullptr&&fast->next!=nullptr){
            slow=slow->next;
            fast=fast->next->next;
        }
        ListNode* mid=slow;
        ListNode* pre=nullptr;
        ListNode* cur=mid;
        while(cur!=nullptr){
            ListNode* next=cur->next;
            cur->next=pre;
            pre=cur;
            cur=next;
        }
        while(pre!=nullptr&&head!=nullptr){
            if(pre->val!=head->val){
                return false;
            }   
            pre=pre->next;
            head=head->next;
        }
        return true;
    }
};


/*
反转部分链表

使用两个指针，一个指针preLeft指向left的前一个节点，一个指针lastRight指向right的后一个节点
然后反转left到right之间的链表，最后将preLeft的next指向反转后的链表的头节点，将反转后的链表的尾节点指向lastRight

需要考虑的情况：
如果left==1，也就是意味着需要从头节点开始反转链表, 则preLeft为nullptr
    则反转后的链表的头节点为head
如果preLeft!=nullptr，则反转后的链表的头节点为preLeft->next,也就是需要把preLeft->next指向反转后的链表的头节点，串成完整的链表
*/
class Solution {
public:
    ListNode* reverseBetween(ListNode* head, int left, int right) {
        ListNode* node1=head;
        ListNode* preLeft=nullptr;
        ListNode* lastRight=nullptr;
        int n=0;
        while(node1!=nullptr){
            ++n;
            preLeft=n==left-1?node1:preLeft;
            lastRight=n==right+1?node1:lastRight;
            node1=node1->next;
        }
        if (left > right || right > n || left < 1){
            return head;
        }
        node1=preLeft==nullptr?head:preLeft->next;
        ListNode* next=nullptr;
        ListNode* node2=node1->next;
        node1->next=lastRight;
        while(node2!=lastRight){
            next=node2->next;
            node2->next=node1;
            node1=node2;
            node2=next;
        }
        if(preLeft!=nullptr){
            preLeft->next=node1;
            return head;
        }
        return node1;
    }
};