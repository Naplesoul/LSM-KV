#include "SkipList.h"
#include "BloomFilter.h"
#include <fstream>
#include <stdlib.h>
#include <string.h>

SkipList::SkipList(std::vector<Entry> entrys)
{
    for(auto it = entrys.begin(); it != entrys.end(); ++it)
        put((*it).key, (*it).val);
}

SkipList::~SkipList()
{
    Node *del, *h;
    del = head;
    h = head;
    while(del) {
        h = h->down;
        while(del) {
            Node *next = del->right;
            delete del;
            del = next;
        }
        del = h;
    }
}

uint64_t SkipList::length()
{
    return listLength;
}

uint64_t SkipList::size()
{
    return listSize;
}

std::string *SkipList::get(const uint64_t &key) const
{
    bool found = false;
    Node *cur = head, *tmp;
    while(cur){
        while((tmp = cur->right) && tmp->key < key)
        {
            cur = tmp;
        }
        if((tmp = cur->right) && tmp->key == key)
        {
            cur = tmp;
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

// 返回表中原来是否有该key
bool SkipList::put(const uint64_t &key, const std::string &val)
{
    std::vector<Node*> pathList;    //从上至下记录搜索路径
    Node *p = head;
    Node *tmp;
    while(p){
        while((tmp = p->right) && tmp->key < key) {
            p = tmp;
        }
        // 如果list中已有该key，则替换
        if ((tmp = p->right) && tmp->key == key) {
            p = tmp;
            if (p->val == val)
                return false;
            listSize = listSize - (p->val).size() + val.size();
            while (p) {
                p->val = val;
                p = p->down;
            }
            return true;
        }
        pathList.push_back(p);
        p = p->down;
    }

    bool insertUp = true;
    Node* downNode= nullptr;
    while(insertUp && pathList.size() > 0){   //从下至上搜索路径回溯，50%概率
        Node *insert = pathList.back();
        pathList.pop_back();
        insert->right = new Node(insert->right, downNode, key, val); //add新结点
        downNode = insert->right;    //把新结点赋值为downNode
        insertUp = (rand()&1);   //50%概率
    }
    if(insertUp){  //插入新的头结点，加层
        Node * oldHead = head;
        head = new Node();
        head->right = new Node(NULL, downNode, key, val);
        head->down = oldHead;
    }
    ++listLength;
    // 加一个key、一个offset、一个string的大小
    listSize += 12 + val.size();
    return false;
}

// 返回是否成功删除（原表中是否有该key）
bool SkipList::remove(const uint64_t &key)
{
    return put(key, "~DELETED~");
}

Node *SkipList::getListHead()
{
    Node *cur = head;
    while(cur->down) {
        cur = cur->down;
    }
    return cur->right;
}

SSTableCache *SkipList::save2SSTable(const std::string &dir, const uint64_t &currentTime)
{
    SSTableCache *cache = new SSTableCache;

    Node *cur = getListHead();
    char *buffer = new char[listSize];
    BloomFilter *filter = cache->bloomFilter;

    *(uint64_t*)buffer = currentTime;
    (cache->header).timeStamp = currentTime;

    *(uint64_t*)(buffer + 8) = listLength;
    (cache->header).size = listLength;

    *(uint64_t*)(buffer + 16) = cur->key;
    (cache->header).minKey = cur->key;

    char *index = buffer + 10272;
    uint32_t offset = 10272 + listLength * 12;
    while(true) {
        filter->add(cur->key);
        *(uint64_t*)index = cur->key;
        index += 8;
        *(uint32_t*)index = offset;
        index += 4;

        (cache->indexes).push_back(Index(cur->key, offset));
        uint32_t strLen = (cur->val).size();
        uint32_t newOffset = offset + strLen;
        if(newOffset > listSize) {
            printf("Buffer Overflow!!!\n");
            exit(-1);
        }
        memcpy(buffer + offset, (cur->val).c_str(), strLen);
        offset = newOffset;
        if(cur->right)
            cur = cur->right;
        else
            break;
    }
    *(uint64_t*)(buffer + 24) = cur->key;
    (cache->header).maxKey = cur->key;
    filter->save2Buffer(buffer + 32);

    std::string filename = dir + "/" + std::to_string(currentTime) + ".sst";
    cache->path = filename;
    std::ofstream outFile(filename, std::ios::binary | std::ios::out);
    outFile.write(buffer, listSize);

    delete[] buffer;
    outFile.close();
    return cache;
}
