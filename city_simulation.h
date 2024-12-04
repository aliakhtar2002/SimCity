#include <vector>
#include <iostream>
#include <string>
#include <limits>

using namespace std;

// Struct representing a single cell in the city grid
struct Cell {
    char type;          // Type of zone
    int population;     // Population of the zone (For Residential, Industrial, Commercial)
    int pollution;      // Pollution level for Industrial zones (affects nearby cells)
};

// Class representing the city grid and its simulation
class City {
public:
    // Existing function declarations...
    bool prioritizeGrowth();                 // Prioritizes growth of zones based on specified rules
    int getTotalAdjacentPopulation(int x, int y); // Gets the total adjacent population for a specific cell

    City(int width, int height);
    bool initializeGridFromCSV(const std::string& fileName);
    void simulate(int steps, int refreshRate);
    void displayRegion();
    void analyzeArea(int x1, int y1, int x2, int y2);

    // New functions for enhancements
    void displayFinalSummary();             // Displays the final simulation summary
    void promptForAreaAnalysis();           // Prompts user to input area for analysis

private:
    int width, height;
    vector<vector<Cell>> grid;
    int availableWorkers, availableGoods;

    void simulateStep();
    bool growResidential();
    bool growIndustrial();
    bool growCommercial();
    bool spreadPollution();
    bool isPowerAdjacent(int x, int y);
    bool hasAdjacentPopulation(int x, int y);
};
