#include <iostream>
#include <windows.h>
#include <psapi.h>

void printMemoryInfo(HANDLE hProcess) {
    PROCESS_MEMORY_COUNTERS_EX pmc;
    if (GetProcessMemoryInfo(hProcess, (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc))) {
        double peakWorkingSetMB = pmc.PeakWorkingSetSize / (1024.0 * 1024.0);
        std::cout << "Peak Memory Usage: " << peakWorkingSetMB << " MB" << std::endl;
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <command>" << std::endl;
        return 1;
    }

    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    int numCores = sysInfo.dwNumberOfProcessors;

    FILETIME startTime, dummy;
    GetSystemTimeAsFileTime(&startTime);

    STARTUPINFOW si = { sizeof(STARTUPINFOW) };
    PROCESS_INFORMATION pi;

    size_t size = strlen(argv[1]) + 1;
    wchar_t* wideString = new wchar_t[size];
    size_t outSize;
    mbstowcs_s(&outSize, wideString, size, argv[1], size - 1);

    if (!CreateProcessW(nullptr, wideString, nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi)) {
        std::cerr << "Failed to create process. Error code: " << GetLastError() << std::endl;
        delete[] wideString;
        return 1;
    }

    delete[] wideString;

    WaitForSingleObject(pi.hProcess, INFINITE);

    FILETIME endTime;
    GetSystemTimeAsFileTime(&endTime);

    ULARGE_INTEGER start, end;
    start.HighPart = startTime.dwHighDateTime;
    start.LowPart = startTime.dwLowDateTime;
    end.HighPart = endTime.dwHighDateTime;
    end.LowPart = endTime.dwLowDateTime;

    double elapsedTime = (end.QuadPart - start.QuadPart) / 10000000.0;
    std::cout << "Elapsed Time: " << elapsedTime << " seconds" << std::endl;

    FILETIME creationTime, exitTime, kernelTime, userTime;
    if (GetProcessTimes(pi.hProcess, &creationTime, &exitTime, &kernelTime, &userTime)) {
        ULARGE_INTEGER kernel, user;
        kernel.HighPart = kernelTime.dwHighDateTime;
        kernel.LowPart = kernelTime.dwLowDateTime;
        user.HighPart = userTime.dwHighDateTime;
        user.LowPart = userTime.dwLowDateTime;

        double kernelTimeSec = kernel.QuadPart / 10000000.0;
        double userTimeSec = user.QuadPart / 10000000.0;

        std::cout << "Kernel Time: " << kernelTimeSec << " seconds" << std::endl;
        std::cout << "User Time: " << userTimeSec << " seconds" << std::endl;

        double cpuUsage = 100.0 * (kernelTimeSec + userTimeSec) / (elapsedTime * numCores);
        std::cout << "CPU Usage: " << cpuUsage << " %" << std::endl;
        
        printMemoryInfo(pi.hProcess);
    }
    else {
        std::cerr << "Failed to get process times." << std::endl;
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return 0;
}