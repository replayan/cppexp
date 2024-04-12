#include <iostream>
#include <fstream>
#include <Windows.h>
#include <thread>
#include <chrono>
#include <ctime>
#include <sstream>
#include <mutex>

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

void updateResourceStats(UsageStats& stats, double currentUsage) {
    stats.currentUsage = currentUsage;
    if (currentUsage > stats.maxUsage)
        stats.maxUsage = currentUsage;
    stats.totalUsage += currentUsage;
    stats.count++;
}

void writeStatsToFile(const string& filename, const stringstream& dataStream) {
    static mutex mtx; // Declare a static mutex

    // Lock the mutex to ensure exclusive access to the file
    lock_guard<mutex> lock(mtx);

    ofstream outfile(filename);
    if (!outfile.is_open()) {
        cerr << "Error: Could not open file " << filename << endl;
        return;
    }

    outfile << dataStream.str(); // Write the contents of the stringstream to the file

    outfile.close();
}

void monitorCPU(UsageStats& cpuStats, stringstream& dataStream) {
    while (true) {
        double cpu_usage;
        if (getCPUUsage(cpu_usage)) {
            updateResourceStats(cpuStats, cpu_usage);
        }
        this_thread::sleep_for(chrono::milliseconds(1000));
    }
}

void monitorRAM(UsageStats& ramStats, stringstream& dataStream) {
    while (true) {
        double ram_usage;
        if (getRAMUsage(ram_usage)) {
            updateResourceStats(ramStats, ram_usage);
        }
        this_thread::sleep_for(chrono::milliseconds(1000));
    }
}

void monitorDisk(UsageStats& diskStats, stringstream& dataStream) {
    while (true) {
        double disk_usage;
        if (getDiskUsage(disk_usage)) {
            updateResourceStats(diskStats, disk_usage);
        }
        this_thread::sleep_for(chrono::milliseconds(1000));
    }
}

string getCurrentTimestamp() {
    char buffer[80];
    time_t rawtime;
    struct tm timeinfo;

    time(&rawtime);
    localtime_s(&timeinfo, &rawtime);

    strftime(buffer, sizeof(buffer), "%I:%M:%S %p", &timeinfo);
    string time_str(buffer);

    strftime(buffer, sizeof(buffer), "%A, %d %B %Y (%Z)", &timeinfo);
    string date_location_str(buffer);

    return time_str + "\n" + date_location_str + "\n";
}

int main() {
    string filename = "resource_usage.txt";
    stringstream dataStream;

    UsageStats cpuStats = {0.0, 0.0, 0.0, 0};
    UsageStats ramStats = {0.0, 0.0, 0.0, 0};
    UsageStats diskStats = {0.0, 0.0, 0.0, 0};

    auto startTime = chrono::steady_clock::now();

    thread cpuThread(monitorCPU, ref(cpuStats), ref(dataStream));
    thread ramThread(monitorRAM, ref(ramStats), ref(dataStream));
    thread diskThread(monitorDisk, ref(diskStats), ref(dataStream));

    while (true) {
        // Clear the stringstream
        dataStream.str("");

        auto currentTime = chrono::steady_clock::now();
        auto elapsedTime = chrono::duration_cast<chrono::seconds>(currentTime - startTime).count();

        dataStream << "LIVE RESMON :->" << endl;
        dataStream << "Timestamp: " << getCurrentTimestamp() << endl;
        dataStream << "Running Time: " << elapsedTime << " seconds" << endl << "\n";
        dataStream << "Current CPU Usage: " << cpuStats.currentUsage << "%" << endl;
        dataStream << "Max CPU Usage: " << cpuStats.maxUsage << "%" << endl;
        dataStream << "Average CPU Usage: " << (cpuStats.totalUsage / cpuStats.count) << "%" << endl;
        dataStream << "--------------------------------------" << endl;
        dataStream << "Current RAM Usage: " << ramStats.currentUsage << "%" << endl;
        dataStream << "Max RAM Usage: " << ramStats.maxUsage << "%" << endl;
        dataStream << "Average RAM Usage: " << (ramStats.totalUsage / ramStats.count) << "%" << endl;
        dataStream << "--------------------------------------" << endl;
        dataStream << "Current Disk Usage: " << diskStats.currentUsage << "%" << endl;
        dataStream << "Max Disk Usage: " << diskStats.maxUsage << "%" << endl;
        dataStream << "Average Disk Usage: " << (diskStats.totalUsage / diskStats.count) << "%" << endl;

        // Write the contents of the stringstream to the file
        writeStatsToFile(filename, dataStream);

        // Sleep for a short duration
        this_thread::sleep_for(chrono::milliseconds(1000));
    }

    cpuThread.join();
    ramThread.join();
    diskThread.join();

    return 0;
}
