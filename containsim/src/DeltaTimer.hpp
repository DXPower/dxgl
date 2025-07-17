#pragma once

class DeltaTimer {
    double m_last_time{};

public:
    double Tick();
};