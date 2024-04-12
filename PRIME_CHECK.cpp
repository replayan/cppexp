#include <iostream>
#include <vector>
#include <future>
#include <chrono>
#include <cmath>
#include <algorithm>
#include <mutex>

using namespace std;
using namespace chrono;

mutex prime_count_mutex;  // Mutex for thread-safe access to prime_count

bool isPrime(unsigned long long n) {
    if (n <= 1) return false;
    if (n <= 3) return true;
    if (n % 2 == 0 || n % 3 == 0) return false;
    for (unsigned long long i = 5; i * i <= n; i += 6) {
        if (n % i == 0 || n % (i + 2) == 0) return false;
    }
    return true;
}

int segmentedSieveCount(unsigned long long start, unsigned long long end) {
    int count = 0;
    for (unsigned long long i = start; i < end; ++i) {
        if (isPrime(i)) {
            ++count;
        }
    }
    return count;
}

string formatTime(long long duration) {
    long long hours = duration / 3600;
    long long minutes = (duration % 3600) / 60;
    long long seconds = duration % 60;
    string formattedTime = "";
    if (hours > 0) {
        formattedTime += to_string(hours) + " hours, ";
    }
    if (minutes > 0) {
        formattedTime += to_string(minutes) + " minutes, ";
    }
    formattedTime += to_string(seconds) + " seconds";
    return formattedTime;
}

int main() {
    const unsigned long long limit = 1000000000; // upto 1 billion
    const int num_threads = thread::hardware_concurrency();
    vector<future<int>> futures;
    int prime_count = 0;  // Shared variable for prime count

    auto start_time = high_resolution_clock::now();

    for (int i = 0; i < num_threads; ++i) {
        unsigned long long start = (limit / num_threads) * i;
        unsigned long long end = (limit / num_threads) * (i + 1);
        futures.push_back(async(launch::async, [&prime_count, start, end] {
            int local_count = segmentedSieveCount(start, end);
            lock_guard<mutex> lock(prime_count_mutex);  // Lock the mutex
            prime_count += local_count;  // Update the shared prime_count
            return local_count;
        }));
    }

    for (auto& future : futures) {
        future.get();  // Wait for all threads to finish
    }

    auto end_time = high_resolution_clock::now();
    auto duration = duration_cast<seconds>(end_time - start_time).count();

    cout << "Number of primes up to " << limit << ": " << prime_count << endl;
    cout << "Time taken: " << formatTime(duration) << endl;

    return 0;
}
/*
#include <iostream>
#include <chrono>
#include <cmath>
using namespace std;
using namespace chrono;

bool isPrime(unsigned long long n) {
    if (n <= 1) return false;
    if (n <= 3) return true;
    if (n % 2 == 0 || n % 3 == 0) return false;
    for (unsigned long long i = 5; i * i <= n; i += 6) {
        if (n % i == 0 || n % (i + 2) == 0) return false;
    }
    return true;
}

int segmentedSieveCount(unsigned long long start, unsigned long long end) {
    int count = 0;
    for (unsigned long long i = start; i < end; ++i) {
        if (isPrime(i)) {
            ++count;
        }
    }
    return count;
}

string formatTime(long long duration) {
    long long hours = duration / 3600;
    long long minutes = (duration % 3600) / 60;
    long long seconds = duration % 60;
    string formattedTime = "";
    if (hours > 0) {
        formattedTime += to_string(hours) + " hours, ";
    }
    if (minutes > 0) {
        formattedTime += to_string(minutes) + " minutes, ";
    }
    formattedTime += to_string(seconds) + " seconds";
    return formattedTime;
}

int main() {
    const unsigned long long limit = 10000000000; // up to 1 billion
    int prime_count = 0;

    auto start_time = high_resolution_clock::now();
    prime_count = segmentedSieveCount(0, limit);
    auto end_time = high_resolution_clock::now();

    auto duration = duration_cast<seconds>(end_time - start_time).count();
    cout << "Number of primes up to " << limit << ": " << prime_count << endl;
    cout << "Time taken (single-threaded): " << formatTime(duration) << endl;

    return 0;
}
*/