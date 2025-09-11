#ifndef TERRAIN_H
#define TERRAIN_H

#include "Constants.h"

class Terrain{
    private:
        double currentGrade_;
        double targetGrade_;
        std::mt19937 rng_;
        std::uniform_real_distribution<double> gradeDist_;
        std::uniform_real_distribution<double> changeDist_;
        std::vector<double> elevationHistory_;

    public:
        Terrain();
        ~Terrain();

        void update(double dt);
        [[nodiscard]] double getCurrentGrade() const;
        [[nodiscard]] double getGradeForce(double Mass) const;
        std::vector<std::string> getASCIIDisplay() const;
        void addElevationPoint(double elevation);
};

#endif // TERRAIN_H