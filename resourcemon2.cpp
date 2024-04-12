#include <iostream>
#include <fstream>
#include <Windows.h>
#include <thread>
#include <chrono>
#include <ctime>
#include <sstream>

using namespace std;

struct UsageStats {
    double currentUsage;
    double maxUsage;
    double totalUsage;
    int count;
};

bool getCPUUsage(double& cpu_usage) {
    FILETIME idleTime, kernelTime, userTime;

    if (!GetSystemTimes(&idleTime, &kernelTime, &userTime)) {
        cerr << "Error: Unable to get system times." << endl;
        return false;
    }

    static ULONGLONG prevIdleTime = 0, prevKernelTime = 0, prevUserTime = 0;
    ULONGLONG idleTimeNow = (reinterpret_cast<ULONGLONG*>(&idleTime))[0];
    ULONGLONG kernelTimeNow = (reinterpret_cast<ULONGLONG*>(&kernelTime))[0];
    ULONGLONG userTimeNow = (reinterpret_cast<ULONGLONG*>(&userTime))[0];

    double idle = idleTimeNow - prevIdleTime;
    double kernel = kernelTimeNow - prevKernelTime;
    double user = userTimeNow - prevUserTime;
    double system = kernel + user;

    cpu_usage = ((system - idle) / system) * 100.0;

    prevIdleTime = idleTimeNow;
    prevKernelTime = kernelTimeNow;
    prevUserTime = userTimeNow;

    return true;
}

bool getRAMUsage(double& ram_usage) {
    MEMORYSTATUSEX memoryStatus;
    memoryStatus.dwLength = sizeof(memoryStatus);
    if (!GlobalMemoryStatusEx(&memoryStatus)) {
        cerr << "Error: Unable to get memory status." << endl;
        return false;
    }

    double totalRAM = memoryStatus.ullTotalPhys / (1024.0 * 1024.0); // Convert bytes to MB
    double usedRAM = (memoryStatus.ullTotalPhys - memoryStatus.ullAvailPhys) / (1024.0 * 1024.0); // Convert bytes to MB

    ram_usage = (usedRAM / totalRAM) * 100.0;

    return true;
}

bool getDiskUsage(double& disk_usage) {
    ULARGE_INTEGER freeBytesAvailable, totalNumberOfBytes, totalNumberOfFreeBytes;

    if (!GetDiskFreeSpaceEx(NULL, &freeBytesAvailable, &totalNumberOfBytes, &totalNumberOfFreeBytes)) {
        cerr << "Error: Unable to get disk space." << endl;
        return false;
    }

    double totalDiskSpace = totalNumberOfBytes.QuadPart / (1024.0 * 1024.0 * 1024.0); // Convert bytes to GB
    double freeDiskSpace = totalNumberOfFreeBytes.QuadPart / (1024.0 * 1024.0 * 1024.0); // Convert bytes to GB

    disk_usage = ((totalDiskSpace - freeDiskSpace) / totalDiskSpace) * 100.0;

    return true;
}

void clearConsole() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD screen;
    screen.X = 0;
    screen.Y = 0;
    DWORD written;
    FillConsoleOutputCharacter(hConsole, ' ', 80 * 25, screen, &written);
    SetConsoleCursorPosition(hConsole, screen);
}

bool writeToFile(const string& filename, const string& data) {
    ofstream outfile(filename, ios::trunc);
    if (!outfile.is_open()) {
        cerr << "Error: Could not open file " << filename << endl;
        return false;
    }
    outfile << data << endl;
    outfile.close();
    return true;
}

int main() {
    string filename = "resource_usage.txt";

    UsageStats cpuStats = {0.0, 0.0, 0.0, 0};
    UsageStats ramStats = {0.0, 0.0, 0.0, 0};
    UsageStats diskStats = {0.0, 0.0, 0.0, 0};

    while (true) {
        double cpu_usage;
        if (!getCPUUsage(cpu_usage)) {
            cerr << "Error: Failed to get CPU usage." << endl;
            continue;
        }

        double ram_usage;
        if (!getRAMUsage(ram_usage)) {
            cerr << "Error: Failed to get RAM usage." << endl;
            continue;
        }

        double disk_usage;
        if (!getDiskUsage(disk_usage)) {
            cerr << "Error: Failed to get Disk usage." << endl;
            continue;
        }

        cpuStats.currentUsage = cpu_usage;
        ramStats.currentUsage = ram_usage;
        diskStats.currentUsage = disk_usage;

        if (cpu_usage > cpuStats.maxUsage)
            cpuStats.maxUsage = cpu_usage;
        if (ram_usage > ramStats.maxUsage)
            ramStats.maxUsage = ram_usage;
        if (disk_usage > diskStats.maxUsage)
            diskStats.maxUsage = disk_usage;

        cpuStats.totalUsage += cpu_usage;
        ramStats.totalUsage += ram_usage;
        diskStats.totalUsage += disk_usage;
        cpuStats.count++;
        ramStats.count++;
        diskStats.count++;

        double avgCPU = cpuStats.totalUsage / cpuStats.count;
        double avgRAM = ramStats.totalUsage / ramStats.count;
        double avgDisk = diskStats.totalUsage / diskStats.count;

        clearConsole();

        time_t now = time(0);
        tm* current_time = localtime(&now);
        char time_buffer[80];
        strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", current_time);

        stringstream data_stream;
        data_stream << "LIVE RESMON :-> " << endl;
        data_stream << "Timestamp: " << time_buffer << endl;
        data_stream << "Current CPU Usage: " << cpu_usage << "%" << endl;
        data_stream << "Max CPU Usage: " << cpuStats.maxUsage << "%" << endl;
        data_stream << "Average CPU Usage: " << avgCPU << "%" << endl;
        data_stream << "--------------------------------------" << endl;
        data_stream << "Current RAM Usage: " << ram_usage << "%" << endl;
        data_stream << "Max RAM Usage: " << ramStats.maxUsage << "%" << endl;
        data_stream << "Average RAM Usage: " << avgRAM << "%" << endl;
        data_stream << "--------------------------------------" << endl;
        data_stream << "Current Disk Usage: " << disk_usage << "%" << endl;
        data_stream << "Max Disk Usage: " << diskStats.maxUsage << "%" << endl;
        data_stream << "Average Disk Usage: " << avgDisk << "%" << endl;
        string data = data_stream.str();

        if (!writeToFile(filename, data)) {
            cerr << "Error: Failed to write data to file." << endl;
        }

        this_thread::sleep_for(chrono::milliseconds(1000));
    }

    return 0;
}
