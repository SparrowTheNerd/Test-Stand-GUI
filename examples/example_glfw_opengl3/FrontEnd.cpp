#include "FrontEnd.h"
#include "imgui.h"
#include "implot.h"
#include <GLFW/glfw3.h>
#include "BackEnd.h"
#include "graphing.h"
#include <format>
#include <chrono>
const float PI = (float)3.14159265358979323846;

using namespace std;

int width, height;
const int sendItHeight = 250; const int sendItWidth = 500;
const int edgeMargin = 30;
float plotHeight;

bool showDemo = false;
bool comCon;    //is a COM port connected?

std::string fireText = "SEND IT";
std::string armText = "ARM";
std::chrono::steady_clock::time_point holdTimer;
bool sent = false;

namespace StandGUI
{
    packet dataPacket;  //serial data packet struct
    HANDLE hCom;    //COM port handle

    static void TextCentered(std::string text, ImVec2 centerPos) {
        auto textWidth = ImGui::CalcTextSize(text.c_str()).x;

        ImGui::SetCursorPos(ImVec2(centerPos.x - textWidth / 2.f, centerPos.y));
        ImGui::Text(text.c_str());
    }

    float reMap(float value, float from1, float to1, float from2, float to2) {
        return (value - from1) / (to1 - from1) * (to2 - from2) + from2;
    }

    ImU32 statusColor(bool status) {
        if (status) {
            return IM_COL32(0,255,0,255);
        }
        else { return IM_COL32(255, 0, 0, 255); }
    }

    void RenderUI(ImGuiIO io, GLFWwindow* window) {
        glfwGetWindowSize(window, &width, &height);
        ImGui::DockSpaceOverViewport(ImGui::GetMainViewport()); //enable docking

        ImGui::Begin("Main Window");

        static int item_current_idx = -1;
        int numPorts = 0;
        char** comPorts = StandBackend::listComPorts(numPorts);

        ImGui::SetCursorPos(ImVec2(edgeMargin, height - ImGui::GetFontSize() - edgeMargin));
        ImGui::PushItemWidth(ImGui::GetFontSize()*10);  //10 seems to work nicely with the preview text
        const char* current_item;
        if (item_current_idx == -1) {       //display instruction until an item is selected
            current_item = "Select a COM Port";
        }
        else { current_item = comPorts[item_current_idx]; }
        if (ImGui::BeginCombo("##COM Dropdown", current_item)) {
            for (int n = 0; n < numPorts; n++)
            {
                const bool is_selected = (item_current_idx == n);
                if (ImGui::Selectable(comPorts[n], is_selected))
                    item_current_idx = n;
                // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        ImGui::PopItemWidth();
        ImGui::SameLine();

        if (ImGui::Button("Connect")) {
            //COM port connect code
            StandBackend::closeComPort(hCom);
            comCon = false;
            if (strcmp(current_item, "Select a COM Port") != 0) {
                hCom = StandBackend::connectToComPort(current_item);
                comCon = true;
            }
        }

        static float t;
        static ScrollingBuffer tankPr, combPr, force, rssi;
        static int valvePerc;
        static bool QDStat, armStat, ignitStat;
        if (comCon && hCom != INVALID_HANDLE_VALUE) {
            char buffer[255];
            DWORD bytesRead = 0;
            if (!ReadFile(hCom, buffer, sizeof(buffer), &bytesRead, NULL)) {
                std::cerr << "Failed to read from COM port. Error: " << GetLastError() << std::endl;
                CloseHandle(hCom);
            }
            //sd:cout << bytesRead << std::endl;
            if (bytesRead % sizeof(dataPacket) == 0 && bytesRead > 0) { //if any multiple of a packet
                memcpy(&dataPacket, buffer, sizeof(dataPacket));
                t = dataPacket.timeStamp;
                tankPr.AddPoint(t, dataPacket.tankPrs);  //add points to graph buffers
                combPr.AddPoint(t, dataPacket.combnPrs);
                force.AddPoint(t, dataPacket.force);
                rssi.AddPoint(t, dataPacket.rssi);
                valvePerc = dataPacket.status >> 3;     //break apart the status byte into its constituent values
                if (valvePerc > 25) {
                    valvePerc = 25 + (valvePerc - 25) * 12.5;
                }
                QDStat = dataPacket.status & 0b100;
                armStat = dataPacket.status & 0b010;
                ignitStat = dataPacket.status & 0b001;

                /*for (int i = 0; i < sizeof(dataPacket); i++) {
                    printf("0x%02X ", buffer[i]&0xFF);
                }
                std::cout << std::endl;*/
            }
            else { t += ImGui::GetIO().DeltaTime; }     //smooth out scrolling by adding frame time to the view window if no packet
        }


        ImGui::SetCursorPos(ImVec2(width - sendItWidth - edgeMargin, height - sendItHeight - edgeMargin));  //SEND IT button
        ImGui::PushFont(io.Fonts->Fonts[1]);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.f, 0.2f, 0.2f, 1.f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.f, 0.2f, 0.2f, 0.75f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.941f, 0.439f, 0.235f, 1.f));
        ImGui::Button(fireText.c_str(), ImVec2(sendItWidth, sendItHeight));
        if (ImGui::IsItemActive()) {
            holdTimer = std::chrono::high_resolution_clock::now();
        }
        if (ImGui::IsMouseDown(0) && ImGui::IsItemHovered()) {
            if (!sent && armStat) {
                auto curTime = std::chrono::high_resolution_clock::now();
                if (curTime - holdTimer < std::chrono::seconds(1)) {
                    fireText = "3";
                }
                else if (curTime - holdTimer < std::chrono::seconds(2)) {
                    fireText = "2";
                }
                else if (curTime - holdTimer < std::chrono::seconds(3)) {
                    fireText = "1";
                }
                else {
                    if (comCon && hCom != INVALID_HANDLE_VALUE) {
                        char buff[1] = { 'F' };
                        DWORD bytesWritten = 0;
                        WriteFile(hCom, buff, 1, &bytesWritten, NULL);
                        sent = true;
                    }
                    else fireText = "ERROR";
                }
            }
            else if (sent) fireText = "SENT";
            else fireText = "NOT ARMED";
        }
        else {
            fireText = "SEND IT";
            sent = false;
        }

        ImGui::SetCursorPos(ImVec2(width - 2*sendItWidth - 2*edgeMargin, height - sendItHeight - edgeMargin));
        if (armStat) {
            armText = "DISARM";
        }
        else armText = "ARM";
        if (ImGui::Button(armText.c_str(), ImVec2(sendItWidth, sendItHeight))) {
            if (comCon && hCom != INVALID_HANDLE_VALUE) {
                char buff[1] = { 'A' };
                DWORD bytesWritten = 0;
                WriteFile(hCom, buff, 1, &bytesWritten, NULL);
            }
        }

        // Always center this window when appearing
        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        ImGui::PopFont();
        //ImGui::PushFont(io.Fonts->Fonts[2]);
        if (ImGui::BeginPopupModal("Authenticate", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            if (ImGui::Button("SEND IT", ImVec2(200, 0))) {
                if (comCon && hCom != INVALID_HANDLE_VALUE) {
                    char buff[1] = { 'A' };
                    DWORD bytesWritten = 0;
                    WriteFile(hCom, buff, 1, &bytesWritten, NULL);
                }
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::Button("Cancel", ImVec2(200,0))) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
            
        ImGui::PopStyleColor(3);
       

        static ImPlotAxisFlags flags = ImPlotAxisFlags_Lock;
        plotHeight = height / 6.f;

        Graphing::plotGraph(plotHeight, ImVec2(0, 0), "Tank Pressure [lbf/in^2]", "##Tank Pressure", tankPr, width, t, 0, 1000, flags);
        Graphing::plotGraph(plotHeight, ImVec2(0, plotHeight+6), "Combustion Pressure [lbf/in^2]", "##Combustion Pressure", combPr, width, t, 0, 1000, flags);
        Graphing::plotGraph(plotHeight, ImVec2(0, 2 * plotHeight+12), "Thrust [N]", "##Thrust", force, width, t, 0, 1000, flags);

        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        float valveChart = reMap(valvePerc, 0, 100, -5.f * PI / 4.f, PI/4.f);
        float arcWidth = 35.f;
        draw_list->PathArcTo(ImVec2((width-2*width/3)/2-100, height-350), 250.f, valveChart, -5*PI / 4.f);
        draw_list->PathStroke(IM_COL32(255,51,51,255), ImDrawFlags_None, arcWidth);
        draw_list->PathArcTo(ImVec2((width - 2 * width / 3) / 2 -100, height - 350), 250.f, PI/4.f, -5 * PI / 4.f);
        draw_list->PathStroke(IM_COL32(255, 51, 51, 50), ImDrawFlags_None, arcWidth);

        ImGui::PushFont(io.Fonts->Fonts[2]);
        TextCentered("Valve Position", ImVec2((width - 2 * width / 3) / 2 -100, height - 400));
        TextCentered(std::format("{}%%", valvePerc), ImVec2((width - 2 * width / 3) / 2 - 80, height - 350));
        ImGui::PopFont();

        draw_list->AddCircleFilled(ImVec2(width - 440, height - 452), 20.f, statusColor(QDStat));
        ImGui::SetCursorPos(ImVec2(width -400, height - 500));
        ImGui::Text("Quick Disconnect Status");
        draw_list->AddCircleFilled(ImVec2(width - 440, height - 402), 20.f, statusColor(armStat));
        ImGui::SetCursorPos(ImVec2(width -400, height - 450));
        ImGui::Text("Arming Status");
        draw_list->AddCircleFilled(ImVec2(width - 440, height - 352), 20.f, statusColor(ignitStat));
        ImGui::SetCursorPos(ImVec2(width -400, height - 400));
        ImGui::Text("Igniter Status");

        ImGui::SetCursorPos(ImVec2(width - 400, height - 600));
        ImGui::Text("%d", dataPacket.rssi);

        //ImGui::Begin("FPS Counter");  //display FPS in a dockable ImGUI window
        //ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();

        /*ImGui::Begin("Valve Position Slider");
        ImGui::SliderInt("##ValveSlider", &valvePerc, 0, 31);
        ImGui::Checkbox("Quick Disconnect Status", &QDStat);
        ImGui::Checkbox("Arming Status", &armStat);
        ImGui::Checkbox("Igniter Status", &ignitStat);
        ImGui::End();*/

        if (showDemo) { ImGui::ShowDemoWindow(); }

        StandBackend::freeCharArray(comPorts, numPorts);
    }

    //get rid of anything that needs to be destroyed at program close
    void destroyer() {  
        StandBackend::closeComPort(hCom);
    }
}
