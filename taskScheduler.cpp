#include "taskScheduler.h"

int taskScheduler::getSortIndex(taskScheduler::task temp)
{
    // find the correct index to keep tasks sorted, using binary search
    int r, l, mid;
    l = 0;
    r = tasks.size() - 1;
    while (l <= r)
    {
        mid = (l + r) / 2;
        if (tasks[mid].date.day_of_week < temp.date.day_of_week)
            l = mid + 1;
        else if (tasks[mid].date.day_of_week > temp.date.day_of_week)
            r = mid - 1;
        else //same day_of_week -> check hour
        {
            if (tasks[mid].date.hour < temp.date.hour)
                l = mid + 1;
            else if (tasks[mid].date.hour > temp.date.hour)
                r = mid - 1;
            else //same day_of_week and hour -> check minute
            {
                if (tasks[mid].date.minute < temp.date.minute)
                    l = mid + 1;
                else if (tasks[mid].date.minute > temp.date.minute)
                    r = mid - 1;
                else // task with same time and day of other task
                    return mid;
            }
        }
    }
    return l;
}

int taskScheduler::convertStringToInt(string str)
{
    int ret;
    try {
        ret = stoi(str);
    }
    catch (const std::invalid_argument& ia) {
        std::cerr << "Invalid argument: " << ia.what() << '\n';
        exit(0);
    }
    catch (const std::out_of_range& oor) {
        std::cerr << "Out of Range error: " << oor.what() << '\n';
        exit(0);
    }
    return ret;
}

bool taskScheduler::readTasksAndSaveSorted()
{
    string line, word;

    ifstream file(fileName, ios::in);

    if (file.is_open())
    {
        task temp;
        getline(file, line); // skip the first line        

        while (getline(file, line))
        {
            stringstream str(line);

            getline(str, word, ',');

            temp.date.minute = convertStringToInt(word);
            if (temp.date.minute < 0 || temp.date.minute > 59)
            {
                cout << "ERROR: " << fileName << " file has invalid data in minute field.Fix it and rerun the program.";
                return false;
            }
            getline(str, word, ',');
            temp.date.hour = convertStringToInt(word);
            if (temp.date.hour < 0 || temp.date.hour > 23)
            {
                cout << "ERROR: " << fileName << " file has invalid data in hour field. Fix it and rerun the program.";
                return false;
            }
            getline(str, word, ',');
            temp.date.day_of_week = convertStringToInt(word);
            if (temp.date.day_of_week < 0 || temp.date.day_of_week > 6)
            {
                cout << "ERROR: " << fileName << " file has invalid data in day_of_week field. Fix it and rerun the program.";
                return false;
            }
            if (str.eof())
            {
                cout << "ERROR: " << fileName << " file has invalid data. A line has less than 4 parameters. Fix it and rerun the program.";
                return false;
            }
            getline(str, word, ',');
            temp.command = word;

            if (tasks.empty())
                tasks.push_back(temp);
            else
                tasks.insert(tasks.begin() + getSortIndex(temp), temp);
        }
    }
    else
    {
        cout << "ERROR: Could not open the file\n";
        return false;
    }
    file.close();
    return true;
}

taskScheduler::mydate taskScheduler::getCurrentDate()
{
    mydate ret;
    struct tm newtime;
    time_t now;

    now = time(0);
    localtime_s(&newtime, &now);

    // convert Sun = 0 to Sun = 6
    if (newtime.tm_wday == 0)// Sunday
        ret.day_of_week = 6;
    // convert Mon-Sut from 1-6 to 0-5
    else
        ret.day_of_week = newtime.tm_wday - 1;

    ret.hour = newtime.tm_hour;
    ret.minute = newtime.tm_min;
    return ret;
}

int taskScheduler::getfirstTaskIndex()
{
    task temp;
    temp.date = getCurrentDate();
    return getSortIndex(temp);
}

void taskScheduler::runTask(string command)
{
    time_t tt;
    tm local_tm;
    char currentTime[30];
    tt = chrono::system_clock::to_time_t(chrono::system_clock::now());
    ctime_s(currentTime, sizeof(currentTime), &tt);
    cout << command << " is running!     Current time is " << currentTime;
}

void taskScheduler::taskRunner()
{
    task next;
    static int ind = -1;
    int daysUntilNextTask;
    mydate currentDate;
    time_t tt;
    tm local_tm;

    ind = getfirstTaskIndex();
    while (true)
    {
        // sleep this thread until next task time arrived
        currentDate = getCurrentDate();
        daysUntilNextTask = tasks[ind].date.day_of_week - currentDate.day_of_week;
        if (daysUntilNextTask < 0)
            daysUntilNextTask = 7 - currentDate.day_of_week + tasks[ind].date.day_of_week;

        chrono::system_clock::time_point timePt = chrono::system_clock::now() + chrono::hours(daysUntilNextTask * 24);
        // convert to tm to set the correct time
        tt = chrono::system_clock::to_time_t(timePt);
        localtime_s(&local_tm, &tt);
        local_tm.tm_sec = 0;
        local_tm.tm_min = tasks[ind].date.minute;
        local_tm.tm_hour = tasks[ind].date.hour;
        // convert back to time_point
        timePt = chrono::system_clock::from_time_t(mktime(&local_tm));
        this_thread::sleep_until(timePt);

        // run next task
        thread(&taskScheduler::runTask, this, tasks[ind].command).detach();

        // find the next task
        if (ind == tasks.size() - 1)// after last task start from the first task again
            ind = 0;
        else
            ind++;
    }
}

void taskScheduler::listenOnFileChanges(LPTSTR lpDir, thread& scheduler)
{
    DWORD dwWaitStatus;
    HANDLE dwChangeHandles;
    TCHAR lpDrive[4];
    TCHAR lpFile[_MAX_FNAME];
    TCHAR lpExt[_MAX_EXT];
    time_t lastTimeModified = 0;
    struct stat result;
    //string filename = "./tasks.csv";

    _tsplitpath_s(lpDir, lpDrive, 4, NULL, 0, lpFile, _MAX_FNAME, lpExt, _MAX_EXT);

    // Watch the directory for file creation and deletion. 
    dwChangeHandles = FindFirstChangeNotification(
        lpDir,                         // directory to watch 
        FALSE,                         // do not watch subtree 
        FILE_NOTIFY_CHANGE_LAST_WRITE); // watch file name changes 

    if (dwChangeHandles == INVALID_HANDLE_VALUE)
    {
        printf("\n ERROR: FindFirstChangeNotification function failed.\n");
        ExitProcess(GetLastError());
    }

    // Make a final validation check on our handles.
    if ((dwChangeHandles == NULL))
    {
        printf("\n ERROR: Unexpected NULL from FindFirstChangeNotification.\n");
        ExitProcess(GetLastError());
    }

    // get last time the file modified in order to use it for re-read the file when needed
    if (stat(fileName.c_str(), &result) == 0)
        lastTimeModified = result.st_mtime;

    // Change notification is set. Now wait on both notification handles 
    while (TRUE)
    {
        // Wait for notification.
        dwWaitStatus = WaitForSingleObject(dwChangeHandles, INFINITE);

        switch (dwWaitStatus)
        {
        case WAIT_OBJECT_0:
            // A file was changed
            // it called twice for every change because that the size of the file changed 
            //and also the last time modified changed so we will check if the last time 
            //modified changed and if yes we will re-read the file

            if (stat(fileName.c_str(), &result) == 0)
            {
                if (lastTimeModified < result.st_mtime)
                {
                    cout << fileName << " file modified on " << result.st_mtime << " re-read the file" << endl;
                    lastTimeModified = result.st_mtime;
                    scheduler.~thread();// stop scheduler thread
                    tasks.clear();
                    // short sleep to let the file update properly before read it again
                    // withou this sleep the getline in readTasksAndSaveSorted() fail with failbit
                    // I beleive there is a better solution for this without the sleep
                    this_thread::sleep_for(std::chrono::milliseconds(100));
                    if (readTasksAndSaveSorted())
                    {
                        scheduler = thread{ &taskScheduler::taskRunner, this };
                        scheduler.detach();
                    }
                }
            }

            if (FindNextChangeNotification(dwChangeHandles) == FALSE)
            {
                printf("\n ERROR: FindNextChangeNotification function failed.\n");
                ExitProcess(GetLastError());
            }
            break;

        default:
            printf("\n ERROR: Unhandled dwWaitStatus.\n");
            ExitProcess(GetLastError());
            break;
        }
    }
}

void taskScheduler::start()
{
    thread scheduler;
    if (readTasksAndSaveSorted())
    {
        scheduler = thread{ &taskScheduler::taskRunner, this };
        scheduler.detach();
    }

    LPTSTR lpDir = const_cast<LPTSTR>(TEXT("."));// current directory    
    listenOnFileChanges(lpDir, scheduler);
}