#include "kvstore.h"
#include "utils.h"
#include <string>
#include <algorithm>
#include <fstream>
#include <iostream>


KVStore::KVStore(const std::string &dir): KVStoreAPI(dir)
{
    if(dir[dir.length()] == '/')
        dataDir = dir.substr(0, dir.length() - 1);
    else
        dataDir = dir;
    currentTime = 0;
    // load cache from existed SSTables
    if (utils::dirExists(dataDir)) {
        std::vector<std::string> levelNames;
        int levelNum = utils::scanDir(dataDir, levelNames);
        if (levelNum > 0) {
            for(int i = 0; i < levelNum; ++i) {
                std::string levelName = "level-" + std::to_string(i);
                // check if the level directory exists
                if(std::count(levelNames.begin(), levelNames.end(), levelName) == 1) {
                    cache.push_back(std::vector<SSTableCache*>());

                    std::string levelDir = dataDir + "/" + levelName;
                    std::vector<std::string> tableNames;
                    int tableNum = utils::scanDir(levelDir, tableNames);

                    for(int j = 0; j < tableNum; ++j) {
                        SSTableCache* curCache = new SSTableCache(levelDir + "/" + tableNames[j]);
                        uint64_t curTime = (curCache->header).timeStamp;
                        cache[i].push_back(curCache);
                        if(curTime > currentTime)
                            currentTime = curTime;
                    }
                    // make sure the timeStamp of cache is decending
                    std::sort(cache[i].begin(), cache[i].end(), cacheTimeCompare);
                } else
                    break;
            }
        } else {
            utils::mkdir((dataDir + "/level-0").c_str());
            cache.push_back(std::vector<SSTableCache*>());
        }
    } else {
        utils::mkdir(dataDir.c_str());
        utils::mkdir((dataDir + "/level-0").c_str());
        cache.push_back(std::vector<SSTableCache*>());
    }
    currentTime++;
    memTable = new SkipList();
}

KVStore::~KVStore()
{
    if(memTable->length() > 0)
        memTable->save2SSTable(dataDir + "/level-0", currentTime++);
    delete memTable;
    compact();
    for(auto it1 = cache.begin(); it1 != cache.end(); ++it1) {
        for(auto it2 = (*it1).begin(); it2 != (*it1).end(); ++it2)
            delete (*it2);
    }
}

/**
 * Insert/Update the key-value pair.
 * No return values for simplicity.
 */
void KVStore::put(uint64_t key, const std::string &s)
{
    if(memTable->size() + s.size() + 12 < MAX_TABLE_SIZE) {
        memTable->put(key, s);
        return;
    }
    cache[0].push_back(memTable->save2SSTable(dataDir + "/level-0", currentTime++));
    delete memTable;
    memTable = new SkipList;
    // make sure the timeStamp of cache is decending
    std::sort(cache[0].begin(), cache[0].end(), cacheTimeCompare);
    compact();
    memTable->put(key, s);
}
/**
 * Returns the (string) value of the given key.
 * An empty string indicates not found.
 */
std::string KVStore::get(uint64_t key)
{
    std::string *val = memTable->get(key);
    if(val) {
        if(*val == "~DELETED~")
            return "";
        return *val;
    }
    // cannot find in memTable, try to find in SSTables
    int levelNum = cache.size();
    for(int i = 0; i < levelNum; ++i) {
        // find keys in SSTableCaches, from the one with biggest timestamp
        for(auto it = cache[i].begin(); it != cache[i].end(); ++it) {
            // check if the key is in the range of the sstablecache
            if(key <= ((*it)->header).maxKey && key >= ((*it)->header).minKey) {
                int pos = (*it)->get(key);
                if(pos < 0)
                    continue;
                std::ifstream file((*it)->path, std::ios::binary);
                if(!file) {
                    printf("Lost file: %s", ((*it)->path).c_str());
                    exit(-1);
                }

                std::string value;
                uint32_t length, offset = ((*it)->indexes)[pos].offset;
                file.seekg(offset);

                // check if it is the last entry
                if((unsigned long)pos == ((*it)->indexes).size() - 1) {
                    file >> value;
                } else {
                    uint32_t nextOffset = ((*it)->indexes)[pos + 1].offset;
                    length = nextOffset - offset;
                    char *result = new char[length + 1];
                    result[length] = '\0';
                    file.read(result, length);
                    value = result;
                    delete[] result;
                }
                file.close();
                if(value == "~DELETED~")
                    return "";
                else
                    return value;
            }

        }
    }
    return "";
}
/**
 * Delete the given key-value pair if it exists.
 * Returns false iff the key is not found.
 */
bool KVStore::del(uint64_t key)
{
    std::string val = get(key);
    if(val == "")
        return false;
    put(key, "~DELETED~");
    return true;
}

/**
 * This resets the kvstore. All key-value pairs should be removed,
 * including memtable and all sstables files.
 */
void KVStore::reset()
{
    delete memTable;
    memTable = new SkipList();
    for(auto it1 = cache.begin(); it1 != cache.end(); ++it1) {
        for(auto it2 = (*it1).begin(); it2 != (*it1).end(); ++it2)
            delete (*it2);
    }
    cache.clear();
    utils::rmdir(dataDir.c_str());
    utils::mkdir(dataDir.c_str());
    utils::mkdir((dataDir + "/level-0").c_str());
    cache.push_back(std::vector<SSTableCache*>());
}

void KVStore::compact()
{
    uint64_t levelMax = 1;
    uint32_t levelNum = cache.size();
    for(uint32_t i = 0; i < levelNum; ++i) {
        levelMax *= 2;
        if(cache[i].size() > levelMax)
            compactLevel(i);
        else
            break;
    }
}

void KVStore::compactLevel(uint32_t level)
{
    std::vector<Range> levelRange;
    std::vector<SSTable> tables2Compact;

    // compact all SSTables in level-0
    if(level == 0) {
        for(auto it = cache[level].begin(); it != cache[level].end(); ++it) {
            levelRange.push_back(Range(((*it)->header).minKey, ((*it)->header).maxKey));
            tables2Compact.push_back(SSTable(*it));
        }
        cache[level].clear();
    } else {

        uint64_t mid = 1 << level;
        auto it = cache[level].begin();
        for(uint64_t i = 0; i < mid; ++i)
            ++it;
        uint64_t newestTime = ((*it)->header).timeStamp;
        while(it != cache[level].begin()) {
            if(((*it)->header).timeStamp > newestTime)
                break;
            --it;
        }
        if(((*it)->header).timeStamp > newestTime)
            ++it;
        while(it != cache[level].end()) {
            levelRange.push_back(Range(((*it)->header).minKey, ((*it)->header).maxKey));
            tables2Compact.push_back(SSTable(*it));
            it = cache[level].erase(it);
        }
    }

    // check if the next level exists
    ++level;
    if(level < cache.size()) {
        for(auto it = cache[level].begin(); it != cache[level].end();) {
            if(haveIntersection(*it, levelRange)) {
                tables2Compact.push_back(SSTable(*it));
                it = cache[level].erase(it);
            } else {
                ++it;
            }
        }
    } else {
        utils::mkdir((dataDir + "/level-" + std::to_string(level)).c_str());
        cache.push_back(std::vector<SSTableCache*>());
    }
    for(auto it = tables2Compact.begin(); it != tables2Compact.end(); ++it)
        utils::rmfile((*it).path.c_str());
    sort(tables2Compact.begin(), tables2Compact.end(), tableTimeCompare);
    SSTable::merge(tables2Compact);
    std::vector<SSTableCache*> newCaches = tables2Compact[0].save(dataDir + "/level-" + std::to_string(level));
    for(auto it = newCaches.begin(); it != newCaches.end(); ++it) {
        cache[level].push_back(*it);
    }
    std::sort(cache[level].begin(), cache[level].end(), cacheTimeCompare);
}
