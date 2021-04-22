#include "SSTable.h"
#include <fstream>
#include <iostream>
#include <string.h>

SSTable::SSTable(SSTableCache *cache)
{
    path = cache->path;
    std::ifstream file(path, std::ios::binary);
    if(!file) {
        printf("Fail to open file %s", path.c_str());
        exit(-1);
    }
    timeStamp = (cache->header).timeStamp;
    size = (cache->header).size;

    // load from files
    file.seekg((cache->indexes)[0].offset);
    for(auto it = (cache->indexes).begin();;) {
        uint64_t key = (*it).key;
        uint32_t offset = (*it).offset;
        std::string val;
        if(++it == (cache->indexes).end()) {
            file >> val;
            entries.push_back(Entry(key, val));
            break;
        } else {
            uint32_t length = (*it).offset - offset;
            char *buf = new char[length + 1];
            buf[length] = '\0';
            file.read(buf, length);
            val = buf;
            delete[] buf;
            entries.push_back(Entry(key, val));
        }
    }

    delete cache;
}

void SSTable::merge(std::vector<SSTable> &tables)
{
    uint32_t size = tables.size();
    if(size == 1)
        return;
    uint32_t group = size/2;
    std::vector<SSTable> next;
    for(uint32_t i = 0; i < group; ++i) {
        next.push_back(merge2(tables[2*i], tables[2*i + 1]));
    }
    if(size % 2 == 1)
        next.push_back(tables[size - 1]);
    merge(next);
    tables = next;
}

SSTable SSTable::merge2(SSTable &a, SSTable &b)
{
    uint64_t aTime = a.timeStamp, bTime = b.timeStamp;
    bool aPriority = aTime > bTime;
    SSTable result;
    result.timeStamp = aPriority ? aTime : bTime;
    while(!a.entries.empty() && !b.entries.empty()) {
        uint64_t aKey = a.entries.front().key, bKey = b.entries.front().key;
        if(aKey > bKey) {
            result.entries.push_back(b.entries.front());
            b.entries.pop_front();
        } else if(aKey < bKey) {
            result.entries.push_back(a.entries.front());
            a.entries.pop_front();
        } else {
            if(aPriority) {
                result.entries.push_back(a.entries.front());
                a.entries.pop_front();
                b.entries.pop_front();
            } else {
                result.entries.push_back(b.entries.front());
                a.entries.pop_front();
                b.entries.pop_front();
            }
        }
    }
    while(!a.entries.empty()){
        result.entries.push_back(a.entries.front());
        a.entries.pop_front();
    }
    while(!b.entries.empty()){
        result.entries.push_back(b.entries.front());
        b.entries.pop_front();
    }
    return result;
}

std::vector<SSTableCache*> SSTable::save(const std::string &dir, uint64_t &currentTime)
{
    std::vector<SSTableCache*> caches;
    SSTable newTable;
    while(!entries.empty()) {
        if(newTable.size + 12 + entries.front().val.size() >= MAX_TABLE_SIZE) {
            caches.push_back(newTable.saveSingle(dir, currentTime++));
            newTable = SSTable();
        }
        newTable.add(entries.front());
        entries.pop_front();
    }
    if(newTable.length > 0) {
        caches.push_back(newTable.saveSingle(dir, currentTime++));
    }
    return caches;
}

void SSTable::add(const Entry &entry)
{
    size += 12 + entry.val.size();
    length++;
    entries.push_back(entry);
}

SSTableCache *SSTable::saveSingle(const std::string &dir, const uint64_t &currentTime)
{
    SSTableCache *cache = new SSTableCache;

    char *buffer = new char[size];
    BloomFilter *filter = cache->bloomFilter;

    *(uint64_t*)buffer = currentTime;
    (cache->header).timeStamp = currentTime;

    *(uint64_t*)(buffer + 8) = length;
    (cache->header).size = length;

    *(uint64_t*)(buffer + 16) = entries.front().key;
    (cache->header).minKey = entries.front().key;

    char *index = buffer + 10272;
    uint32_t offset = 10272 + length * 12;

    for(auto it = entries.begin(); it != entries.end(); ++it) {
        filter->add((*it).key);
        *(uint64_t*)index = (*it).key;
        index += 8;
        *(uint32_t*)index = offset;
        index += 4;

        (cache->indexes).push_back(Index((*it).key, offset));
        uint32_t strLen = ((*it).val).size();
        uint32_t newOffset = offset + strLen;
        if(newOffset > size) {
            printf("Buffer Overflow!!!\n");
            exit(-1);
        }
        memcpy(buffer + offset, ((*it).val).c_str(), strLen);
        offset = newOffset;
    }

    *(uint64_t*)(buffer + 24) = entries.back().key;
    (cache->header).maxKey = entries.back().key;
    filter->save2Buffer(buffer + 32);

    std::string filename = dir + "/" + std::to_string(currentTime) + ".sst";
    cache->path = filename;
    std::ofstream outFile(filename, std::ios::binary | std::ios::out);
    outFile.write(buffer, size);

    delete[] buffer;
    outFile.close();
    return cache;
}

SSTableCache::SSTableCache(const std::string &dir)
{
    path = dir;
    std::ifstream file(dir, std::ios::binary);
    if(!file) {
        printf("Fail to open file %s", dir.c_str());
        exit(-1);
    }
    // load header
    file.read((char*)&header.timeStamp, 8);
    file.read((char*)&header.size, 8);
    uint64_t length = header.size;
    file.read((char*)&header.minKey, 8);
    file.read((char*)&header.maxKey, 8);

    // load bloom filter
    char *filterBuf = new char[FILTER_SIZE/8];
    file.read(filterBuf, FILTER_SIZE/8);
    bloomFilter = new BloomFilter(filterBuf);


    char *indexBuf = new char[length * 12];
    file.read(indexBuf, length * 12);
    for(uint32_t i = 0; i < length; ++i) {
        indexes.push_back(Index(*(uint64_t*)(indexBuf + 12*i), *(uint32_t*)(indexBuf + 12*i + 8)));
    }

    delete[] filterBuf;
    delete[] indexBuf;
    file.close();

}

int SSTableCache::get(const uint64_t &key)
{
    if(!bloomFilter->contains(key))
        return -1;
    return find(key, 0, indexes.size() - 1);
}

int SSTableCache::find(const uint64_t &key, int start, int end)
{
    if(start > end)
        return -1;
    if(start == end) {
        if(indexes[start].key == key)
            return start;
        else
            return -1;
    }
    int mid = (start + end) / 2;
    if(indexes[mid].key == key)
        return mid;
    else if(indexes[mid].key < key)
        return find(key, mid + 1, end);
    else
        return find(key, start, mid - 1);
}

bool cacheTimeCompare(SSTableCache *a, SSTableCache *b)
{
    return (a->header).timeStamp > (b->header).timeStamp;
}

bool tableTimeCompare(SSTable &a, SSTable &b)
{
    return a.timeStamp > b.timeStamp;
}
