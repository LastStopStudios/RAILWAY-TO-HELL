#pragma once
#include <list>
#include <queue>
#include <functional> // for std::greater
#include "Vector2D.h"
#include "SDL2/SDL.h"
#include "Map.h"

enum ASTAR_HEURISTICS {
    MANHATTAN = 0,
    EUCLIDEAN,
    SQUARED
};

class Pathfinding
{

public:

	Pathfinding();

	~Pathfinding();

    //BFS Pathfinding methods
    void ResetPath(Vector2D pos);
    void DrawPath();
    bool IsWalkable(int x, int y);

    //Methods for BFS + Pathfinding and cost function for Dijkstra
    int MovementCost(int x, int y);
    void ComputePath(int x, int y);

    //A* Pathfinding methods
    void PropagateAStar(ASTAR_HEURISTICS heuristic);
    bool ReachedPlayer(Vector2D goal);

private:
    int Find(std::vector<Vector2D> vector, Vector2D elem);

public:

    //BFS Pathfinding variables
    Map* map;
    MapLayer* layerNav;
    std::queue<Vector2D> frontier;
    std::vector<Vector2D> visited;
    SDL_Texture* pathTex = nullptr;
    Vector2D destination;

    //Dijkstra Pathfinding variables
    std::priority_queue<std::pair<int, Vector2D>, std::vector<std::pair<int, Vector2D>>, std::greater<std::pair<int, Vector2D>> > frontierDijkstra;
    std::vector<Vector2D> breadcrumbs; //list of tiles that form the path
    std::vector<std::vector<int>> costSoFar; //matrix that stores the accumulated cost in the propagation of the Dijkstra algorithm
    std::list<Vector2D> pathTiles; //list of tiles that form the path
    SDL_Texture* tileX = nullptr; //texture used to show the path 

    //A* Pathfinding variables
    std::priority_queue<std::pair<int, Vector2D>, std::vector<std::pair<int, Vector2D>>, std::greater<std::pair<int, Vector2D>> > frontierAStar;
    //6915
    int blockedGid = 49; //Gid of the tiles that block the path - Important adjust this value to your map
    int highCostGid = 50; //Gid of the tiles that have high cost - Important adjust this value to your map

};

