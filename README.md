# LoggerClient

A C++ application built with Qt6 and CMake to connect to a logger server and display log messages.

## Table of Contents

- [Description](#description)
- [Features](#features)
- [Technologies Used](#technologies-used)
- [Prerequisites](#prerequisites)
- [Building](#building)
- [Running](#running)
- [Updating](#updating)
- [Contributing](#contributing)

## Description

LoggerClient is a cross-platform desktop application designed to receive, display, and manage log messages from a compatible logger server. It provides a user-friendly graphical interface for real-time log monitoring, filtering, and analysis.

## Features

*   Connects to a logger server over the network.
*   Parses logs from a file or through copy/paste.
*   Can save log messages to a file, after searching or filtering.
*   Displays log messages in a structured and filterable view.
*   Allows management of multiple network addresses and connection profiles.
*   Supports custom logger patterns for parsing and highlighting log entries.
*   Provides powerful keyword highlighting and search capabilities.
*   Includes customizable notifications for critical events.
*   Cross-platform compatibility (Windows, Linux, macOS).

## Technologies Used

*   C++
*   Qt6 (Widgets, Network)
*   CMake

## Prerequisites

*   A C++ compiler (e.g., MSVC, GCC, Clang)
*   CMake (version 3.25 or higher)
*   Qt6 development libraries (including Widgets and Network modules)

## Building

1.  Clone the repository:
    ```bash
    git clone <repository_url>
    cd LoggerClient
    ```
2.  Create a build directory and navigate into it:
    ```bash
    mkdir build
    cd build
    ```
3.  Configure the project with CMake. Ensure Qt6 is found. You might need to specify the Qt6 installation path:
    ```bash
    cmake .. -DCMAKE_PREFIX_PATH="<path/to/your/Qt6/installation>"
    ```
4.  Build the project:
    ```bash
    cmake --build . --config Release
    ```

## Running

After building, the executable will be located in the build directory (e.g., `build/Release/LoggerClient` on Windows, `build/LoggerClient` on Linux/macOS).

```bash
# Example for Windows
.\build\Release\LoggerClient.exe

# Example for Linux/macOS
./build/LoggerClient
```

## Updating

To update LoggerClient to a newer version while preserving your preferences:

1. Navigate to your current LoggerClient installation directory.
2. Identify the `.ini` configuration file. **Do not delete this file**, as it contains all your saved settings and configurations.
3. Safely remove all other files and folders from the installation directory.
4. Extract and copy the contents of the new release into the directory.

## Contributing

Contributions are welcome! Please fork the repository and create a pull request with your changes.
