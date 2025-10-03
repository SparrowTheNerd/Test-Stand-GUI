#include "BackEnd.h"
#include <windows.h>
#include <iostream>
#include <string>
#include <vector>

namespace StandBackend
{
    char** listComPorts(int& numPorts) {
        std::vector<std::string> comPorts;
        for (int i = 1; i <= 256; i++) { // COM1 to COM256
            std::string comPort = "COM" + std::to_string(i);
            std::string devicePath = "\\\\.\\" + comPort;
            HANDLE hCom = CreateFileA(devicePath.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
            if (hCom != INVALID_HANDLE_VALUE) {
                comPorts.push_back(comPort);
            }
            else if (GetLastError() == ERROR_ACCESS_DENIED) {
                // Port might be in use by another process
                comPorts.push_back(comPort + " (in use)");
            }
            CloseHandle(hCom);
        }
        numPorts = comPorts.size();

        // Allocate memory for the char** array
        char** charArray = new char* [numPorts];
        for (int i = 0; i < numPorts; ++i) {
            charArray[i] = new char[comPorts[i].size() + 1];
            std::strcpy(charArray[i], comPorts[i].c_str());
        }

        return charArray;
    }

    // Function to free the allocated memory
    void freeCharArray(char** charArray, int numPorts) {
        for (int i = 0; i < numPorts; i++) {
            delete[] charArray[i]; // Free each string
        }
        delete[] charArray; // Free the array of char*
    }

    HANDLE connectToComPort(const char* comPortName) {
        std::string devicePath = "\\\\.\\" + std::string(comPortName);
        HANDLE hCom = CreateFileA(devicePath.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
        if (hCom == INVALID_HANDLE_VALUE) {
            std::cerr << "Failed to open COM port " << std::string(comPortName) << std::endl;
            CloseHandle(hCom);
            return INVALID_HANDLE_VALUE;
        }
        DCB dcb;
        dcb.DCBlength = sizeof(dcb);
        if (!GetCommState(hCom, &dcb)) {
            // Handle the error.
            CloseHandle(hCom);
            return INVALID_HANDLE_VALUE;
        }

        dcb.BaudRate = 115200;  // Set baud rate to 115200
        dcb.ByteSize = 8;         // Data bit length
        dcb.StopBits = ONESTOPBIT; // One stop bit
        dcb.Parity = NOPARITY;    // No parity
        dcb.fDtrControl = DTR_CONTROL_ENABLE;   //data terminal ready signal enable

        if (!SetCommState(hCom, &dcb)) {
            std::cerr << "Error: Unable to set COM port state for " << comPortName << std::endl;
            CloseHandle(hCom);
            return INVALID_HANDLE_VALUE;
        }

        COMMTIMEOUTS timeouts;
        timeouts.ReadIntervalTimeout = MAXDWORD;         // Max time between read chars
        timeouts.ReadTotalTimeoutConstant = 0;    // Timeout per read
        timeouts.ReadTotalTimeoutMultiplier = 0;  // Total timeout for read
        timeouts.WriteTotalTimeoutConstant = 50;   // Timeout per write
        timeouts.WriteTotalTimeoutMultiplier = 10; // Total timeout for write

        if (!SetCommTimeouts(hCom, &timeouts)) {
            std::cerr << "Error: Unable to set COM port timeouts for " << comPortName << std::endl;
            CloseHandle(hCom);
            return INVALID_HANDLE_VALUE;
        }

        std::cout << "Connected to " << std::string(comPortName) << std::endl;
        return hCom;
    }

    void closeComPort(HANDLE hCom) {
        if (hCom != INVALID_HANDLE_VALUE) {
            CloseHandle(hCom);
            std::cout << "COM port closed." << std::endl;
        }
    }
}
