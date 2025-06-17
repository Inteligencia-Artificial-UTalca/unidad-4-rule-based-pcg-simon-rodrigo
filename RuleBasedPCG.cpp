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

void initializeRandomMap(Map& map, int W, int H, double aliveProbability = 0.45) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dist(0.0, 1.0);

    for (int y = 0; y < H; ++y) {
        for (int x = 0; x < W; ++x) {
            map[y][x] = (dist(gen) < aliveProbability) ? 1 : 0;
        }
    }
}


Map cellularAutomata(const Map& currentMap, int W, int H, int R, double U) {
    Map newMap = currentMap; // Initially, the new map is a copy of the current one

    // TODO: IMPLEMENTATION GOES HERE for the cellular automata logic.
    // Iterate over each cell and apply the transition rules.
    // Remember that updates should be based on the 'currentMap' state
    // and applied to the 'newMap' to avoid race conditions within the same iteration.

    for (int y = 0; y < H; ++y) {
        for (int x = 0; x < W; ++x) {
            int onesCount = 0;
            int totalCells = 0;

            // Revisar vecindario en un cuadrado de radio R
            for (int dy = -R; dy <= R; ++dy) {
                for (int dx = -R; dx <= R; ++dx) {
                    int nx = x + dx;
                    int ny = y + dy;

                    // Ignorar la propia celda
                    if (dx == 0 && dy == 0) continue;

                    // Puedes elegir si considerar celdas fuera del mapa como 0, 1 o aleatorio
                    if (nx >= 0 && nx < W && ny >= 0 && ny < H) {
                        if (currentMap[ny][nx] == 1)
                            onesCount++;
                    } else {
                        // Considerar celdas fuera del borde como 1
                        onesCount++;
                    }

                    totalCells++;
                }
            }

            // Calcular proporción de 1s
            double density = static_cast<double>(onesCount) / totalCells;
            newMap[y][x] = (density >= U) ? 1 : 0;
        }
    }

    return newMap;
}

void changeDirection(int& x, int& y, int& dir, int w, int h)
{
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_int_distribution<int> dist(0, 3);
    bool moved = false;

    for (int attempts = 0; attempts < 10 && !moved; ++attempts) {
        switch (dir) {
        case 0: // abajo
            if (y + 1 < h) { y++; moved = true; break; }
            break;
        case 1: // arriba
            if (y - 1 >= 0) { y--; moved = true; break; }
            break;
        case 2: // derecha
            if (x + 1 < w) { x++; moved = true; break; }
            break;
        case 3: // izquierda
            if (x - 1 >= 0) { x--; moved = true; break; }
            break;
        }
        if (!moved)
            dir = dist(mt); // elige una nueva dirección si no puede moverse
    }
}

void generateRoom(Map& map, int x, int y, int roomW, int roomH) {
    int startX = std::max(0, x - roomW / 2);
    int startY = std::max(0, y - roomH / 2);
    int endX = std::min((int)map[0].size() - 1, x + roomW / 2); // columnas
    int endY = std::min((int)map.size() - 1, y + roomH / 2);    // filas

    for (int i = startY; i <= endY; ++i) {
        for (int j = startX; j <= endX; ++j) {
            map[i][j] = 1;
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
    std::uniform_real_distribution<float> distFloat(0.0, 1.0);
    std::uniform_int_distribution<int> dirGen(0, 3);
    int currentDir = dirGen(gen);//direccion inicial aleatoria

    for (int i = 0; i < J; ++i) {
        for (int j = 0; j < I; ++j) {
            float changeProb = distFloat(gen);

            if (j == 0 || changeProb <= probChangeDirection) {
                currentDir = dirGen(gen);//nueva direccion aleatoria
                changeDirection(agentX, agentY, currentDir, W, H);
                probChangeDirection = 0.2;//reiniciar
            } else {
                changeDirection(agentX, agentY, currentDir, W, H);
                probChangeDirection += probIncreaseChange;
            }

            //Marca el paso del agente
            if (agentY >= 0 && agentY < H && agentX >= 0 && agentX < W) {
                newMap[agentY][agentX] = 1;
            }

            float roomProb = distFloat(gen);
            if (roomProb <= probGenerateRoom) {
                generateRoom(newMap, agentX, agentY, roomSizeX, roomSizeY);
                probGenerateRoom = 0.1;
            } else {
                probGenerateRoom += probIncreaseRoom;
            }
        }
    }

    return newMap;
}

int main() {
    std::cout << "--- CELLULAR AUTOMATA AND DRUNK AGENT SIMULATION ---" << std::endl;

    // --- Initial Map Configuration ---
    int mapRows = 20;
    int mapCols = 30;
    Map myMap(mapRows, std::vector<int>(mapCols, 0)); // Map initialized with zeros

    Map DrunkMap = myMap;//este mapa sera exclusivo para drunk agent

    // TODO: IMPLEMENTATION GOES HERE: Initialize the map with some pattern or initial state.
    // For example, you might set some cells to 1 for the cellular automata
    // or place the drunk agent at a specific position.

    // Drunk Agent's initial position
    int drunkAgentX = mapRows / 2;
    int drunkAgentY = mapCols / 2;
    // If your agent modifies the map at start, you could do it here:
    // myMap[drunkAgentX][drunkAgentY] = 2; // Assuming '2' represents the agent

    
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
    int da_roomSizeX = 4;
    int da_roomSizeY = 3;
    double da_probGenerateRoom = 0.1;
    double da_probIncreaseRoom = 0.05;
    double da_probChangeDirection = 0.2;
    double da_probIncreaseChange = 0.03;


    initializeRandomMap(myMap, mapCols, mapRows, 0.45);//inicializar el mapa con ruidos aleatorios
    std::cout << "\nInitial map state (con ruido aleatorio):" << std::endl;
    printMap(myMap);

    // --- Main Simulation Loop ---
    for (int iteration = 0; iteration < numIterations; ++iteration) {
        std::cout << "\n--- Cellurar automata Iteration " << iteration + 1 << " ---" << std::endl;

        // TODO: IMPLEMENTATION GOES HERE: Call the Cellular Automata and/or Drunk Agent functions.
        // The order of calls will depend on how you want them to interact.

        // Example: First the cellular automata, then the agent
        myMap = cellularAutomata(myMap, ca_W, ca_H, ca_R, ca_U);
        /*myMap = drunkAgent(myMap, da_W, da_H, da_J, da_I, da_roomSizeX, da_roomSizeY,
                           da_probGenerateRoom, da_probIncreaseRoom,
                           da_probChangeDirection, da_probIncreaseChange,
                           drunkAgentX, drunkAgentY);*/

        printMap(myMap);

        // You can add a delay to visualize the simulation step by step
        // #include <thread> // For std::this_thread::sleep_for
        // #include <chrono> // For std::chrono::milliseconds
        // std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    std::cout << "\n--- Simulation Finished ---" << std::endl;

    std::cout << "\nInitial map state:" << std::endl;
    printMap(DrunkMap);
    // --- Main Simulation Loop ---
    for (int iteration = 0; iteration < numIterations; ++iteration) {
        std::cout << "\n--- Drunk agent Iteration " << iteration + 1 << " ---" << std::endl;

        // TODO: IMPLEMENTATION GOES HERE: Call the Cellular Automata and/or Drunk Agent functions.
        // The order of calls will depend on how you want them to interact.

        // Example: First the cellular automata, then the agent
        //myMap = cellularAutomata(myMap, ca_W, ca_H, ca_R, ca_U);
        DrunkMap = drunkAgent(DrunkMap, da_W, da_H, da_J, da_I, da_roomSizeX, da_roomSizeY,
                           da_probGenerateRoom, da_probIncreaseRoom,
                           da_probChangeDirection, da_probIncreaseChange,
                           drunkAgentX, drunkAgentY);

        printMap(DrunkMap);

        // You can add a delay to visualize the simulation step by step
        // #include <thread> // For std::this_thread::sleep_for
        // #include <chrono> // For std::chrono::milliseconds
        // std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    std::cout << "\n--- Simulation Finished ---" << std::endl;

    return 0;
}