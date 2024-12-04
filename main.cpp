#include "city_simulation.h"  
#include <iostream>            
#include <fstream>             
#include <string>              
#include <algorithm>           

using namespace std;

int main() {
    // Configuration variables
    int grid_width = 8, grid_height = 9, simulation_steps = 20, refresh_rate = 1;
    string csvFileName;

    // Read configuration from config.txt
    // Open the configuration file for reading
    ifstream configFile("config.txt");
    if (!configFile) {
        cerr << "Error: Unable to open config.txt" << endl;
        return 1;  // Exit if config.txt cannot be opened
    }

    string line;
    // Read each line from the configuration file
    while (getline(configFile, line)) {
        size_t delimiterPos = line.find(':');  // Find position of delimiter ':'
        if (delimiterPos != string::npos) {
            string key = line.substr(0, delimiterPos);        // Extract the key (before ':')
            string value = line.substr(delimiterPos + 1);    // Extract the value (after ':')

            // Remove any leading or trailing whitespace from the value string
            value.erase(remove(value.begin(), value.end(), ' '), value.end());

            // Parse the configuration settings based on the key
            if (key == "Region Layout") {
                csvFileName = value;  // Set CSV file name for grid layout
            } else if (key == "Time Limit") {
                simulation_steps = std::stoi(value);  // Set number of simulation steps
            } else if (key == "Refresh Rate") {
                refresh_rate = std::stoi(value);  // Set refresh rate for simulation display
            }
        }
    }
    configFile.close();  // Close the config file

    // Create the city grid with the specified width and height
    City city(grid_width, grid_height);

    // Initialize the city grid from the provided CSV file
    if (!city.initializeGridFromCSV(csvFileName)) {
        cerr << "Error: Could not initialize grid from CSV file." << endl;
        return 1;  // Exit if grid initialization fails
    }

    // Run the simulation for the specified number of steps and refresh rate
    city.simulate(simulation_steps, refresh_rate);

    // Perform an area analysis on a specific region in the city grid (example coordinates)
    // Analyzes the area from (0, 0) to (2, 2) on the grid
    city.analyzeArea(0, 0, 2, 2);

    return 0;  
}
