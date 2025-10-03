#pragma once
#include "imgui.h"
#include <GLFW/glfw3.h>
#include <windows.h>
#include <iostream>
#include "FrontEnd.h"
#include <string>

using namespace std;

namespace Graphing
{
   void plotGraph(float plotHeight, ImVec2 topLeft, const char* title, const char* label, StandGUI::ScrollingBuffer data, float width, float t, float minVal, float maxVal, ImPlotAxisFlags flags) {
        ImGui::SetCursorPos(topLeft);
        //ImPlot::PushColormap(ImPlotColormap_Pastel);
        if (ImPlot::BeginPlot(title, ImVec2(width, plotHeight))) {
            ImPlot::SetupAxes(nullptr, nullptr, flags, flags);
            ImPlot::SetupAxisLimits(ImAxis_X1, t - 15.f, t - 0.06, ImGuiCond_Always);
            ImPlot::SetupAxisLimits(ImAxis_Y1, minVal, maxVal);
            if (data.Data.size() > 0) {
                ImPlot::PushStyleColor(ImPlotCol_Line, ImVec4(1.f, 0.f, 0.f, 1.f));
                ImPlot::PushStyleColor(ImPlotCol_Fill, ImVec4(1.f, 0.f, 0.f, 0.5f));
                ImPlot::PlotLine(label, &data.Data[0].x, &data.Data[0].y, data.Data.size(), 0, data.Offset, 2 * sizeof(float));
                ImPlot::PushStyleVar(ImPlotStyleVar_FillAlpha, 0.25f);
                ImPlot::PlotShaded(label, &data.Data[0].x, &data.Data[0].y, data.Data.size(), -INFINITY, 0, data.Offset, 2 * sizeof(float));
                ImPlot::PopStyleVar();
                ImPlot::PopStyleColor(2);
            }
            ImPlot::EndPlot();
        }
    }
}
