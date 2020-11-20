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
int ThreadPool::maxThreads;
int ThreadPool::maxWorkingThreads = 5;
int ThreadPool::workingThreads = 0;
HANDLE hThreadArray[255];
//HANDLE *hThreadArray;
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
        while (tasks.empty() && !exitFlag)
        {
            Sleep(30);
        }
        if (!tasks.empty())
        {
       
            mutex.lock();
            if (!tasks.empty())
            {
                task = tasks.front();
                tasks.pop_front();
            }
            else
            {
                mutex.unlock();
                continue;
            }
            mutex.unlock();

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
    }
    return 0;
}

ThreadPool::ThreadPool(const int n)
{
    maxThreads = n;
    maxWorkingThreads = 5;
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
void ThreadPool::AddTask(std::function<void()> task)
{
    if (workingThreads < maxWorkingThreads)
    {
        tasks.push_back(task);
        workingThreads++;
        WriteLogAndConsole("Add new task");
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

            tasks.push_back(task);
            workingThreads++;
            WriteLogAndConsole("Add new task");
        }
    }
}

void ThreadPool::StopThreads()
{
    exitFlag = true;
    WaitForMultipleObjects(maxWorkingThreads, hThreadArray, TRUE, INFINITE);

    for (int i = 0; i < maxWorkingThreads; i++)
    {
        CloseHandle(hThreadArray[i]);
    }
    // delete[] hThreadArray;
}