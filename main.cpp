#include <iostream>
#include <bitset>
#include <chrono>
#include <thread>
#include <vector>
#include <cstring>
#include <cstdlib>

template <int Width, int Height>
class CellularAutomaton
{
public:
    static constexpr int GridSize = Width * Height;

    CellularAutomaton(int s)
        : speed(s)
    {
        initializeRandom();
    }

    void run(bool displayEnabled)
    {
        std::cout << "\033[2J\033[1;1H";

        auto startTotal = std::chrono::high_resolution_clock::now();
        int iteration = 0;

        while (true)
        {
            std::cout << "\033[H";

            auto startIter = std::chrono::high_resolution_clock::now();
            bool isAlive = update();
            auto endIter = std::chrono::high_resolution_clock::now();

            if (displayEnabled)
            {
                display();
                std::this_thread::sleep_for(std::chrono::milliseconds(speed));
            }

            if (isAlive == false)
            {
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
    int speed;
    std::bitset<GridSize> grid;
    std::bitset<GridSize> nextGrid;
    std::bitset<GridSize> prevGrid;

    void initializeRandom()
    {
        srand(time(0));
        for (int i = 0; i < GridSize; ++i)
        {
            grid[i] = rand() % 2;
        }
    }

    bool update()
    {
        nextGrid.reset(); 

        for (int y = 0; y < Height; ++y)
        {
            for (int x = 0; x < Width; ++x)
            {
                int index = y * Width + x;
                int liveNeighbors = countLiveNeighbors(x, y);

                bool alive = grid[index];
                nextGrid[index] = (alive && (liveNeighbors == 2 || liveNeighbors == 3)) ||
                                    (!alive && liveNeighbors == 3);
            }
        }

        if((nextGrid == prevGrid) || (nextGrid == grid))
        {
            return false;
        }

        prevGrid = grid; 
        grid = nextGrid; 

        return true;

    }

    int countLiveNeighbors(int x, int y) const
    {
        int count = 0;

        for (int dy = -1; dy <= 1; ++dy)
        {
            for (int dx = -1; dx <= 1; ++dx)
            {
                if (dx == 0 && dy == 0)
                    continue;

                int nx = x + dx;
                int ny = y + dy;

                if (nx >= 0 && nx < Width && ny >= 0 && ny < Height)
                {
                    count += grid[ny * Width + nx];
                }
            }
        }

        return count;
    }

    void display() const
    {
        for (int y = 0; y < Height; ++y)
        {
            for (int x = 0; x < Width; ++x)
            {
                std::cout << (grid[y * Width + x] ? "\033[38;5;82m◆\033[0m" : " ");
            }
            std::cout << '\n';
        }
    }
};

int main(int argc, char *argv[])
{
    bool displayEnabled = true;
    int speed = 100;

    
    for (int i = 1; i < argc; ++i)
    {
        if (std::strcmp(argv[i], "-s") == 0 && i + 1 < argc)
        {
            speed = std::atoi(argv[++i]);
        }
        else if (std::strcmp(argv[i], "-nd") == 0)
        {
            displayEnabled = false;
        }
    }

    CellularAutomaton<32, 32> ca(speed);
    ca.run(displayEnabled);

    return 0;
}
