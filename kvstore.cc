#include "kvstore.h"
#include "utils.h"
#include <string>
#include <algorithm>

#define MAX_TABLE_SIZE 2097152

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
                    tableCount.push_back(tableNum);

                    for(int j = 0; j < tableNum; ++j) {
                        SSTableCache* curCache = new SSTableCache(levelDir + "/" + tableNames[j]);
                        uint64_t curTime = (curCache->header).timeStamp;
                        cache[i].push_back(curCache);
                        if(curTime > currentTime)
                            currentTime = curTime;
                    }
                    // make sure the timeStamp of cache is decending
                    std::sort(cache[i].begin(), cache[i].end(), timeCmp);
                } else
                    break;
            }
        }
    } else {
        utils::mkdir(dataDir.c_str());
        utils::mkdir((dataDir + "/level-0").c_str());
    }
    currentTime++;
    memTable = new SkipList();
}

KVStore::~KVStore()
{
    delete memTable;
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
    // make sure the timeStamp of cache is decending
    std::sort(cache[0].begin(), cache[0].end(), timeCmp);
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
    if(val)
        return *val;
    // cannot find in memTable, try to find in SSTables
    std::string result;
    int levelNum = cache.size();
    for(int i = 0; i < levelNum; ++i) {
        // find keys in SSTableCaches, from the one with biggest timestamp
        for(auto it = cache[i].begin(); it != cache[i].end(); ++it) {
            int pos = (*it)->get(key);
            if(pos == -1)
                continue;
            if(pos == ((*it)->indexes).size() - 1) {

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
    return memTable->remove(key);
}

/**
 * This resets the kvstore. All key-value pairs should be removed,
 * including memtable and all sstables files.
 */
void KVStore::reset()
{
    delete memTable;
    memTable = new SkipList();
}

void KVStore::compact()
{

}
