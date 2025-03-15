'''
使用双指针，一个指针先走k步，然后两个指针一起走，当先走的指针到达链表末尾时，后走的指针正好到达倒数第k个节点，这是常规解法

这里提出一个特殊的解法，先遍历一次链表，每经过一个节点就把n的值-1
    如果n>0说明n的值大于链表长度，直接返回头节点
    如果n==0说明n的值等于链表长度，删除头节点
    如果n<0说明n的值小于链表长度，从链表头开始遍历，每经过一个节点就把n的值+1，直到n==0，此时删除当前节点
        这里有一点需要注意，因为n==0的位置恰好是需要删除的节点，而需要获取到删除节点的前一个节点，所以n需要+=1
'''
class Solution {
public:
    ListNode* removeNthFromEnd(ListNode* head, int n) {
        if(head==nullptr){
            return head;
        }
        ListNode* t=head;
        while(t!=nullptr){
            --n;
            t=t->next;
        }
        if(n>0){
            return head;
        }
        else if(n==0){
            return head->next;
        }
        else{
            n+=1;
            t=head;
            while(n<0&&t!=nullptr){
                t=t->next;
                ++n;
            }
            if(t->next){
                t->next=t->next->next;
            }
            return head;
        }
    }
};