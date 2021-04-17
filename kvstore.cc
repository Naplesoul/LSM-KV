#include "kvstore.h"
#include "utils.h"
#include <string>
#include <algorithm>


KVStore::KVStore(const std::string &dir): KVStoreAPI(dir)
{
    currentTime = 0;
    // load cache from existed SSTables
    if (utils::dirExists(dir)) {
        std::vector<std::string> levelDirs;
        int levelNum = utils::scanDir(dir, levelDirs);
        if (levelNum > 0) {
            for(int i = 0; i < levelNum; ++i) {
                std::string levelName = "level-" + std::to_string(i);
                if(std::count(levelDirs.begin(), levelDirs.end(), levelName) == 1) {
                    cache.push_back(std::vector<SSTableCache*>());
                    tableCount.push_back(0);
                    std::string levelDir = dir[dir.size()] == '/' ? dir + levelName : dir + "/" + levelName;
                    std::vector<std::string> tableDirs;
                    int tableNum = utils::scanDir(levelDir, tableDirs);
                    tableCount[i] = tableNum;
                    for(int j = 0; j < tableNum; ++j) {
                        SSTableCache* curCache = new SSTableCache(levelDir + "/" + tableDirs[j]);
                        uint64_t curTime = curCache->timeStamp();
                        cache[i].push_back(curCache);
                        if(curTime > currentTime)
                            currentTime = curTime;
                    }
                } else
                    break;
            }
        }
    } else {
        utils::mkdir(dir.c_str());
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
    else
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
