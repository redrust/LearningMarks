/*
随机链表复制
1. 使用一个哈希表来存储每个节点和它的复制节点
2. 遍历原链表，创建每个节点的复制节点，并存储在哈希表中
3. 再次遍历原链表，设置每个节点的复制节点的next和random指针
4. 返回哈希表中第一个节点的复制节点

进阶解法：
1. 在每个节点后面插入它的复制节点
2. 设置复制节点的random指针
3. 将复制节点从原链表中分离出来
*/
class Solution {
public:
    Node* copyRandomList(Node* head) {
        if(!head){
            return head;
        }
        unordered_map<Node*,Node*> m;
        Node* newNode=new Node(head->val);
        m[head]=newNode;
        Node* cur=head->next;
        Node* node=nullptr;
        while(cur!=nullptr){
            node=new Node(cur->val);
            m[cur]=node;
            cur=cur->next;
        }
        cur=head;
        Node* temp=nullptr;
        while(cur!=nullptr){
            temp=m[cur];
            temp->random=m[cur->random];
            if(cur->next){
                temp->next=m[cur->next];
            }
            cur=cur->next;
        }
        return newNode;
    }
};