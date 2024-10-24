#pragma once
#include "irwp.h"
#include "pendulum.h"
#include "implot/implot/implot.h"

#define PI 3.14159265358979323846f

// ImVec2 calcPendd(float &theta, float pendLen) {   
//     float x = pendLen * cos(theta + 1.5708);
//     float y = pendLen * sin(theta + 1.5708);
//     return ImVec2(x, y);
// }
// utility structure for realtime plot

// utility structure for realtime plot
struct RollingBuffer {
    float Span;
    ImVector<ImVec2> Data;
    RollingBuffer() {
        // Span = 2.0f;  // Time span to keep data (e.g., last 2 seconds)
        Data.reserve(2000);
    }
    
    void AddPoint(float x, float y) {
        // Add the point as usual
        Data.push_back(ImVec2(x, y));
        
        // Remove points older than the time span
        float currentTime = x;  // Assuming x represents time in seconds
        while (!Data.empty() && Data.front().x < currentTime - Span) {
            Data.erase(Data.begin());
        }
    }
};


float get_theta_w_dd(   
    float theta_p,  float theta_w, 
    float m_p,      float m_w, 
    float I_w,      float I_p, 
    float t,        
    float g,
    float L_p,      float L_w) {

    float num = 4 * I_p * t + 4 * I_w * t + m_p * L_p*L_p * t + 4 * m_w * L_p*L_p * t + 4 * I_w * m_w * g * L_p * cos(theta_p);
    float den = 4 * I_p*I_p - 4 * I_w*I_w + 4 * I_p * I_w + I_p * m_p * L_p*L_p + 4 * I_p * m_w * L_p*L_p;

    if (den == 0) {
        return 0;
    } else {
        return num/den;
    }
}

float get_theta_p_dd(   
    float theta_p,  float theta_w, 
    float m_p,      float m_w, 
    float I_w,      float I_p, 
    float t, 
    float g,
    float L_p, float L_w) {
    
    float num = -4 * ((I_w * t) + (I_p * m_w * g * L_p * cos(theta_p)));
    float den = 4 * I_p * I_p - 4 * I_w * I_w + 4 * I_p * I_w + I_p * m_p * L_p * L_p + 4 * I_p * m_w * L_p * L_p;
    
    if (den == 0) {
        return 0;
    } else {
        return num/den;
    }
    
}

// Increment IRWP forward in time
void updateIrwp(float deltaTime, 
    float &theta_p,     float &theta_w,             // angle
    float &theta_p_d,   float &theta_w_d,           // 1st der
    float m_p,  float m_w, float I_w, float I_p,    // param
    float g,    float L_p, float L_w, float &t) {

    // calculate torque
    // if (ImGui::Button("enable controller")) {
    float k1 = -407.6380f;
    float k2 = -37.4618f;
    float k3 = -189.3853f;
    float k4 = -34.6967f;
    // for less crazy response:
    // float k1 = -25.7246f;
    // float k2 = -0.1338f;
    // float k3 = -11.2586f;
    // float k4 = -0.5575f;
    // t = (-k1 * theta_p) + (-k2 * theta_w) + (-k3 * theta_p_d) + (-k4 * theta_w_d);
    
    // limit torque
    // float torque_limit = 40.0f;
    float torque = (-k1 * theta_p) + (-k2 * theta_w) + (-k3 * theta_p_d) + (-k4 * theta_w_d);
    // if ( torque > torque_limit) {
    //     t = torque_limit;  
    // } else if (torque < -torque_limit) {
    //     t = -torque_limit;
    // }
    t = torque;

    float theta_p_dd = get_theta_p_dd(theta_p, theta_w, m_p, m_w, I_w, I_p, t, g, L_p, L_w);
    float theta_w_dd = get_theta_w_dd(theta_p, theta_w, m_p, m_w, I_w, I_p, t, g, L_p, L_w);
    
    
    
    // Update angular velocities
    theta_p_d += theta_p_dd * deltaTime;
    theta_w_d += theta_w_dd * deltaTime;

    // Update angles based on velocities
    theta_p += theta_p_d * deltaTime;
    theta_w += theta_w_d * deltaTime;

}


void showIrwp(ImGuiIO& io) {
    {
        // make full screen
        static bool use_work_area = true;
        // static ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings;

        // const ImGuiViewport* viewport = ImGui::GetMainViewport();
        // ImGui::SetNextWindowPos(use_work_area ? viewport->WorkPos : viewport->Pos);
        // ImGui::SetNextWindowSize(use_work_area ? viewport->WorkSize : viewport->Size); 
        
        // Define pendulum properties
        static float theta_p = (-PI/2) - 0.5;        // pendulum angle (radian)
        static float theta_w = 17.0f;        // wheel angle (radian) // why does this need to be changed
        static float theta_p_d = 0.0f;      // pendulum angular velocity (rad/s)
        static float theta_w_d = 0.0f;      // wheel angular velocity (rad/s)
        static float L_p = 1.0f;            // pendulum length (meter)
        static float L_w = 0.5f;            // wheel radius (meter)
        static float g = -9.81f;              // gravity (m/s2)

        static float I_p = 0.5f;            // pendulum moment of inertia (kg/m2)
        static float I_w = 0.5f;            // wheel moment of inertia  (kg/m2)
        static float m_p = 1.0f;            // pendulum mass (kg)
        static float m_w = 1.0f;            // wheel mass (kg)
        static float t = 0.0f;              // torque (Nm)

        // Add sliders to make the simulation interactive
        ImGui::Begin("Controls");
            ImGui::SliderFloat("pendulum angle (adjust this!)", &theta_p, -2.0f, 0.0f, "%.2f rad");
            ImGui::BeginDisabled(true);
            // ImGui::SliderFloat("wheel angle", &theta_w, -100.0f, 100.0f, "%.2f rad");
            // ImGui::SliderFloat("pendulum length", &L_p, 0.1f, 5.0f, "%.2f m");
            // ImGui::SliderFloat("pendulum inertia", &I_p, 0.1f, 5.0f, "%.2f kg/m2");
            // ImGui::SliderFloat("wheel inertia", &I_w, 0.1f, 5.0f, "%.2f kg/m2");
            // ImGui::SliderFloat("wheel radius", &L_w, 0.1f, 5.0f, "%.2f m");
            // ImGui::SliderFloat("theta_p", &theta_p, -15.0, 15.0, "%.2f m");
            // ImGui::SliderFloat("theta_w", &theta_w, -100.0, 100.0, "%.2f m");
            // ImGui::SliderFloat("theta_p_d", &theta_p_d, -15.0, 15.0);
            // ImGui::SliderFloat("theta_w_d", &theta_w_d, -15.0, 15.0);
            ImGui::SliderFloat("wheel torque", &t, -50.0, 50.0, "%.2f Nm");
            // ImGui::SliderFloat("damping term", &mu, 0.01f, 5.0f);
            // ImGui::SliderFloat("gravity", &g, 0.1f, 20.0f);
            // ImGui::Text("Pendulum position x: %.2f, y: %.2f", posX, posY);
            ImGui::EndDisabled();
        ImGui::End();

        // Draw pendulum shapes
        ImGui::Begin("drawing");

            // establish colors
            const ImU32 col = IM_COL32(190, 200, 200, 255);     // generic white for contrast, frame
            const ImU32 col_red = IM_COL32(255, 20, 6, 255);    // red, for wheel orientation

            ImDrawList* draw_list = ImGui::GetWindowDrawList(); // needed to draw simple shapes
            const ImVec2 p = ImGui::GetCursorScreenPos();   
            
            ImVec2 dCenter = ImVec2(p.x + (ImGui::GetWindowWidth() * 0.5f), p.y + (ImGui::GetWindowHeight() * 0.6f));
            static float thickness = 6.0f;

            // Calculate where the end of the pendulum (x,y coords) should be placed
            ImVec2 pos_p = calcPend(theta_p-1.5707963267949, L_p*100); // 1 m is 100 px in this world
            ImVec2 pos_w = calcPend((theta_p-1.5707963267949 + theta_w), L_w*100);
            ImVec2 pos_w_inv = calcPend((theta_p-1.5707963267949 + theta_w - PI), L_w*100);

            ImVec2 dPos_p = ImVec2(dCenter.x + pos_p.x, dCenter.y + pos_p.y); // d for display
            ImVec2 dPos_w = ImVec2(dPos_p.x + pos_w.x, dPos_p.y + pos_w.y);
            ImVec2 dPos_w_inv = ImVec2(dPos_p.x + pos_w_inv.x, dPos_p.y + pos_w_inv.y);

            draw_list->AddLine(dCenter, dPos_p, col, thickness);
            draw_list->AddLine(dPos_p, dPos_w, col_red, thickness); // real position
            draw_list->AddLine(dPos_p, dPos_w_inv, col, thickness); // visual aid to see wheel moves
            draw_list->AddCircleFilled(dCenter, 5.0f, col);
            draw_list->AddCircleFilled(dPos_p, 5.0f, col);
            draw_list->AddCircle(dPos_p, L_w*100, col, 0, thickness);

            // show the people you're cool by printing the framerate
            ImVec2 windowSize = ImGui::GetWindowSize();
            ImGui::SetCursorPos(ImVec2(0, windowSize.y - ImGui::GetFontSize() - ImGui::GetStyle().FramePadding.y * 2));
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);

        ImGui::End();

        ImGui::Begin("Pendulum Phase Plane"); {

            // Setup rolling buffer for disappearing line
            static RollingBuffer rdata1, rdata2;
            static float ti = 0;
            static float history = 0.4f;
            
            ti += ImGui::GetIO().DeltaTime;
            rdata1.AddPoint(ti, theta_p);
            rdata2.AddPoint(ti, theta_p_d);
            rdata1.Span = history;
            rdata2.Span = history;
            // ImGui::SliderFloat("History",&history,0.1f,10.0f,"%.1f s");
            
            // Visual settings
            // static ImPlotAxisFlags flags = ImPlotAxisFlags_NoTickLabels;
            float mk_size = 5.0f;
            float mk_weight = 0.0f;
            float ln_weight = 2.5f;
            ImVec4 mk_col_org = ImVec4(0.960f, 0.661f, 0.234f, 1.0f); // nice orange

            if (ImPlot::BeginPlot("##Scrolling", ImVec2(-1,-1), ImPlotFlags_NoLegend)) {
                
                // setup axes 
                ImPlot::SetupAxes("theta_p", "theta_p_d");
                ImPlot::SetupAxisLimits(ImAxis_X1,-4, 1);
                ImPlot::SetupAxisLimits(ImAxis_Y1,-15, 10);
                
                // plot point
                ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle, mk_size, mk_col_org, mk_weight, mk_col_org);
                ImPlot::PlotScatter("state", &theta_p, &theta_p_d, 1, 0, 0, 2 * sizeof(float));

                // plot disappearing line
                ImPlot::PushStyleVar(ImPlotStyleVar_LineWeight, ln_weight);
                // ImPlot::SetNextFillStyle(mk_col_org, 1.0f);
                ImPlot::SetNextLineStyle(mk_col_org);
                ImPlot::PlotLine("pendulum state", &rdata1.Data[0].y, &rdata2.Data[0].y, rdata1.Data.size(), 0, 0, 2 * sizeof(float));
                ImPlot::PopStyleVar();

                ImPlot::EndPlot();
            }
        }
        ImGui::End();

        ImGui::Begin("Wheel Phase Plane");{

            // Setup rolling buffer for disappearing line
            static RollingBuffer rdata1, rdata2;
            static float ti = 0;
            static float history = 0.4f;

            ti += ImGui::GetIO().DeltaTime;
            rdata1.AddPoint(ti, theta_w);
            rdata2.AddPoint(ti, theta_w_d);
            rdata1.Span = history;
            rdata2.Span = history;
            // ImGui::SliderFloat("History",&history,0.1f,10.0f,"%.1f s");

            // Visual settings
            // static ImPlotAxisFlags flags = ImPlotAxisFlags_NoTickLabels;
            float mk_size = 5.0f;
            float mk_weight = 0.0f;
            float ln_weight = 2.5f;
            ImVec4 mk_col_blue = ImVec4(0.05f, 0.38f, 0.75f, 1.0f); // nice blue

            if (ImPlot::BeginPlot("##Scrolling", ImVec2(-1,-1), ImPlotFlags_NoLegend)) {

                // setup axes
                ImPlot::SetupAxes("theta_p", "theta_p_d");
                ImPlot::SetupAxisLimits(ImAxis_X1,-6, 24);
                ImPlot::SetupAxisLimits(ImAxis_Y1,-10, 10);

                // plot point
                ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle, mk_size, mk_col_blue, mk_weight, mk_col_blue);
                ImPlot::PlotScatter("state", &theta_w, &theta_w_d, 1, 0, 0, 2 * sizeof(float));
                
                // plot disappearing line
                ImPlot::PushStyleVar(ImPlotStyleVar_LineWeight, ln_weight);
                ImPlot::SetNextFillStyle(mk_col_blue, 1.0f);
                ImPlot::PlotLine("pendulum state", &rdata1.Data[0].y, &rdata2.Data[0].y, rdata1.Data.size(), 0, 0, 2 * sizeof(float));
                ImPlot::PopStyleVar();

                ImPlot::EndPlot();
            }
        }
        ImGui::End();

        // Update the pendulum according to how much time has passed
        static float previousTime = ImGui::GetTime();
        float currentTime = ImGui::GetTime();
        float deltaTime = currentTime - previousTime;
        previousTime = currentTime;



        // updatePendulum(deltaTime, theta, theta_dot, mu, g, L);
        // if (ImGui::Button("step")) {
        updateIrwp(deltaTime, theta_p, theta_w, theta_p_d, theta_w_d, m_p, m_w, I_w, I_p, g, L_p, L_w, t);
        // }

        

        // ImGui::End();
    }
}