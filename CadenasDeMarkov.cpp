#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include <random>
#include <fstream>
#include <sstream>
#include <ctime>
#include <filesystem>
#include <algorithm>

//para conveniencia
using Slice = std::string;


enum class SliceMode {
    Cube,
    Ship,
    PortalToCube,
    PortalToShip,
    TransitionPortal,
    Unknown
};


enum class Difficulty {
    Easy,
    Medium,
    Hard
};






std::vector<Slice> loadSlicesFromFolder(const std::string& folderPath)//cargar ejemplos de niveles
{
    std::vector<Slice> allSlices;
    for (const auto& entry : std::filesystem::directory_iterator(folderPath)) {
        if (entry.path().extension() == ".txt") {
            std::ifstream file(entry.path());
            std::stringstream buffer;
            buffer << file.rdbuf();
            std::string fileContent = buffer.str();
            if (!fileContent.empty()) {
                if (fileContent.back() == '\n') {
                    fileContent.pop_back();
                }
                allSlices.push_back(fileContent);
            }
            std::cout << "Cargado: " << entry.path().filename() << "\n";
        }
    }
    return allSlices;
}

void printLevelHorizontal(const std::vector<Slice>& level)//no funciono como pense
 {
    if (level.empty()) return;
    size_t maxHeight = 0;
    std::vector<std::vector<std::string>> linesOfSlices(level.size());

    for (size_t i = 0; i < level.size(); ++i) {
        std::stringstream ss(level[i]);
        std::string line;
        while (std::getline(ss, line, '\n')) {
            linesOfSlices[i].push_back(line);
        }
        size_t currentHeight = linesOfSlices[i].size();
        if (currentHeight == 0 && !level[i].empty()) currentHeight = 1;
        maxHeight = std::max(maxHeight, currentHeight);
    }

    for (size_t row = 0; row < maxHeight; ++row) {
        for (const auto& sliceLines : linesOfSlices) {
            if (row < sliceLines.size()) {
                std::cout << sliceLines[row];
            } else {
                size_t sliceWidth = 0;
                if (!sliceLines.empty()) {
                    sliceWidth = sliceLines[0].length();
                }
                for (size_t k = 0; k < sliceWidth; ++k) {
                    std::cout << ' ';
                }
            }
        }
        std::cout << '\n';
    }
}

int chooseNextIndex(const std::vector<double>& probabilities)
{
    double r = (double)std::rand() / RAND_MAX;
    double cumulative = 0.0;
    for (int j = 0; j < (int)probabilities.size(); ++j) {
        cumulative += probabilities[j];
        if (r <= cumulative)
            return j;
    }
    return (int)probabilities.size() - 1;
}

void printLevel(const std::vector<Slice>& level) {
    for (const auto& slice : level)
        std::cout << slice << std::endl;
}

//implementacion de las funciones de modo y deseabilidad

SliceMode getSliceMode(const Slice& s) {
    bool hasCubePortal = s.find("#### c ####") != std::string::npos;
    bool hasShipPortal = s.find("#### s ####") != std::string::npos;

    if (hasCubePortal && hasShipPortal) return SliceMode::TransitionPortal;
    if (hasCubePortal) return SliceMode::PortalToCube;
    if (hasShipPortal) return SliceMode::PortalToShip;

    if (s.find('|') != std::string::npos || s.find('#') != std::string::npos) {
        return SliceMode::Cube;
    }
    return SliceMode::Unknown;
}

//funciones auxiliares para verificar contenido de slices
bool getSliceContainsSpikes(const Slice& s) {
    return s.find('*') != std::string::npos;
}

bool getSliceContainsGround(const Slice& s) {
    return s.find('|') != std::string::npos;
}


double getBaseSliceDesirability(const Slice& s, Difficulty difficulty)//considerar dificultad para la deseabilidad de los slices
{
    double desirability = 0.0;
    bool hasSpike = false;
    bool hasBlock = false;
    bool hasGround = false;
    bool hasNonSpaceChar = false;

    for (char c : s) {
        if (c == '|') {
            desirability += 2.0;
            hasGround = true;
            hasNonSpaceChar = true;
        } else if (c == '#') {
            desirability += 1.5;
            hasBlock = true;
            hasNonSpaceChar = true;
        } else if (c == '*') {
            desirability -= 5.0; //penalizacion base por pinchos
            hasSpike = true;
            hasNonSpaceChar = true;
        }
    }

    // Ajustes de dificultad para pinchos
    if (hasSpike) {
        if (difficulty == Difficulty::Easy) {
            desirability -= 7.0; //mayor penalizacion en Fácil
        } else if (difficulty == Difficulty::Medium) {
            desirability -= 3.0; //penalizacion moderada
        } else if (difficulty == Difficulty::Hard) {
            desirability += 45.0; //pinchos deseables en Difícil
        }
    }
    
    SliceMode mode = getSliceMode(s);

    if (mode == SliceMode::PortalToCube || mode == SliceMode::PortalToShip) {
        desirability += 7.0;
    } else if (mode == SliceMode::TransitionPortal) {
        desirability += 8.0;
    }
    
    if (!hasNonSpaceChar && s.length() > 0) {
        desirability += 0.5;
    }

    return std::max(desirability, -10.0);
}

//funcion generateLevelWithMatrix considera la matriz de adyacencia para probabilidades, longitud del nivel y dificultad

std::vector<Slice> generateLevelWithMatrix(const std::vector<Slice>& indexToSlice,
                                           const std::vector<std::vector<double>>& adjacencyMatrix,
                                           int length, Difficulty difficulty) { 
    std::vector<Slice> level;
    int N = (int)indexToSlice.size();
    std::srand(std::time(nullptr));

    std::vector<SliceMode> currentSliceModes(N);
    for(int i = 0; i < N; ++i) {
        currentSliceModes[i] = getSliceMode(indexToSlice[i]);
    }

    //seleccion del primer slice 
    int current = -1;
    std::vector<int> possibleStarts;
    for(int i = 0; i < N; ++i) {
        if (currentSliceModes[i] == SliceMode::Cube || currentSliceModes[i] == SliceMode::Unknown || currentSliceModes[i] == SliceMode::PortalToCube) {
            if (currentSliceModes[i] == SliceMode::PortalToCube && indexToSlice[i].find("#### s ####") != std::string::npos) {
                
            } else {
                possibleStarts.push_back(i);
            }
        }
    }
    if (!possibleStarts.empty()) {
        current = possibleStarts[std::rand() % possibleStarts.size()];
    } else {
        current = std::rand() % N;
    }
    level.push_back(indexToSlice[current]);

    //generar slices siguientes
    const int CUBE_WARMUP_SLICES = 3; 

    for (int i = 1; i < length; ++i) {
        std::vector<double> currentProbabilities = adjacencyMatrix[current];
        
        // Aplicar filtro para el periodo de calentamiento
        if (i < CUBE_WARMUP_SLICES) {
            std::vector<double> filteredProbabilities(N, 0.0);
            double filterSum = 0.0;
            for (int j = 0; j < N; ++j) {
                SliceMode nextMode = currentSliceModes[j];
                if (nextMode == SliceMode::Cube || nextMode == SliceMode::PortalToCube || nextMode == SliceMode::Unknown) {
                    if (nextMode == SliceMode::TransitionPortal) {
                        filteredProbabilities[j] = currentProbabilities[j] * 0.001;
                    } else {
                        filteredProbabilities[j] = currentProbabilities[j];
                    }
                } else {
                    filteredProbabilities[j] = currentProbabilities[j] * 0.001;
                }
                filterSum += filteredProbabilities[j];
            }

            if (filterSum > 0) {
                for (int j = 0; j < N; ++j) {
                    filteredProbabilities[j] /= filterSum;
                }
                current = chooseNextIndex(filteredProbabilities);
            } else {
                std::vector<int> fallbackIndices;
                for(int k = 0; k < N; ++k) {
                    if (currentSliceModes[k] == SliceMode::Cube || currentSliceModes[k] == SliceMode::Unknown) {
                        fallbackIndices.push_back(k);
                    }
                }
                if (!fallbackIndices.empty()) {
                    current = fallbackIndices[std::rand() % fallbackIndices.size()];
                } else {
                    current = chooseNextIndex(adjacencyMatrix[current]);
                }
            }
        } else {
            //aplicar ajustes de dificultad
            for (int j = 0; j < N; ++j) {
                const Slice& nextSlice = indexToSlice[j];
                bool nextSliceHasSpikes = getSliceContainsSpikes(nextSlice);
                
                if (difficulty == Difficulty::Easy) {
                    if (nextSliceHasSpikes) {
                        currentProbabilities[j] *= 0.1;
                    }
                } else if (difficulty == Difficulty::Hard) {
                    //penalizar la slice 16 (nivel16.txt) en dificultad difícil (muy facil esa)
                    if (nextSlice == "#### c ####\n|         # \n|       #\n|      #\n|       #\n|         #\n#          |\n#         |\n  #        |\n#         |\n#          |\n|         # \n|       #\n|      #\n|       #\n|\n#\n#         |\n #        |\n#       |\n #        |\n#         |\n#### s ####") { // nivel16.txt 
                        currentProbabilities[j] *= 0.2; 
                    }

                    //aumentar aun más el multiplicador especifico para nivel20.txt
                    if (nextSlice == "* *\n * *\n  * *\n  * *\n   * *\n   * *\n#### s ####") { //nivel20.txt 
                        currentProbabilities[j] *= 10.0; 
                    } else if (nextSlice == "##### \n       *\n       *\n       |\n" || nextSlice == "*|\n       | \n       *\n       *\n       |\n       |") { //nivel21.txt 
                        currentProbabilities[j] *= 5.0; 
                    } else if (nextSlice == "|  \n####       | \n# ###* *\n#######* * \n# ###* *\n###        |\n#### c ####") { //nivel22.txt 
                        currentProbabilities[j] *= 5.0; 
                    }
                    
                    if (nextSliceHasSpikes) {
                        currentProbabilities[j] *= 2.0; //sigue doblando si tiene pinchos
                    }
                }
            }

            //normalizar despues de aplicar ajustes por dificultad
            double sum = 0.0;
            for(double p : currentProbabilities) sum += p;
            if (sum > 0) {
                for(int j = 0; j < N; ++j) currentProbabilities[j] /= sum;
            } else {
                for(int k = 0; k < N; ++k) currentProbabilities[k] = 1.0 / N;
            }

            current = chooseNextIndex(currentProbabilities);
        }
        
        level.push_back(indexToSlice[current]);
    }
    
    return level;
}



Difficulty promptUserForDifficulty() {
    int choice;
    std::cout << "Selecciona la dificultad del nivel:\n";
    std::cout << "1. Facil\n";
    std::cout << "2. Medio\n";
    std::cout << "3. Dificil\n";
    std::cout << "Opcion: ";
    std::cin >> choice;

    switch (choice) {
        case 1: return Difficulty::Easy;
        case 2: return Difficulty::Medium;
        case 3: return Difficulty::Hard;
        default:
            std::cout << "Entrada invalida, se usara dificultad MEDIO.\n";
            return Difficulty::Medium;
    }
}

int main() {
    auto slices = loadSlicesFromFolder("niveles");

    if (slices.size() < 2) {
        std::cerr << "Error: se necesitan al menos 2 slices en total.\n";
        return 1;
    }

    std::unordered_map<Slice, int> sliceToIndex;
    std::vector<Slice> indexToSlice;
    int idx = 0;
    for (const auto& s : slices) {
        if (sliceToIndex.find(s) == sliceToIndex.end()) {
            sliceToIndex[s] = idx++;
            indexToSlice.push_back(s);
        }
    }

    int N = (int)indexToSlice.size();
    std::vector<std::vector<double>> adjacencyMatrix(N, std::vector<double>(N, 0.0));
    std::vector<std::vector<double>> transitionCounts(N, std::vector<double>(N, 0.0));

    //contar transiciones
    for (size_t i = 1; i < slices.size(); ++i) {
        int prevIdx = sliceToIndex[slices[i - 1]];
        int currIdx = sliceToIndex[slices[i]];
        transitionCounts[prevIdx][currIdx] += 1.0;
    }

    std::vector<SliceMode> sliceModes(N);
    for (int i = 0; i < N; ++i) {
        sliceModes[i] = getSliceMode(indexToSlice[i]);
    }

    
    Difficulty selectedDifficulty = promptUserForDifficulty();
    std::cout << "Generando nivel en dificultad: ";
    if (selectedDifficulty == Difficulty::Easy) std::cout << "FACIL\n";
    else if (selectedDifficulty == Difficulty::Medium) std::cout << "MEDIO\n";
    else std::cout << "DIFICIL\n";

    //aplicamos la ponderacion de jugabilidad y modo a los conteos de transicion
    for (int i = 0; i < N; ++i) { //indice del slice anterior
        SliceMode prevMode = sliceModes[i];

        for (int j = 0; j < N; ++j) { //indice del slice actual (transicionando a)
            SliceMode currMode = sliceModes[j];
            const Slice& currSlice = indexToSlice[j];//referencia al slice actual

            double weightedCount = transitionCounts[i][j];
        
            weightedCount += getBaseSliceDesirability(currSlice, selectedDifficulty);

            //reglas de transicion especificas de modo
            if (prevMode == SliceMode::Cube && currMode == SliceMode::Ship) {
                if (currMode != SliceMode::PortalToShip && currMode != SliceMode::TransitionPortal) {
                    weightedCount *= 0.01;
                }
            } else if (prevMode == SliceMode::Ship && currMode == SliceMode::Cube) {
                if (currMode != SliceMode::PortalToCube && currMode != SliceMode::TransitionPortal) {
                    weightedCount *= 0.01;
                }
            }
            else if (prevMode == SliceMode::Cube && currMode == SliceMode::PortalToShip) {
                weightedCount *= 5.0;
            } else if (prevMode == SliceMode::Ship && currMode == SliceMode::PortalToCube) {
                weightedCount *= 5.0;
            } else if (prevMode == SliceMode::Cube && currMode == SliceMode::TransitionPortal) {
                weightedCount *= 4.0;
            } else if (prevMode == SliceMode::Ship && currMode == SliceMode::TransitionPortal) {
                weightedCount *= 4.0;
            }
            else if (prevMode == SliceMode::Cube && currMode == SliceMode::Cube) {
                weightedCount *= 1.2;
            } else if (prevMode == SliceMode::Ship && currMode == SliceMode::Ship) {
                weightedCount *= 1.2;
            }
            else if (prevMode == SliceMode::TransitionPortal && (currMode == SliceMode::Cube || currMode == SliceMode::PortalToCube)) {
                weightedCount *= 1.5;
            } else if (prevMode == SliceMode::TransitionPortal && (currMode == SliceMode::Ship || currMode == SliceMode::PortalToShip)) {
                weightedCount *= 1.5;
            }

            if (prevMode == SliceMode::Unknown || currMode == SliceMode::Unknown) {
                weightedCount *= 0.5;
            }

            adjacencyMatrix[i][j] = std::max(weightedCount, 0.001);
        }
    }

    //mormalizar filas para que sumen 1
    for (int i = 0; i < N; ++i) {
        double rowSum = 0;
        for (int j = 0; j < N; ++j)
            rowSum += adjacencyMatrix[i][j];
        if (rowSum > 0) {
            for (int j = 0; j < N; ++j)
                adjacencyMatrix[i][j] /= rowSum;
        } else {
            for (int j = 0; j < N; ++j)
                adjacencyMatrix[i][j] = 1.0 / N;
        }
    }

    //generar el nivel pasando la dificultad
    auto level = generateLevelWithMatrix(indexToSlice, adjacencyMatrix, 20, selectedDifficulty);
    printLevel(level);
    //printLevelHorizontal(level);
    return 0;
}
