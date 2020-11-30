#include <thread>
#include <mutex>
#include <list>
#include <functional>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include "ThreadPool.h"



std::list<std::function<void()>> ThreadPool::tasks;
std::mutex ThreadPool::mutex;
bool ThreadPool::exitFlag = false;
bool ThreadPool::EndOfTaskFlag = false;
int ThreadPool::maxThreads;
int ThreadPool::maxWorkingThreads = 5;
int ThreadPool::workingThreads = 0;
HANDLE hThreadArray[255];
//HANDLE *hThreadArray;

HANDLE ThreadPool::hAccess;
HANDLE ThreadPool::hFull;



void ThreadPool::WriteLogAndConsole(std::string message)
{
    std::cout << message << std::endl;

    std::ofstream out("Log.txt", std::ios::app);
    out << message << std::endl;
    out.close();
}

DWORD WINAPI ThreadPool::Task(void* args)
{
    std::function<void()> task;
    while (!exitFlag)
    {
        
            WaitForSingleObject(
                hFull,	// pointer to the semaphore
                50		// waiting interval
            );
        
         WaitForSingleObject(
                hAccess,	// pointer to the semaphore
                INFINITE		//waiting interval
            );
        
        if (!tasks.empty())
        {
            task = tasks.front();
            tasks.pop_front();
        }
       
         ReleaseSemaphore(
                hAccess,	// pointer to the semaphore
                1,		// changes the counter to 1
                NULL);
         try
         {
             task();
         }
         catch (const std::exception& e)
         {
             WriteLogAndConsole(e.what());
         }
         workingThreads--;
    }
    return 0;
}

ThreadPool::ThreadPool(const int n)
{
    maxThreads = n;
    maxWorkingThreads = 5;
    hAccess = CreateSemaphore(
        NULL,	
        1,	// initial state
        1,	// max state
        NULL	// without name
    );
    hFull = CreateSemaphore(
        NULL,	// no atributies
        0,	// initial state
        10000,	// max state
        NULL	// without name
    );
    //*hThreadArray = new HANDLE[n];
    DWORD threadId;
    for (int i = 0; i < maxWorkingThreads; i++)
    {
        hThreadArray[i] = CreateThread(NULL, 0, ThreadPool::Task, 0, 0, &threadId);

        std::stringstream stream;
        stream << "Thread " << threadId << " was create";
        ThreadPool::WriteLogAndConsole(stream.str());
    }

}
void ThreadPool::Add(std::function<void()> task)
{
    tasks.push_back(task);
    ReleaseSemaphore(
        hFull,	// pointer to the semaphore
        1,		// changes the counter to 1
        NULL);
    workingThreads++;
    WriteLogAndConsole("Add new task");
}
void ThreadPool::AddTask(std::function<void()> task)
{
    if (workingThreads < maxWorkingThreads)
    {
        ThreadPool::Add(task);
    }
    else
    {
        WriteLogAndConsole("WARNING: The maximum allowed number of running threads has been exceeded");
        if (maxWorkingThreads + 1 <= maxThreads)
        {
            DWORD threadId;
            hThreadArray[maxWorkingThreads] = CreateThread(NULL, 0, ThreadPool::Task, 0, 0, &threadId);
            maxWorkingThreads++;
            std::stringstream stream;
            stream << "Thread " << threadId << " was create";
            ThreadPool::WriteLogAndConsole(stream.str());

            ThreadPool::Add(task);
        }
    }
}

void ThreadPool::StopThreads()
{
    EndOfTaskFlag = true;
    exitFlag = true;
    WaitForMultipleObjects(maxWorkingThreads, hThreadArray, TRUE, INFINITE);
    CloseHandle(hAccess);
    CloseHandle(hFull);
    for (int i = 0; i < maxWorkingThreads; i++)
    {
        CloseHandle(hThreadArray[i]);
    }
   
}