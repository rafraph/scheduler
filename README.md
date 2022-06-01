# scheduler
c++ (version 14) program that demonstarte task scheduler from csv file

## How it Works
This program read tasks from csv file and "execute" them. In this example the csv file is called, "tasks.csv".

The first line of the file is header. Each line from the second line is a task. The first 3 fields of the task is the time (minute 0-59, hour 0-23) and week day (0-6, where 0=Monday, 6=Sunday) when the task should run. If several tasks have the same time, they will be run simultaneously.

The last field is the text of the command of the task. For demonstration propose the task "executed" by printing the command field.

In the beginning, the start function called. This function call to:
* readTasksAndSaveSorted - function that read the csv file and store the tasks in sorted vector
* Than, launch a thread that run taskRunner function. This function run each task when the the specific time of the task arrived.
* In order to listen to changes in the csv file, the listenOnFileChanges function is called. It listen to changes in the current directory, and when the tasks.csv file is changed, it terminate the thread that run taskRunner, read the csv file again, upate the vector of the tasks with the name list and launch the thread that run the taskRunner from the beginning.



## How to use it
1. Clone the repository
2. Click on scheduler.sln file to open the Visual Studio solution (it was build on VS 2022).
3. Click on Run
4. You can change the tasks.csv file in the middle of the run and it will be updated with the new task list on runtime

Note: if the csv file is not written correctly, the program will check it in readTasksAndSaveSorted function and will alert about it. If the csv file is incorrect when running the program, the program will stop and exit. If the csv file is correct when running the program but then changed to be incorrect, the thread of taskRunner will terminate, an alert will be shown on the console but the program will not exit. As soon as the csv file will changed again and will become correct, the thread of taskRunner will run from the beginning again.
