#ifndef SKIPLIST_H
#define SKIPLIST_H

#include <cstdint>
#include <vector>
#include <random>
#include <string>

#include "DataStructs.h"



class SkipList
{
private:
    Node *head;
    uint64_t listLength;
    // 转换为SSTable后的大小，无数据时为10272B
    uint64_t listSize;
public:
    SkipList(): head(new Node()), listLength(0), listSize(10272) {}
    SkipList(std::vector<Entry> entrys);
    ~SkipList();
    uint64_t length();
    uint64_t size();
    std::string *get(const uint64_t &key) const;

    // 返回表中原来是否有该key
    bool put(const uint64_t &key, const std::string &val);
    // 返回是否成功删除（原表中是否有该key）
    bool remove(const uint64_t &key);
    Node* getListHead();
};

#endif // SKIPLIST_H
