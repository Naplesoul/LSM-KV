#include "kvstore.h"
#include <stdlib.h>
#include <ctime>
#include <string>
#include <fstream>
#include <windows.h>
#include <vector>

enum TestType {
    PUT, GET, DEL
};


void test(uint64_t size, TestType type, std::ofstream &out);

int main(int argc, char *argv[])
{
    std::ofstream out("with_cache.csv");

    test(100, GET, out);
    test(1000, GET, out);
    test(10000, GET, out);
    test(100, PUT, out);
    test(1000, PUT, out);
    test(10000, PUT, out);
    test(100, DEL, out);
    test(1000, DEL, out);
    test(10000, DEL, out);

    out.close();
    return 0;
}

void test(uint64_t size, TestType type, std::ofstream &out)
{
    static int n = 0;
    ++n;

    srand((unsigned)time(0));
    LARGE_INTEGER cpuFreq;
    LARGE_INTEGER startTime;
    LARGE_INTEGER endTime;
    double runTime;
    KVStore store("./data" + std::to_string(n));
    std::vector<uint64_t> ranKeys;

    switch (type) {

    case PUT:
        for (uint64_t i = 0; i < size; ++i) {
            uint64_t ranKey = rand();
            QueryPerformanceFrequency(&cpuFreq);
            QueryPerformanceCounter(&startTime);
            store.put(ranKey, std::string(10000, 'h'));
            QueryPerformanceCounter(&endTime);
            runTime = (((endTime.QuadPart - startTime.QuadPart) * 1000000.0f) / cpuFreq.QuadPart);
            out << runTime << ',';
        }
        out << std::endl;
        break;

    case GET:
        for (uint64_t i = 0; i < size; ++i) {
            uint64_t ranKey = rand();
            ranKeys.push_back(ranKey);
            store.put(ranKey, std::string(10000, 'h'));
        }

        for (uint64_t i = 0; i < size; ++i) {
            // 有概率是store中不存在的key
            uint64_t ranKey = rand() % 5 < 4 ? ranKeys[rand() % size] : rand();
            QueryPerformanceFrequency(&cpuFreq);
            QueryPerformanceCounter(&startTime);
            store.get(ranKey);
            QueryPerformanceCounter(&endTime);
            runTime = (((endTime.QuadPart - startTime.QuadPart) * 1000000.0f) / cpuFreq.QuadPart);
            out << runTime << ',';
        }
        out << std::endl;
        break;

    case DEL:
        for (uint64_t i = 0; i < size; ++i) {
            uint64_t ranKey = rand();
            ranKeys.push_back(ranKey);
            store.put(ranKey, std::string(10000, 'h'));
        }

        for (uint64_t i = 0; i < size; ++i) {
            // 有概率是store中不存在的key
            uint64_t ranKey = rand() % 5 < 4 ? ranKeys[rand() % size] : rand();
            QueryPerformanceFrequency(&cpuFreq);
            QueryPerformanceCounter(&startTime);
            store.del(ranKey);
            QueryPerformanceCounter(&endTime);
            runTime = (((endTime.QuadPart - startTime.QuadPart) * 1000000.0f) / cpuFreq.QuadPart);
            out << runTime << ',';
        }
        out << std::endl;
        break;

    }
}
