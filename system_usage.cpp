#include <iostream>
#include <Windows.h>
#include <thread>
#include <vector>

using namespace std;

struct UsageStats {
    double currentUsage;
    double maxUsage;
    double totalUsage;
    int count;
};

void clearScreen() {
    system("cls");
}

double getCPUUsage() {
    FILETIME idleTime, kernelTime, userTime;

    if (!GetSystemTimes(&idleTime, &kernelTime, &userTime)) {
        cerr << "Error: Unable to get system times." << endl;
        return -1.0;
    }

    static ULONGLONG prevIdleTime = 0, prevKernelTime = 0, prevUserTime = 0;
    ULONGLONG idleTimeNow = (reinterpret_cast<ULONGLONG*>(&idleTime))[0];
    ULONGLONG kernelTimeNow = (reinterpret_cast<ULONGLONG*>(&kernelTime))[0];
    ULONGLONG userTimeNow = (reinterpret_cast<ULONGLONG*>(&userTime))[0];

    double idle = idleTimeNow - prevIdleTime;
    double kernel = kernelTimeNow - prevKernelTime;
    double user = userTimeNow - prevUserTime;
    double system = kernel + user;

    double cpu_usage = ((system - idle) / system) * 100.0;

    prevIdleTime = idleTimeNow;
    prevKernelTime = kernelTimeNow;
    prevUserTime = userTimeNow;

    return cpu_usage;
}

double getRAMUsage() {
    MEMORYSTATUSEX memoryStatus;
    memoryStatus.dwLength = sizeof(memoryStatus);
    GlobalMemoryStatusEx(&memoryStatus);

    double totalRAM = memoryStatus.ullTotalPhys / (1024.0 * 1024.0); // Convert bytes to MB
    double usedRAM = (memoryStatus.ullTotalPhys - memoryStatus.ullAvailPhys) / (1024.0 * 1024.0); // Convert bytes to MB

    double ram_usage = (usedRAM / totalRAM) * 100.0;

    return ram_usage;
}

double getDiskUsage() {
    ULARGE_INTEGER freeBytesAvailable, totalNumberOfBytes, totalNumberOfFreeBytes;

    if (!GetDiskFreeSpaceEx(NULL, &freeBytesAvailable, &totalNumberOfBytes, &totalNumberOfFreeBytes)) {
        cerr << "Error: Unable to get disk space." << endl;
        return -1.0;
    }

    double totalDiskSpace = totalNumberOfBytes.QuadPart / (1024.0 * 1024.0 * 1024.0); // Convert bytes to GB
    double freeDiskSpace = totalNumberOfFreeBytes.QuadPart / (1024.0 * 1024.0 * 1024.0); // Convert bytes to GB

    double disk_usage = ((totalDiskSpace - freeDiskSpace) / totalDiskSpace) * 100.0;

    return disk_usage;
}

string getCurrentTimestamp() {
    char buffer[80];
    time_t rawtime;
    struct tm *timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    
    strftime(buffer, sizeof(buffer), "%I:%M:%S %p", timeinfo);
    string time_str(buffer);

    strftime(buffer, sizeof(buffer), "%A, %d %B %Y (%Z)", timeinfo);
    string date_location_str(buffer);

    return time_str + "\n" + date_location_str;
}

int main() {
    // Initialize usage statistics for CPU, RAM, and Disk
    UsageStats cpuStats = {0.0, 0.0, 0.0, 0};
    UsageStats ramStats = {0.0, 0.0, 0.0, 0};
    UsageStats diskStats = {0.0, 0.0, 0.0, 0};

    // Vector to store historical usages for calculating average
    vector<double> cpuHistory;
    vector<double> ramHistory;
    vector<double> diskHistory;

    while (true) {
        clearScreen();

        double cpu_usage = getCPUUsage();
        double ram_usage = getRAMUsage();
        double disk_usage = getDiskUsage();

        // Update current usage
        cpuStats.currentUsage = cpu_usage;
        ramStats.currentUsage = ram_usage;
        diskStats.currentUsage = disk_usage;

        // Update maximum usage
        if (cpu_usage > cpuStats.maxUsage)
            cpuStats.maxUsage = cpu_usage;
        if (ram_usage > ramStats.maxUsage)
            ramStats.maxUsage = ram_usage;
        if (disk_usage > diskStats.maxUsage)
            diskStats.maxUsage = disk_usage;

        // Update total usage and count for average calculation
        cpuStats.totalUsage += cpu_usage;
        ramStats.totalUsage += ram_usage;
        diskStats.totalUsage += disk_usage;
        cpuStats.count++;
        ramStats.count++;
        diskStats.count++;

        // Update history for average calculation
        cpuHistory.push_back(cpu_usage);
        ramHistory.push_back(ram_usage);
        diskHistory.push_back(disk_usage);
        
        cout << "LIVE RESMON :->" << endl;
        cout << "Timestamp: " << getCurrentTimestamp() << endl;
        cout << endl;
        cout << "Current CPU Usage: " << cpu_usage << "%" << endl;
        cout << "Current RAM Usage: " << ram_usage << "%" << endl;
        cout << "Current Disk Usage: " << disk_usage << "%" << endl;
        cout << "--------------------------------------" << endl;

        if (cpuStats.count != 0) {
            double avgCPU = cpuStats.totalUsage / cpuStats.count;
            cout << "Average CPU Usage: " << avgCPU << "%" << endl;
        }
        if (ramStats.count != 0) {
            double avgRAM = ramStats.totalUsage / ramStats.count;
            cout << "Average RAM Usage: " << avgRAM << "%" << endl;
        }
        if (diskStats.count != 0) {
            double avgDisk = diskStats.totalUsage / diskStats.count;
            cout << "Average Disk Usage: " << avgDisk << "%" << endl;
        }

        cout << "--------------------------------------" << endl;
        cout << "Max CPU Usage: " << cpuStats.maxUsage << "%" << endl;
        cout << "Max RAM Usage: " << ramStats.maxUsage << "%" << endl;
        cout << "Max Disk Usage: " << diskStats.maxUsage << "%" << endl;

        this_thread::sleep_for(chrono::seconds(1));
    }
    return 0;
}
