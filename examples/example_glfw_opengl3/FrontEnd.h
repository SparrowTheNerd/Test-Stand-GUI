#pragma once
#include "imgui.h"
#include <GLFW/glfw3.h>
#include <windows.h>
#include <iostream>


namespace StandGUI
{
//#pragma pack(push,1)
    struct packet {
        float timeStamp, tankPrs, combnPrs, force;
        uint8_t status;
        int16_t rssi;
    };
//#pragma pack(pop)
    struct ScrollingBuffer {
        int MaxSize;
        int Offset;
        ImVector<ImVec2> Data;
        ScrollingBuffer(int max_size = 600) {
            MaxSize = max_size;
            Offset = 0;
            Data.reserve(MaxSize);
        }
        void AddPoint(float x, float y) {
            if (Data.size() < MaxSize)
                Data.push_back(ImVec2(x, y));
            else {
                Data[Offset] = ImVec2(x, y);
                Offset = (Offset + 1) % MaxSize;
            }
        }
        void Erase() {
            if (Data.size() > 0) {
                Data.shrink(0);
                Offset = 0;
            }
        }

    };

    void RenderUI(ImGuiIO,GLFWwindow*);
    void destroyer();
}
