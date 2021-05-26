#include "kvstore.h"
#include <stdlib.h>
#include <ctime>
#include <string>
#include <fstream>
#include <windows.h>

int main(int argc, char *argv[])
{
    LARGE_INTEGER cpuFreq;
    LARGE_INTEGER startTime;
    LARGE_INTEGER endTime;
    double runTime;
    std::ofstream out("get_part_cache.csv");
    KVStore store("./data");
    for (uint64_t i = 0; i < 10000; ++i)
        store.put(i, std::string(10086, 'h'));
    for (uint64_t i = 0; i < 10000; ++i) {
        QueryPerformanceFrequency(&cpuFreq);
        QueryPerformanceCounter(&startTime);
        store.get(i);
        QueryPerformanceCounter(&endTime);
        runTime = (((endTime.QuadPart - startTime.QuadPart) * 1000000.0f) / cpuFreq.QuadPart);
        out << runTime << std::endl;
    }
    out.close();
    return 0;
}
