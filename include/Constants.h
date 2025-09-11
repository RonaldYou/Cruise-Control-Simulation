#ifndef CONSTANTS_H
#define CONSTANTS_H

namespace PhysicsConstants {
    // Air density (kg/m³)
    const double RHO = 1.225;  // At 15°C, sea level
    
    // Gravitational acceleration (m/s²)
    const double GRAVITY = 9.81;
}

namespace SedanConstants {
    const double KERB_WEIGHT = 1310.0; //kg
    const double MAX_ENGINE_FORCE = 12333.33; //N
    const double DRAG_COEFFICIENT = 0.31;
    const double FRONTAL_AREA = 3.214; //m^2
    const double ROLLING_RESISTANCE_COEFFICIENT = 0.015;
}

namespace TerrainConstants {
    inline constexpr double HILL_CHANGE_PROBABILITY = 0.05;  // 5% chance per step
    inline constexpr double MAX_GRADE = 0.15;                // 15% max grade (steep!)
    inline constexpr double MIN_GRADE = -0.15;               // -15% max downhill
    inline constexpr double GRADE_SMOOTHING = 0.8;           // How gradually grade changes
    inline constexpr int DISPLAY_WIDTH = 80;                 // ASCII display width
    inline constexpr int DISPLAY_HEIGHT = 15;                // ASCII display height
}

namespace SimulationConstants{
    const double TIME_STEP = 0.2;  // seconds

    const long LOOP_DELAY = 200; //ms
}

#endif
