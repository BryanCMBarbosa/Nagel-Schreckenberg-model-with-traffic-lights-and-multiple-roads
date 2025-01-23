#include "TrafficLightGroup.h"

TrafficLightGroup::TrafficLightGroup()
    : currentIndex(0), inGreenPhase(true), inTransitionPhase(false), groupTimer(0), transitionTime(0)
{
}

void TrafficLightGroup::setTransitionTime(int time)
{
    transitionTime = time;
}

void TrafficLightGroup::setCoords(int c, int r)
{
    column = c;
    row = r;
}

void TrafficLightGroup::setGridShape(int h, int w)
{
    gridHeight = h;
    gridWidth = w;
}

void TrafficLightGroup::calculateCentralities()
{
    calculateDegreeCentrality();
    calculateClosenessCentrality();
    calculateBetweennessCentrality();
}

void TrafficLightGroup::calculateDegreeCentrality()
{
    int count = 0;

    if (row > 0) count++;
    if (row < gridHeight - 1) count++;
    if (column > 0) count++;
    if (column < gridWidth - 1) count++;

    degreeCentrality = count;
}

void TrafficLightGroup::calculateClosenessCentrality()
{
    int totalNodes = gridHeight * gridWidth;
    std::vector<std::vector<int>> distances(gridHeight, std::vector<int>(gridWidth, std::numeric_limits<int>::max()));
    std::queue<std::pair<int, int>> q;

    distances[row][column] = 0;
    q.push({row, column});

    while (!q.empty())
    {
        auto [r, c] = q.front();
        q.pop();

        for (auto [dr, dc] : std::vector<std::pair<int, int>>{{-1, 0}, {1, 0}, {0, -1}, {0, 1}})
        {
            int nr = r + dr, nc = c + dc;
            if (nr >= 0 && nr < gridHeight && nc >= 0 && nc < gridWidth && distances[nr][nc] == std::numeric_limits<int>::max())
            {
                distances[nr][nc] = distances[r][c] + 1;
                q.push({nr, nc});
            }
        }
    }

    double totalDistance = 0.0;
    for (int r = 0; r < gridHeight; r++)
    {
        for (int c = 0; c < gridWidth; c++)
        {
            if (distances[r][c] != std::numeric_limits<int>::max())
            {
                totalDistance += distances[r][c];
            }
        }
    }

    closenessCentrality = (totalDistance > 0) ? (1.0 / totalDistance) * (totalNodes - 1) : 0.0;
}

void TrafficLightGroup::calculateBetweennessCentrality()
{
    betweennessCentrality = 0.0;
    int totalNodes = gridHeight * gridWidth;

    for (int startRow = 0; startRow < gridHeight; ++startRow)
    {
        for (int startCol = 0; startCol < gridWidth; ++startCol)
        {
            if (startRow == row && startCol == column) continue; // Skip the current node

            std::vector<std::vector<int>> distances(gridHeight, std::vector<int>(gridWidth, std::numeric_limits<int>::max()));
            std::vector<std::vector<int>> paths(gridHeight, std::vector<int>(gridWidth, 0));
            std::queue<std::pair<int, int>> q;

            distances[startRow][startCol] = 0;
            paths[startRow][startCol] = 1; // One path from source to itself
            q.push({startRow, startCol});

            // BFS for shortest paths
            while (!q.empty())
            {
                auto [currentRow, currentCol] = q.front();
                q.pop();

                for (auto [dr, dc] : std::vector<std::pair<int, int>>{{-1, 0}, {1, 0}, {0, -1}, {0, 1}})
                {
                    int nextRow = currentRow + dr, nextCol = currentCol + dc;
                    if (nextRow >= 0 && nextRow < gridHeight && nextCol >= 0 && nextCol < gridWidth)
                    {
                        // Found a shorter path
                        if (distances[nextRow][nextCol] > distances[currentRow][currentCol] + 1)
                        {
                            distances[nextRow][nextCol] = distances[currentRow][currentCol] + 1;
                            paths[nextRow][nextCol] = paths[currentRow][currentCol];
                            q.push({nextRow, nextCol});
                        }
                        // Found an additional shortest path
                        else if (distances[nextRow][nextCol] == distances[currentRow][currentCol] + 1)
                        {
                            paths[nextRow][nextCol] += paths[currentRow][currentCol];
                        }
                    }
                }
            }

            // Count how many shortest paths pass through the current node
            for (int endRow = 0; endRow < gridHeight; ++endRow)
            {
                for (int endCol = 0; endCol < gridWidth; ++endCol)
                {
                    if (endRow == startRow && endCol == startCol) continue;
                    if (endRow == row && endCol == column) continue;

                    if (distances[endRow][endCol] != std::numeric_limits<int>::max() &&
                        distances[row][column] < distances[endRow][endCol])
                    {
                        betweennessCentrality += static_cast<double>(paths[row][column]) / paths[endRow][endCol];
                    }
                }
            }
        }
    }

    // Normalize betweenness centrality by the total number of possible pairs
    betweennessCentrality /= ((totalNodes - 1) * (totalNodes - 2));
}

void TrafficLightGroup::addTrafficLight(std::shared_ptr<TrafficLight> trafficLight)
{
    trafficLights.push_back(trafficLight);
    trafficLight->setGroup(shared_from_this());
}

void TrafficLightGroup::initialize()
{
    if (!trafficLights.empty())
    {
        currentIndex = 0;
        inGreenPhase = true;
        inTransitionPhase = false;
        groupTimer = 0;
        calculateTotalCycleTime();

        for (auto& tl : trafficLights)
            tl->state = false;

        trafficLights[currentIndex]->state = true;
    }
}

void TrafficLightGroup::calculateTotalCycleTime()
{
    totalCycleTime = 0;
    for (const auto& tl : trafficLights)
    {
        totalCycleTime += tl->timeOpen;
    }
    totalCycleTime += transitionTime * trafficLights.size();
}

void TrafficLightGroup::update()
{
    if (trafficLights.empty())
        return;

    groupTimer++;

    if (inGreenPhase)
    {
        auto currentTrafficLight = trafficLights[currentIndex];

        if (groupTimer >= currentTrafficLight->timeOpen)
        {
            currentTrafficLight->state = false;
            groupTimer = 0;
            inGreenPhase = false;
            inTransitionPhase = true;
        }
    }
    else if (inTransitionPhase)
    {
        if (groupTimer >= transitionTime)
        {
            groupTimer = 0;
            inGreenPhase = true;
            inTransitionPhase = false;

            currentIndex = (currentIndex + 1) % trafficLights.size();
            auto nextTrafficLight = trafficLights[currentIndex];
            nextTrafficLight->state = true;
        }
    }
}