# Resource Monitoring Application

This project is a C++ resource monitoring application that tracks CPU, RAM, and disk usage in real-time on Windows operating systems. It provides users with detailed statistics including current usage, maximum usage, and average usage over time.

## Features

- **Real-time Monitoring**: Continuously monitors CPU, RAM, and disk usage in real-time.
- **Statistics Tracking**: Tracks current usage, maximum usage, and average usage for CPU, RAM, and disk.
- **Flexible Output**: Outputs statistics to a text file for easy viewing and analysis.
- **Informative Timestamps**: Includes timestamps with each data entry for reference.
- **Running Time Display**: Displays the running time of the application in hours:minutes:seconds format.
- **Multi-threaded**: Utilizes multi-threading to monitor resources concurrently.

## Demo Video

[Watch Demo Video](https://github.com/replayan/cppexp/assets/86905096/652ac45b-5bb9-43ac-a5c6-5b68a696f960)

## Usage

1. Clone the repository:

   ```bash
   git clone https://github.com/replayan/cppexp.git
   ```

2. Navigate to the project directory:

   ```bash
   cd cppexp
   ```

3. Compile the source code using a C++ compiler:

   ```bash
   g++ main.cpp -o resmonitoring -std=c++11 -lpthread
   ```

4. Run the compiled executable:

   ```bash
   ./resmonitoring
   ```

5. View the output in the `resource_usage.txt` file generated in the project directory.

## Requirements

- C++ compiler (e.g., g++)
- Windows operating system (for real-time monitoring)

## Contributing

Contributions are welcome! Please feel free to open issues or submit pull requests to contribute to this project.
