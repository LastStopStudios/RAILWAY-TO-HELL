#pragma once

#include "Module.h"
#include <list>
#include <vector>
#include "Vector2D.h"
#include "Textures.h"
#include "SceneLoader.h" 

class PhysBody;
class Textures;
class SceneLoader; // Forward declaration

//Add attributes to the property structure
struct Properties
{
    struct Property
    {
        std::string name;
        bool value; //We assume that we are going to work only with bool for the moment
        std::string sensor;
        int dialogo;
    };

    std::list<Property*> propertyList;

    ~Properties()
    {
        for (const auto& property : propertyList)
        {
            delete property;
        }

        propertyList.clear();
    }

    //Method to ask for the value of a custom property
    Property* GetProperty(const char* name);

};

struct MapLayer
{
    int id;
    std::string name;
    int width;
    int height;
    std::vector<int> tiles;
    Properties properties;

    int Get(int i, int j) const
    {
        return tiles[(j * width) + i];
    }
};

// Ignore Terrain Types and Tile Types for now, but we want the image!

struct TileSet
{
    int firstGid;
    std::string name;
    int tileWidth;
    int tileHeight;
    int spacing;
    int margin;
    int tileCount;
    int columns;
    SDL_Texture* texture;

    SDL_Rect GetRect(unsigned int gid) {
        SDL_Rect rect = { 0 };

        int relativeIndex = gid - firstGid;
        rect.w = tileWidth;
        rect.h = tileHeight;
        rect.x = margin + (tileWidth + spacing) * (relativeIndex % columns);
        rect.y = margin + (tileHeight + spacing) * (relativeIndex / columns);

        return rect;
    }

};


struct MapData
{
	int width;
	int height;
	int tileWidth;
	int tileHeight;
    std::list<TileSet*> tilesets;
    std::list<MapLayer*> layers;
};

class Map : public Module
{
public:

    Map();

    // Destructor
    virtual ~Map();

    // Called before render is available
    bool Awake();

    bool PostUpdate();

    // Called before the first frame
    bool Start();

    // Called each loop iteration
    bool Update(float dt);

    // Called before quitting
    bool CleanUp();

    // Load new map
    bool Load(std::string path, std::string mapFileName);

    //Implement function to the Tileset based on a tile id
    TileSet* GetTilesetFromTileId(int gid) const;

    void DrawTileWithoutGaps(SDL_Texture* texture, int x, int y, SDL_Rect* section, float scale) const;

    //Load a group of properties 
    bool LoadProperties(pugi::xml_node& node, Properties& properties);

    int GetWidth() {
        return mapData.width;
    }

    int GetHeight() {
        return mapData.height;
    }

    int GetTileWidth() {
        return mapData.tileWidth;
    }

    int GetTileHeight() {
        return mapData.tileHeight;
    }

    MapLayer* GetNavigationLayer();

    Vector2D WorldToMap(int x, int y);
    Vector2D MapToWorld(int x, int y) const;

    void SetTargetScale(float newScale);
    void RenderLayer(MapLayer* mapLayer, float scaleFactor);
    float currentScaleFactor;

public: 
    std::string mapFileName;
    std::string mapPath;

private:
    bool mapLoaded;
    //Declare a variable data of the struct MapData
    MapData mapData;
    std::vector<PhysBody*> colliders;
};