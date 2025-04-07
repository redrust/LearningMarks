/*
给定链表中的一个节点，删除该节点。

解法：
1. 将当前节点的值赋值为下一个节点的值
2. 将当前节点的next指针指向下一个节点的next指针

存在问题：
1. 如果当前节点是最后一个节点，则无法删除
2. 它本质上并不是删除节点，而是将当前节点的值赋值为下一个节点的值，然后删除下一个节点，在工程应用中，节点的值以及相关逻辑可能非常复杂，所以不建议使用这种方法

*/
class Solution {
public:
    void deleteNode(ListNode* node) {
        ListNode* next=node->next;
        node->val=next->val;
        node->next=next->next;
    }
};