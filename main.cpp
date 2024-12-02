#include <iostream>
#include <bitset>
#include <chrono>
#include <thread>
#include <cstdlib>
#include <cstring>

/**
 * Cellular Automaton
 *
 * Goals
 *  - Implement a simple cellular automaton simulation using the Game of Life rules.
 *  - Use a DynamicBitset class to manage the grid state.
 *  - Display the grid state in the console.
 *  - Measure the time taken for each iteration and the total time for the simulation.
 *  - Allow customization of the board dimensions and speed of the simulation.
 *  - Allow to run concurrent simulations with different parameters (like percentage of cells alive).
 *  - Search for stable or oscillating patterns in the grid.
 * */

// A DynamicBitset class that allows for dynamic allocation of bits on the heap and provides a safe interface for reading and writing bit values. 
// Likely to be slower than std::vector<bool>... but it's a good exercise in memory management and operator overloading.
// Potentially more memory efficient than std::vector<bool> since std::bitset<1> is a single bit in size.
class DynamicBitset {
public:
    DynamicBitset(int size) : size(size), data(new std::bitset<1>[size]()) {}

    DynamicBitset(const DynamicBitset& other) : size(other.size), data(new std::bitset<1>[other.size]) {
        for (int i = 0; i < size; ++i) {
            data[i] = other.data[i];
        }
    }

    DynamicBitset& operator=(const DynamicBitset& other) {
        if (this == &other) return *this; // Handle self-assignment

        delete[] data; // Free existing memory
        size = other.size;
        data = new std::bitset<1>[size];
        for (int i = 0; i < size; ++i) {
            data[i] = other.data[i];
        }
        return *this;
    }

    ~DynamicBitset() {
        delete[] data; // Properly delete allocated memory
    }

    bool test(int index) const {
        // Safely read the value of a specific bit
        if (index >= 0 && index < size) {
            return data[index].test(0);
        }
        return false;
    }

    void set(int index, bool value) {
        // Safely set the value of a specific bit
        if (index >= 0 && index < size) {
            data[index] = value ? std::bitset<1>("1") : std::bitset<1>("0");
        }
    }

    void reset() {
        // Reset all bits to 0
        for (int i = 0; i < size; ++i) {
            data[i].reset();
        }
    }

    bool operator==(const DynamicBitset& other) const {
        // Compare two DynamicBitset objects
        if (size != other.size) return false;
        for (int i = 0; i < size; ++i) {
            if (data[i] != other.data[i]) return false;
        }
        return true;
    }

private:
    int size;
    std::bitset<1>* data;
};


class CellularAutomaton {
public:
    CellularAutomaton(int width, int height, int speed)
        : width(width), height(height), speed(speed),
          grid(width * height), nextGrid(width * height), prevGrid(width * height)
    {
        initializeRandom();
    }

    void run(bool displayEnabled) {
        std::cout << "\033[2J\033[1;1H"; // Clear screen

        auto startTotal = std::chrono::high_resolution_clock::now();
        int iteration = 0;

        while (true) {
            std::cout << "\033[H"; // Move cursor to the top-left

            auto startIter = std::chrono::high_resolution_clock::now();
            bool isAlive = update();
            auto endIter = std::chrono::high_resolution_clock::now();

            if (displayEnabled) {
                display();
                std::this_thread::sleep_for(std::chrono::milliseconds(speed));
            }

            if (!isAlive) {
                std::cout << "Board has reached a stable or alternating state.\n";
                break;
            }

            auto iterDuration = std::chrono::duration_cast<std::chrono::microseconds>(endIter - startIter);
            std::cout << "Iteration " << iteration + 1 << ": " << iterDuration.count() << " microseconds\n";

            iteration++;
            std::cout << std::flush;
        }

        auto endTotal = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = endTotal - startTotal;

        std::cout << "Total time for " << iteration << " iterations: " << elapsed.count() << " seconds\n";
    }

private:
    int width, height, speed;
    DynamicBitset grid;
    DynamicBitset nextGrid;
    DynamicBitset prevGrid;

    void initializeRandom() {
        srand(time(0)); // Seed random number generator
        for (int i = 0; i < width * height; ++i) {
            grid.set(i, rand() % 2); // Use set method for setting random values
        }
    }

    // TODO: Detect oscillating patterns of periods greater than one. 
    //       Since I am using the DnyamicBitset class, I can create many copies of the grid state without worrying about memory overhead.
    bool update() {
        nextGrid.reset(); // Reset the next grid to all 0s

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                int index = y * width + x;
                int liveNeighbors = countLiveNeighbors(x, y);

                bool alive = grid.test(index); // Use test to read a cell value
                nextGrid.set(index, (alive && (liveNeighbors == 2 || liveNeighbors == 3)) ||
                                     (!alive && liveNeighbors == 3)); // Use set to write a cell value
            }
        }

        if (nextGrid == prevGrid || nextGrid == grid) {
            return false; // Stable or alternating state detected
        }

        prevGrid = grid; // Save the current state to prevGrid
        grid = nextGrid; // Update the current grid to nextGrid

        return true; // Continue simulation
    }

    int countLiveNeighbors(int x, int y) const {
        int count = 0;

        for (int dy = -1; dy <= 1; ++dy) {
            for (int dx = -1; dx <= 1; ++dx) {
                if (dx == 0 && dy == 0) // Skip the current cell
                    continue;

                int nx = x + dx;
                int ny = y + dy;

                // Ensure neighbors are within bounds
                if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
                    count += grid.test(ny * width + nx); // Use test to read neighbor's value
                }
            }
        }

        return count;
    }

    void display() const {
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                std::cout << (grid.test(y * width + x) ? "\033[38;5;82m◆\033[0m" : " ");
            }
            std::cout << '\n';
        }
    }
};

int main(int argc, char *argv[]) {
    int width = 32, height = 32;
    int speed = 100;
    bool displayEnabled = true;

    // Parse command-line arguments
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "-w") == 0 && i + 1 < argc) {
            width = std::atoi(argv[++i]);
        } else if (std::strcmp(argv[i], "-h") == 0 && i + 1 < argc) {
            height = std::atoi(argv[++i]);
        } else if (std::strcmp(argv[i], "-s") == 0 && i + 1 < argc) {
            speed = std::atoi(argv[++i]);
        } else if (std::strcmp(argv[i], "-nd") == 0) {
            displayEnabled = false;
        }
    }

    if (width <= 0 || height <= 0) {
        std::cerr << "Invalid board dimensions. Width and height must be positive integers.\n";
        return 1;
    }

    CellularAutomaton ca(width, height, speed);
    ca.run(displayEnabled);

    return 0;
}
