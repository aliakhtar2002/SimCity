#include "city_simulation.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <limits> // For numeric_limits

using namespace std;

// Constructor to initialize the city grid with default values
City::City(int width, int height) : width(width), height(height) {
    grid.resize(height, vector<Cell>(width));
    availableWorkers = 0;
    availableGoods = 0;
}

// Function to initialize the city grid from a CSV file
bool City::initializeGridFromCSV(const string& fileName) {
    ifstream file(fileName);
    if (!file) {
        cerr << "Error: Unable to open " << fileName << endl;
        return false;
    }

    string line;
    int y = 0;

    while (getline(file, line) && y < height) {
        stringstream ss(line);
        string cellValue;
        int x = 0;

        while (getline(ss, cellValue, ',') && x < width) {
            cellValue.erase(std::remove(cellValue.begin(), cellValue.end(), ' '), cellValue.end());

            if (cellValue.empty()) {
                grid[y][x].type = '-';
            } else {
                switch (cellValue[0]) {
                    case 'I': grid[y][x].type = 'I'; break;
                    case 'C': grid[y][x].type = 'C'; break;
                    case 'R': grid[y][x].type = 'R'; break;
                    case 'T': grid[y][x].type = 'T'; break;
                    case 'P': grid[y][x].type = 'P'; break;
                    case '#': grid[y][x].type = '#'; break;
                    default: grid[y][x].type = '-'; break;
                }
            }

            grid[y][x].population = 0;
            grid[y][x].pollution = 0;
            x++;
        }
        y++;
    }

    file.close();
    return true;
}

// Simulate the city growth for a given number of steps, with periodic display updates
void City::simulate(int steps, int refreshRate) {
    cout << "Initial Region State (Time Step 0):" << endl;
    displayRegion();
    cout << "Available Workers: " << availableWorkers << "\n";
    cout << "Available Goods: " << availableGoods << "\n\n";

    for (int step = 1; step <= steps; ++step) {
        simulateStep();

        if (step % refreshRate == 0) {
            cout << "Time Step " << step << ":\n";
            cout << "Available Workers: " << availableWorkers << "\n";
            cout << "Available Goods: " << availableGoods << "\n";
            displayRegion();
            cout << endl;
        }
    }

    cout << "Simulation completed after " << steps << " time steps.\n";
    displayFinalSummary();
    promptForAreaAnalysis();
}

// Display the final simulation summary
void City::displayFinalSummary() {
    int totalPopulation = 0;
    int totalPollution = 0;

    for (const auto& row : grid) {
        for (const auto& cell : row) {
            totalPopulation += cell.population;
            totalPollution += cell.pollution;
        }
    }

    cout << "\n--- Final Simulation Summary ---\n";
    cout << "Total Population: " << totalPopulation << "\n";
    cout << "Total Pollution: " << totalPollution << "\n";
    cout << "Total Available Workers: " << availableWorkers << "\n";
    cout << "Total Available Goods: " << availableGoods << "\n";
}

// Function to prompt the user for area analysis coordinates
void City::promptForAreaAnalysis() {
    int x1, y1, x2, y2;
    cout << "\nEnter the coordinates for area analysis (x1, y1) to (x2, y2):\n";

    while (true) {
        cout << "x1, y1: ";
        cin >> x1 >> y1;
        cout << "x2, y2: ";
        cin >> x2 >> y2;

        if (cin.fail() || x1 < 0 || y1 < 0 || x2 >= width || y2 >= height || x1 > x2 || y1 > y2) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input. Please enter valid coordinates within the grid dimensions.\n";
        } else {
            break;
        }
    }

    analyzeArea(x1, y1, x2, y2);
}

// Simulate a single step of city growth
void City::simulateStep() {
    vector<vector<Cell>> previousGrid = grid;
    bool gridChanged = false;

    cout << "\nSimulating step...\n";

    gridChanged |= prioritizeGrowth();

    gridChanged |= spreadPollution();

    bool gridIdentical = true;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (grid[y][x].type != previousGrid[y][x].type ||
                grid[y][x].population != previousGrid[y][x].population ||
                grid[y][x].pollution != previousGrid[y][x].pollution) {
                gridIdentical = false;
                break;
            }
        }
        if (!gridIdentical) break;
    }

    if (!gridChanged || gridIdentical) {
        cout << "Simulation halted: No visible or functional changes detected.\n";
        displayFinalSummary();
        promptForAreaAnalysis();
        exit(0);
    }
}

// Function to prioritize the growth of cells based on the provided guidelines
bool City::prioritizeGrowth() {
    bool changed = false;
    vector<pair<int, int>> growthCandidates;

    // Gather all growth candidates
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if ((grid[y][x].type == 'R' && grid[y][x].population < 5 && isPowerAdjacent(x, y)) ||
                (grid[y][x].type == 'I' && grid[y][x].population == 0 && availableWorkers >= 2 && isPowerAdjacent(x, y)) ||
                (grid[y][x].type == 'C' && grid[y][x].population == 0 && availableWorkers >= 1 && availableGoods >= 1 && isPowerAdjacent(x, y))) {
                growthCandidates.push_back({y, x});
            }
        }
    }

    // Sort candidates based on priority rules
    std::sort(growthCandidates.begin(), growthCandidates.end(), [&](const pair<int, int>& a, const pair<int, int>& b) {
        int yA = a.first, xA = a.second;
        int yB = b.first, xB = b.second;
        Cell& cellA = grid[yA][xA];
        Cell& cellB = grid[yB][xB];

        // Priority: Commercial > Industrial > Residential
        if (cellA.type != cellB.type) {
            if (cellA.type == 'C') return true;
            if (cellB.type == 'C') return false;
            if (cellA.type == 'I') return true;
            if (cellB.type == 'I') return false;
        }

        // Priority: Higher population
        if (cellA.population != cellB.population) {
            return cellA.population > cellB.population;
        }

        // Priority: Greater total adjacent population
        int adjacentPopA = getTotalAdjacentPopulation(xA, yA);
        int adjacentPopB = getTotalAdjacentPopulation(xB, yB);
        if (adjacentPopA != adjacentPopB) {
            return adjacentPopA > adjacentPopB;
        }

        // Priority: Smaller Y coordinate
        if (yA != yB) {
            return yA < yB;
        }

        // Priority: Smaller X coordinate
        return xA < xB;
    });

    // Apply growth to each candidate based on its type
    // Apply growth to each candidate based on its type
for (const auto& candidate : growthCandidates) {
    int y = candidate.first;
    int x = candidate.second;

    if (grid[y][x].type == 'R') {
        // Grow residential
        if (grid[y][x].population < 5 && isPowerAdjacent(x, y)) {
            grid[y][x].population++;
            availableWorkers++;
            changed = true;
        }
    } else if (grid[y][x].type == 'I') {
        // Grow industrial (requires 2 workers)
        if (availableWorkers >= 2 && grid[y][x].population == 0 && isPowerAdjacent(x, y)) {
            grid[y][x].population++;
            availableWorkers -= 2;
            availableGoods += grid[y][x].population;
            changed = true;
        }
    } else if (grid[y][x].type == 'C') {
        // Grow commercial (requires 1 worker and 1 good)
        if (availableWorkers >= 1 && availableGoods >= 1 && grid[y][x].population == 0 && isPowerAdjacent(x, y)) {
            grid[y][x].population++;
            availableWorkers--;
            availableGoods--;
            changed = true;
        }
    }
}


    return changed;
}

// Function to get the total adjacent population for a cell
int City::getTotalAdjacentPopulation(int x, int y) {
    int totalPopulation = 0;
    for (int i = -1; i <= 1; ++i) {
        for (int j = -1; j <= 1; ++j) {
            int ny = y + i, nx = x + j;
            if (ny >= 0 && ny < height && nx >= 0 && nx < width && !(i == 0 && j == 0)) {
                totalPopulation += grid[ny][nx].population;
            }
        }
    }
    return totalPopulation;
}


// Grow residential zones by adding population and workers if conditions are met
// Returns true if any changes occurred
bool City::growResidential() {
    bool changed = false;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (grid[y][x].type == 'R' && isPowerAdjacent(x, y)) {
                // Allow growth only if there is space for more people (max population = 5)
                if (grid[y][x].population < 5) {
                    grid[y][x].population++;
                    availableWorkers++;
                    changed = true;
                }
            }
        }
    }
    return changed;
}

// Grow industrial zones by adding population and producing goods if conditions are met
// Returns true if any changes occurred
bool City::growIndustrial() {
    bool changed = false;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (grid[y][x].type == 'I' && grid[y][x].population == 0 && isPowerAdjacent(x, y)) {
                // Industrial zones require 2 workers
                if (availableWorkers >= 2) {
                    grid[y][x].population++;
                    availableWorkers -= 2;
                    availableGoods += grid[y][x].population;  // Goods production based on population
                    changed = true;
                }
            }
        }
    }
    return changed;
}

// Grow commercial zones by adding population if conditions are met (requires both workers and goods)
// Returns true if any changes occurred
bool City::growCommercial() {
    bool changed = false;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (grid[y][x].type == 'C' && grid[y][x].population == 0 && isPowerAdjacent(x, y)) {
                // Commercial zones require 1 worker and 1 good
                if (availableWorkers >= 1 && availableGoods >= 1) {
                    grid[y][x].population++;
                    availableWorkers--;
                    availableGoods--;
                    changed = true;
                }
            }
        }
    }
    return changed;
}

// Spread pollution from industrial zones to adjacent cells
// Returns true if any pollution spread occurred
bool City::spreadPollution() {
    bool pollutionSpread = false;
    vector<vector<int>> newPollution(height, vector<int>(width, 0));

    // Iterate over the grid to calculate new pollution levels
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (grid[y][x].type == 'I' && grid[y][x].population > 0) {
                int pollutionAmount = grid[y][x].population;
                newPollution[y][x] += pollutionAmount;

                // Spread pollution to adjacent cells with decreasing intensity
                for (int i = -1; i <= 1; ++i) {
                    for (int j = -1; j <= 1; ++j) {
                        int newY = y + i, newX = x + j;
                        if (i == 0 && j == 0) continue; // Skip the current cell
                        if (newY >= 0 && newY < height && newX >= 0 && newX < width) {
                            newPollution[newY][newX] += max(0, pollutionAmount - 1);
                            pollutionSpread = true;
                        }
                    }
                }
            }
        }
    }
    // Update the grid with new pollution levels
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            grid[y][x].pollution = newPollution[y][x];
        }
    }

    return pollutionSpread;
}


// Check if there is any adjacent power source (powerline or power plant)
bool City::isPowerAdjacent(int x, int y) {
    for (int i = -1; i <= 1; ++i) {
        for (int j = -1; j <= 1; ++j) {
            int ny = y + i, nx = x + j;
            if (ny >= 0 && ny < height && nx >= 0 && nx < width) {
                if (grid[ny][nx].type == 'T' || grid[ny][nx].type == 'P') {
                    return true;  // Found adjacent power source
                }
            }
        }
    }
    return false;
}

// Display the current state of the region (grid) on the console
void City::displayRegion() {
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            // Display population or type if there is no population
            if (grid[y][x].population == 0) {
                cout << grid[y][x].type << " ";
            } else {
                cout << grid[y][x].population << " ";
            }
        }
        cout << endl;
    }
}

// Analyze a specific area in the city grid, calculating total population and pollution
// - x1, y1: top-left corner of the area
// - x2, y2: bottom-right corner of the area
void City::analyzeArea(int x1, int y1, int x2, int y2) {
    int totalPopulation = 0, totalPollution = 0;

    for (int y = y1; y <= y2; ++y) {
        for (int x = x1; x <= x2; ++x) {
            totalPopulation += grid[y][x].population;
            totalPollution += grid[y][x].pollution;
        }
    }

    // Output analysis results
    cout << "Area Analysis from (" << x1 << ", " << y1 << ") to (" << x2 << ", " << y2 << "):" << endl;
    cout << "Total Population: " << totalPopulation << endl;
    cout << "Total Pollution: " << totalPollution << endl;
}
