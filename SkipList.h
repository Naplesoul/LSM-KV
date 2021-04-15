#ifndef SKIPLIST_H
#define SKIPLIST_H
#include <cstdint>
#include <vector>
#include <random>

template <typename K, typename V>
struct Node{
    Node<K, V> *right, *down;   //向右向下足矣
    K key;
    V val;
    Node(Node<K, V> *right, Node<K, V> *down, K key, V val): right(right), down(down), key(key), val(val){}
    Node(): right(nullptr), down(nullptr) {}
};

template <typename K, typename V>
class SkipList
{
private:
    Node<K, V> *head;
    uint64_t listSize;
public:
    //初始化头结点
    SkipList()
    {
        head = new Node<K, V>();
        listSize = 0;
    }

    ~SkipList()
    {
        clear();
    }

    void reset()
    {
        clear();
        head = new Node<K, V>();
        listSize = 0;
    }

    void clear()
    {
        Node<K, V> *del, *h;
        del = head;
        h = head;
        while(del) {
            h = h->down;
            while(del) {
                Node<K, V> *next = del->right;
                delete del;
                del = next;
            }
            del = h;
        }
    }

    uint64_t size()
    {
        return listSize;
    }

    V *get(const K &key) const
    {
        bool found = false;
        Node<K, V> *cur = head;
        while(cur){
            while(cur->right && cur->right->key < key)
            {
                cur = cur->right;
            }
            if(cur->right && cur->right->key == key)
            {
                cur = cur->right;
                found = true;
                break;
            }
            cur = cur->down;
        }
        if(found)
            return &(cur->val);
        else
            return nullptr;
    }

    void put(const K &key, const V &val)
    {
        std::vector<Node<K, V>*> pathList;    //从上至下记录搜索路径
        Node<K, V> *p = head;
        while(p){
            while(p->right && p->right->key < key){
                p = p->right;
            }
            if (p->right && p->right->key == key) {
                p = p->right;
                while (p) {
                    p->val = val;
                    p = p->down;
                }

            }
            pathList.push_back(p);
            p = p->down;
        }

        bool insertUp = true;
        Node<K, V>* downNode= nullptr;
        while(insertUp && pathList.size() > 0){   //从下至上搜索路径回溯，50%概率
            Node<K, V> *insert = pathList.back();
            pathList.pop_back();
            insert->right = new Node<K, V>(insert->right, downNode, key, val); //add新结点
            downNode = insert->right;    //把新结点赋值为downNode
            insertUp = (rand()&1);   //50%概率
        }
        if(insertUp){  //插入新的头结点，加层
            Node<K, V> * oldHead = head;
            head = new Node<K, V>();
            head->right = new Node<K, V>(NULL, downNode, key, val);
            head->down = oldHead;
        }
        ++listSize;
    }


    bool remove(const K &key)
    {
        bool found = false;
        Node<K, V> *cur = head;
        Node<K, V> *pre = cur;
        while(cur)
        {
            pre = cur;
            while(cur->right && cur->right->key < key)
            {
                pre = cur;
                cur = cur->right;
            }
            if(cur->right && cur->right->key == key)
            {
                pre = cur;
                cur = cur->right;
                found = true;
                break;
            }
            cur = cur->down;
        }
        if (!found)
            return false;
        Node<K, V> *del = cur;
        while(cur)
        {
            pre->right = cur->right;
            del = cur;
            cur = cur->down;
            pre = pre->down;
            while(pre && pre->right != cur)
            {
                pre = pre->right;
            }
            delete del;
        }
        --listSize;
        return true;
    }
};

#endif // SKIPLIST_H
