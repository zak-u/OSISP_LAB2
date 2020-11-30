#include <iostream>
#include <thread>
#include <vector>
#include <list>
#include <functional>
#include <condition_variable>
#include <dos.h>
#include <windows.h>
#include "ThreadPool.h"


void testProc()
{
    Sleep(50);
    std::cout << "done\n";

}

int main()
{
    int n;
    std::cin >> n;

    ThreadPool threadPool(n);
    for (int i = 0; i < 10; i++)
    {
        threadPool.AddTask(testProc);
    }
    Sleep(10);
    threadPool.StopThreads();
   
    return 0;
}