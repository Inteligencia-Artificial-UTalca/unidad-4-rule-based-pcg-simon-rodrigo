#include <iostream>
#include <vector>
#include <random>   // For random number generation
#include <chrono>   // For seeding the random number generator

// Define Map as a vector of vectors of integers.
// You can change 'int' to whatever type best represents your cells (e.g., char, bool).
using Map = std::vector<std::vector<int>>;

/**
 * @brief Prints the map (matrix) to the console.
 * @param map The map to print.
 */
void printMap(const Map& map) {
    std::cout << "--- Current Map ---" << std::endl;
    for (const auto& row : map) {
        for (int cell : row) {
            // Adapt this to represent your cells meaningfully (e.g., ' ' for empty, '#' for occupied).
            std::cout << cell << " ";
        }
        std::cout << std::endl;
    }
    std::cout << "-------------------" << std::endl;
}

/**
 * @brief Function to implement the Cellular Automata logic.
 * It should take a map and return the updated map after one iteration.
 * @param currentMap The map in its current state.
 * @param W Width of the map.
 * @param H Height of the map.
 * @param R Radius of the neighbor window (e.g., 1 for 3x3, 2 for 5x5).
 * @param U Threshold to decide if the current cell becomes 1 or 0.
 * @return The map after applying the cellular automata rules.
 */
Map cellularAutomata(const Map& currentMap, int W, int H, int R, double U) {
    Map newMap = currentMap; // Initially, the new map is a copy of the current one

    // TODO: IMPLEMENTATION GOES HERE for the cellular automata logic.
    // Iterate over each cell and apply the transition rules.
    // Remember that updates should be based on the 'currentMap' state
    // and applied to the 'newMap' to avoid race conditions within the same iteration.

    return newMap;
}

void changeDirection(int& x, int& y, int dir, int w, int h)
{
    bool canContinue = true;

    std::random_device rd;
    std::mt19937 mt(rd());

    while (canContinue)
    {
         switch (dir)
        {
        case 0:
            if(y + 1 >= h)
            {
                std::uniform_int_distribution<int> dist(0, 3);
                dir =  dist(mt);
                break;
            }
            y++;
            canContinue = false;
            break;
        case 1:
            if(y - 1 < 0)
            {
                std::uniform_int_distribution<int> dist(0, 3);
                dir =  dist(mt);
                break;
            }
            y--;
            canContinue = false;
            break;
        case 2:
            if(x + 1 >= w)
            {
                std::uniform_int_distribution<int> dist(0, 3);
                dir =  dist(mt);
                break;
            }
            x++;
            canContinue = false;
            break;
        case 3:
            if(x - 1 < 0)
            {
                std::uniform_int_distribution<int> dist(0, 3);
                dir =  dist(mt);
                break;
            }
            x--;
            canContinue = false;
            break;
        default:
            break;
        }
    }
}

/**
 * @brief Function to implement the Drunk Agent logic.
 * It should take a map and parameters controlling the agent's behavior,
 * then return the updated map after the agent performs its actions.
 *
 * @param currentMap The map in its current state.
 * @param W Width of the map.
 * @param H Height of the map.
 * @param J The number of times the agent "walks" (initiates a path).
 * @param I The number of steps the agent takes per "walk".
 * @param roomSizeX Max width of rooms the agent can generate.
 * @param roomSizeY Max height of rooms the agent can generate.
 * @param probGenerateRoom Probability (0.0 to 1.0) of generating a room at each step.
 * @param probIncreaseRoom If no room is generated, this value increases probGenerateRoom.
 * @param probChangeDirection Probability (0.0 to 1.0) of changing direction at each step.
 * @param probIncreaseChange If direction is not changed, this value increases probChangeDirection.
 * @param agentX Current X position of the agent (updated by reference).
 * @param agentY Current Y position of the agent (updated by reference).
 * @return The map after the agent's movements and actions.
 */
Map drunkAgent(const Map& currentMap, int W, int H, int J, int I, int roomSizeX, int roomSizeY,
               double probGenerateRoom, double probIncreaseRoom,
               double probChangeDirection, double probIncreaseChange,
               int& agentX, int& agentY) {
    Map newMap = currentMap; // The new map is a copy of the current one
    
    std::random_device rd;
    std::mt19937 gen(rd());
    // TODO: IMPLEMENTATION GOES HERE for the Drunk Agent logic.
    // The agent should move randomly.
    // You'll need a random number generator.
    // Consider:
    // - How the agent moves (possible steps).
    // - What it does if it encounters a border or an obstacle (if applicable).
    // - How it modifies the map (e.g., leaving a trail, creating rooms, etc.).
    // - Use the provided parameters (J, I, roomSizeX, roomSizeY, probabilities)
    //   to control its behavior.
    std::uniform_real_distribution<float> distFloat(0.0, 1.1);
    std::srand(std::time(0));
    int currentDir = 0;

    for(int i = 0; i < J; i++)
    {
        for(int j = 0; j < I; j++)
        {
            int direction = std::rand() % 4;
            float changeProb = distFloat(gen);
            

            if(j == 0)
            {
                changeDirection(agentX, agentY, direction, W, H);
                currentDir = direction;
            }
            else if(j != 0 && changeProb <= probChangeDirection)
            {
                currentDir = direction;
                changeDirection(agentX, agentY, currentDir, W, H);
            }
            else
            {
                changeDirection(agentX, agentY, currentDir, W, H);
            }

            newMap[agentX][agentY] = 1;
        }
    }
    /*
       for(cantidad de caminatas)
       {
         for(cantidad de pasos)
         {
            //empieza a moverse
            x , y;

            random;

            switch(random)
            case1:
               direcciones...
            //termina de moverse
         }
        
         
       }
    */

    return newMap;
}

int main() {
    std::cout << "--- CELLULAR AUTOMATA AND DRUNK AGENT SIMULATION ---" << std::endl;

    // --- Initial Map Configuration ---
    int mapRows = 10;
    int mapCols = 20;
    Map myMap(mapRows, std::vector<int>(mapCols, 0)); // Map initialized with zeros

    // TODO: IMPLEMENTATION GOES HERE: Initialize the map with some pattern or initial state.
    // For example, you might set some cells to 1 for the cellular automata
    // or place the drunk agent at a specific position.

    // Drunk Agent's initial position
    int drunkAgentX = mapRows / 2;
    int drunkAgentY = mapCols / 2;
    // If your agent modifies the map at start, you could do it here:
    // myMap[drunkAgentX][drunkAgentY] = 2; // Assuming '2' represents the agent

    std::cout << "\nInitial map state:" << std::endl;
    printMap(myMap);

    // --- Simulation Parameters ---
    int numIterations = 5; // Number of simulation steps

    // Cellular Automata Parameters
    int ca_W = mapCols;
    int ca_H = mapRows;
    int ca_R = 1;      // Radius of neighbor window
    double ca_U = 0.5; // Threshold

    // Drunk Agent Parameters
    int da_W = mapCols;
    int da_H = mapRows;
    int da_J = 5;      // Number of "walks"
    int da_I = 10;     // Steps per walk
    int da_roomSizeX = 5;
    int da_roomSizeY = 3;
    double da_probGenerateRoom = 0.1;
    double da_probIncreaseRoom = 0.05;
    double da_probChangeDirection = 0.2;
    double da_probIncreaseChange = 0.03;


    // --- Main Simulation Loop ---
    for (int iteration = 0; iteration < numIterations; ++iteration) {
        std::cout << "\n--- Iteration " << iteration + 1 << " ---" << std::endl;

        // TODO: IMPLEMENTATION GOES HERE: Call the Cellular Automata and/or Drunk Agent functions.
        // The order of calls will depend on how you want them to interact.

        // Example: First the cellular automata, then the agent
        myMap = cellularAutomata(myMap, ca_W, ca_H, ca_R, ca_U);
        myMap = drunkAgent(myMap, da_W, da_H, da_J, da_I, da_roomSizeX, da_roomSizeY,
                           da_probGenerateRoom, da_probIncreaseRoom,
                           da_probChangeDirection, da_probIncreaseChange,
                           drunkAgentX, drunkAgentY);

        printMap(myMap);

        // You can add a delay to visualize the simulation step by step
        // #include <thread> // For std::this_thread::sleep_for
        // #include <chrono> // For std::chrono::milliseconds
        // std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    std::cout << "\n--- Simulation Finished ---" << std::endl;
    return 0;
}