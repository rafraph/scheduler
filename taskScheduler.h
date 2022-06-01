#pragma once
using namespace std;

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <chrono>
#include <thread>
#include <windows.h>
#include <tchar.h>
#include <sys/types.h>
#include <sys/stat.h>
#define stat _stat

class taskScheduler
{
    struct mydate {
        int minute;
        int hour;
        int day_of_week;
        mydate()
        {
            minute = 0;
            hour = 0;
            day_of_week = 0;
        }
    };

    struct task {
        mydate date;
        string command;
    };

    string fileName;
    vector<task> tasks;

    int getSortIndex(task temp);
    bool readTasksAndSaveSorted();
    int convertStringToInt(string str);
    mydate getCurrentDate();
    int getfirstTaskIndex();
    void runTask(string command);
    void taskRunner();
    void listenOnFileChanges(LPTSTR lpDir, thread& scheduler);

public:
    taskScheduler(string name)
    {
        fileName = name;
    }
    void start();
};