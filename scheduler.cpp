// scheduler.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "taskScheduler.h"

int main()
{
    taskScheduler ts("tasks.csv");
    ts.start();

    return 0;
}
