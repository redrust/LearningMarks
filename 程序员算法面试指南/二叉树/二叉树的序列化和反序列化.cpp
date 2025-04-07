/*
使用先序遍历，将二叉树序列化为字符串，使用"!"作为分隔符
比如：
    1
   / \
  3   2
 / \
 4   5
序列化结果为：1!3!4!#!#!5!#!#!2!#!#!

反序列化：
将字符串按"!"分割成一个队列，然后依次将队列中的元素出队，构建二叉树
比如：
    1!3!4!#!#!5!#!#!2!#!#!
    
    
因为先序遍历的顺序是：根节点->左子树->右子树
所以反序列化的时候，先创建根节点，然后创建左子树，再创建右子树
队列中的元素是根节点、左子树、右子树
逐一出队构建左子树和右子树即可
*/
class Codec {
public:

    // Encodes a tree to a single string.
    string serialize(TreeNode* root) {
        string result;
        serializeHelper(root, result);
        return result;
    }

    void serializeHelper(TreeNode* root, string& result){
        if(root==nullptr){
            result+="#!";
            return;
        }
        result+=to_string(root->val)+"!";
        serializeHelper(root->left, result);
        serializeHelper(root->right, result);
    }

    // Decodes your encoded data to tree.
    TreeNode* deserialize(string data) {
        deque<string> strQueue;
        split(data, strQueue);
        TreeNode* root=deserializeHelper(strQueue);
        return root;
    }

    void split(string& data, deque<string>& result){
        size_t start=0;
        size_t end=data.find('!');
        while(end!=string::npos){
            result.push_back(data.substr(start,end-start));
            start=end+1;
            end=data.find('!',start);
        }
    }

    TreeNode* deserializeHelper(deque<string>& strQueue){
        string val=strQueue.front();
        strQueue.pop_front();
        if(val=="#"){
            return nullptr;
        }
        TreeNode* node=new TreeNode(stoi(val));
        node->left=deserializeHelper(strQueue);
        node->right=deserializeHelper(strQueue);
        return node;
    }
};

// Your Codec object will be instantiated and called as such:
// Codec ser, deser;
// TreeNode* ans = deser.deserialize(ser.serialize(root));