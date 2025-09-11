#include "Terrain.h"

Terrain::Terrain()
    : currentGrade_ (0.0),
      targetGrade_ (0.0),
      rng_ (std::random_device{}()),
      gradeDist_ (TerrainConstants::MIN_GRADE, TerrainConstants::MAX_GRADE),
      changeDist_ (0.0, 1.0){
        elevationHistory_.reserve(TerrainConstants::DISPLAY_WIDTH);
}

void Terrain::update(double dt){
    //randomly decide to change target grade
    if(changeDist_(rng_) < TerrainConstants::CHANGE_PROBABILITY){
        targetGrade_ = gradeDist_(rng_);
    }

    double gradeDiff = targetGrade_ - currentGrade_;
    currentGrade_ += gradeDiff * TerrainConstants::GRADE_CHANGE_SPEED * dt;
}

double Terrain::getGradeForce(double mass) const{
    return -mass * PhysicsConstants::GRAVITY * currentGrade_;
}

void Terrain::addElevationPoint(double elevation){
    if(elevationHistory_.size() == TerrainConstants::DISPLAY_WIDTH){
        elevationHistory_.erase(elevationHistory_.begin()); //runtime of this?
    }

    elevationHistory_.push_back(elevation);
}

std::vector<std::string> Terrain::getASCIIDisplay() const{
    std::vector<std::string> display(TerrainConstants::DISPLAY_HEIGHT + 3);

    if (elevationHistory_.empty()) {
        for (auto& line : display) {
            line = std::string(TerrainConstants::DISPLAY_WIDTH, ' ');
        }
        return display;
    }

    auto minMax = std::minmax_element(elevationHistory.begin(), elevationHistory.end());
    double minElev = *minMax.first;
    double maxElev = *minMax.second;
    double elevRange = maxElev - minElev;

    if(elevRange < 1.0) elevRange = 1.0;

    //clear display
    for(auto& line: display){
        line = std::string(TerrainConstants::DISPLAY_WIDTH, ' ');
    }

    for (size_t i = 0; i < elevationHistory_.size(); ++i) {
        double normalizedElev = (elevationHistory_[i] - minElev) / elevRange;
        int row = static_cast<int>((1.0 - normalizedElev) * (TerrainConstants::DISPLAY_HEIGHT - 1));
        row = std::clamp(row, 0, TerrainConstants::DISPLAY_HEIGHT - 1);
        
        if (i < TerrainConstants::DISPLAY_WIDTH) {
            display[row][i] = '█';
            
            // Fill below the terrain line
            for (int fillRow = row + 1; fillRow < TerrainConstants::DISPLAY_HEIGHT; ++fillRow) {
                display[fillRow][i] = '▓';
            }
        }
    }

    // Add car at the rightmost position
    if (!elevationHistory_.empty()) {
        double carElev = elevationHistory_.back();
        double normalizedCarElev = (carElev - minElev) / elevRange;
        int carRow = static_cast<int>((1.0 - normalizedCarElev) * (TerrainConstants::DISPLAY_HEIGHT - 1));
        carRow = std::clamp(carRow, 0, TerrainConstants::DISPLAY_HEIGHT - 1);
        
        if (carRow > 0) {
            display[carRow - 1][TerrainConstants::DISPLAY_WIDTH - 1] = '🚗';  // Car above terrain
        }
    }

    std::ostringstream gradeInfo;
    gradeInfo << "Current Grade: " << std::fixed << std::setprecision(1) 
              << (currentGrade_ * 100) << "%";
    display[TerrainConstants::DISPLAY_HEIGHT] = gradeInfo.str();
    
    std::string gradeIndicator = (currentGrade_ > 0.02) ? " ⬆️ UPHILL" : 
                                (currentGrade_ < -0.02) ? " ⬇️ DOWNHILL" : " ➡️ FLAT";
    display[TerrainConstants::DISPLAY_HEIGHT + 1] = gradeIndicator;

    return display;
}