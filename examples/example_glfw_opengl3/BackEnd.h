#pragma once
#include <windows.h>
#include <iostream>
#include <string>
#include <vector>
using namespace std;

namespace StandBackend
{
    /*vector<string>listComPorts();
    HANDLE connectToComPort(const std::string& comPort);
    char** convertVectorToCharArray(const vector<string>& vec);*/
    char** listComPorts(int& numPorts);
    void freeCharArray(char** charArray, int numPorts);
    HANDLE connectToComPort(const char* comPortName);
    void closeComPort(HANDLE hCom);
}
