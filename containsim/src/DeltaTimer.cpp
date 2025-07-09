#include "DeltaTimer.hpp"

#include <dxgl/Application.hpp>

double DeltaTimer::Tick() {
    double current_time = dxgl::Application::GetTime();
    double delta_time = current_time - m_last_time;
    m_last_time = current_time;
    return delta_time;
}