#pragma once
#include <ostream>
#include <list>
#include <mutex>
#include <functional>
#include <wtypes.h>

class ThreadPool {
public:
    static void WriteLogAndConsole(std::string mess);
    static std::list<std::function<void()>> tasks;
    static std::mutex mutex;
    static bool exitFlag;
    static bool EndOfTaskFlag ;

    static int  maxThreads;
    static int workingThreads;
    static int maxWorkingThreads;
    static HANDLE hAccess;
    static HANDLE hFull;
    HANDLE  hThreadArray[255];
    // HANDLE  *hThreadArray;
    static DWORD WINAPI _stdcall Task(void* args);
    ThreadPool(int n);
    void AddTask(std::function<void()> task);
    void Add(std::function<void()> task);
    void StopThreads();
};

