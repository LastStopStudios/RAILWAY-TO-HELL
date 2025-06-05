
#include "Engine.h"
#include "Render.h"
#include "Textures.h"
#include "Map.h"
#include "Log.h"
#include "Physics.h"
#include <math.h>
#include <unordered_map>
#include "SceneLoader.h"
#include "GlobalSettings.h"

#define TEXTURE_SIZE_MULTIPLIER 1.5f

Map::Map() : Module(), mapLoaded(false)
{
    name = "map";
}

Map::~Map()
{
}

bool Map::Awake()
{
    name = "map";
    LOG("Loading Map Parser");
    return true;
}

bool Map::Start() {
    return true;
}

bool Map::Update(float dt)
{
    bool ret = true;

    currentScaleFactor = GlobalSettings::GetInstance().GetTextureMultiplier();

    if (mapLoaded) {
        for (const auto& mapLayer : mapData.layers) {
            auto* prop = mapLayer->properties.GetProperty("Draw");
            if (prop && prop->value) {
                RenderLayer(mapLayer, currentScaleFactor);
            }
        }
    }
    return ret;
}

void Map::DrawTileWithoutGaps(SDL_Texture* texture, int x, int y, SDL_Rect* section, float scale) const
{
    if (texture == nullptr) return;

    float worldX = (float)x * scale;
    float worldY = (float)y * scale;

    float renderX = Engine::GetInstance().render->camera.x + worldX;
    float renderY = Engine::GetInstance().render->camera.y + worldY;

    float scaledWidth = mapData.tileWidth * scale;
    float scaledHeight = mapData.tileHeight * scale;

    SDL_FRect destRect;
    destRect.x = renderX;
    destRect.y = renderY;
    destRect.w = scaledWidth;
    destRect.h = scaledHeight;

    SDL_RenderCopyF(Engine::GetInstance().render.get()->renderer, texture, section, &destRect);
}

TileSet* Map::GetTilesetFromTileId(int gid) const
{
    TileSet* set = nullptr;

    for (const auto& tileset : mapData.tilesets) {
        if (gid >= tileset->firstGid && gid < (tileset->firstGid + tileset->tileCount)) {
            set = tileset;
            break;
        }
    }

    return set;
}

bool Map::PostUpdate()
{
    bool ret = true;

    if (mapLoaded) {
        for (const auto& mapLayer : mapData.layers) {
            auto* prop = mapLayer->properties.GetProperty("Prof1");
            if (prop && prop->value) {
                RenderLayer(mapLayer, currentScaleFactor);
            }
        }
    }
    return ret;
}

void Map::RenderLayer(MapLayer* mapLayer, float scaleFactor)
{
    int camX = Engine::GetInstance().render->camera.x;
    int camY = Engine::GetInstance().render->camera.y;
    int camW = Engine::GetInstance().render->camera.w;
    int camH = Engine::GetInstance().render->camera.h;

    int scaledTileWidth = mapData.tileWidth * scaleFactor;
    int scaledTileHeight = mapData.tileHeight * scaleFactor;

    float startTileXf = (float)abs(camX) / (float)scaledTileWidth;
    float startTileYf = (float)abs(camY) / (float)scaledTileHeight;

    int startTileX = (int)floor(startTileXf);
    int startTileY = (int)floor(startTileYf);

    int tilesWide = (camW / scaledTileWidth) + 2;
    int tilesHigh = (camH / scaledTileHeight) + 2;

    int endTileX = startTileX + tilesWide;
    int endTileY = startTileY + tilesHigh;

    if (endTileX > mapData.width) endTileX = mapData.width;
    if (endTileY > mapData.height) endTileY = mapData.height;
    if (startTileX < 0) startTileX = 0;
    if (startTileY < 0) startTileY = 0;

    for (int x = startTileX; x < endTileX; x++) {
        for (int y = startTileY; y < endTileY; y++) {
            int gid = mapLayer->Get(x, y);
            if (gid != 0) {
                TileSet* tileSet = GetTilesetFromTileId(gid);
                if (tileSet != nullptr) {
                    SDL_Rect tileRect = tileSet->GetRect(gid);
                    Vector2D worldPos = MapToWorld(x, y);
                    DrawTileWithoutGaps(tileSet->texture, worldPos.getX(), worldPos.getY(), &tileRect, scaleFactor);
                }
            }
        }
    }
}

bool Map::CleanUp()
{
    LOG("Unloading map");

    for (PhysBody* body : colliders) {
        Engine::GetInstance().physics.get()->DeletePhysBody(body);
    }
    colliders.clear();

    for (const auto& tileset : mapData.tilesets) {
        delete tileset;
    }
    mapData.tilesets.clear();

    for (const auto& layer : mapData.layers) {
        delete layer;
    }
    mapData.layers.clear();

    return true;
}

bool Map::Load(std::string path, std::string fileName)
{
    bool ret = false;

    mapFileName = fileName;
    mapPath = path;
    std::string mapPathName = mapPath + mapFileName;

    pugi::xml_document mapFileXML;
    pugi::xml_parse_result result = mapFileXML.load_file(mapPathName.c_str());

    if (!result) {
        LOG("Could not load map xml file %s. pugi error: %s", mapPathName.c_str(), result.description());
        ret = false;
    }
    else {
        // Load map dimensions
        mapData.width = mapFileXML.child("map").attribute("width").as_int();
        mapData.height = mapFileXML.child("map").attribute("height").as_int();
        mapData.tileWidth = mapFileXML.child("map").attribute("tilewidth").as_int();
        mapData.tileHeight = mapFileXML.child("map").attribute("tileheight").as_int();

        // Load tilesets
        for (pugi::xml_node tilesetNode = mapFileXML.child("map").child("tileset"); tilesetNode; tilesetNode = tilesetNode.next_sibling("tileset")) {
            TileSet* tileSet = new TileSet();
            tileSet->firstGid = tilesetNode.attribute("firstgid").as_int();
            tileSet->name = tilesetNode.attribute("name").as_string();
            tileSet->tileWidth = tilesetNode.attribute("tilewidth").as_int();
            tileSet->tileHeight = tilesetNode.attribute("tileheight").as_int();
            tileSet->spacing = tilesetNode.attribute("spacing").as_int();
            tileSet->margin = tilesetNode.attribute("margin").as_int();
            tileSet->tileCount = tilesetNode.attribute("tilecount").as_int();
            tileSet->columns = tilesetNode.attribute("columns").as_int();

            std::string imgName = tilesetNode.child("image").attribute("source").as_string();
            tileSet->texture = Engine::GetInstance().textures->Load((mapPath + imgName).c_str());

            mapData.tilesets.push_back(tileSet);
        }

        // Load map layers
        for (pugi::xml_node layerNode = mapFileXML.child("map").child("layer"); layerNode; layerNode = layerNode.next_sibling("layer")) {
            MapLayer* mapLayer = new MapLayer();
            mapLayer->id = layerNode.attribute("id").as_int();
            mapLayer->name = layerNode.attribute("name").as_string();
            mapLayer->width = layerNode.attribute("width").as_int();
            mapLayer->height = layerNode.attribute("height").as_int();

            LoadProperties(layerNode, mapLayer->properties);

            for (pugi::xml_node tileNode = layerNode.child("data").child("tile"); tileNode; tileNode = tileNode.next_sibling("tile")) {
                mapLayer->tiles.push_back(tileNode.attribute("gid").as_int());
            }

            mapData.layers.push_back(mapLayer);
        }

        // Map object layers to collider types
        std::unordered_map<std::string, int> layerNameToId = {
            {"Sensores", 1},
            {"Colisiones", 2},
            {"Dialogos", 3},
            {"Ascensores", 4},
            {"Giro", 5},
            {"Ice", 6},
            {"Abyss", 7},
            {"Boss", 8}
        };

        float x = 0.0f, y = 0.0f, width = 0.0f, height = 0.0f;

        for (pugi::xml_node layerNode = mapFileXML.child("map").child("objectgroup"); layerNode; layerNode = layerNode.next_sibling("objectgroup")) {
            std::string layerName = layerNode.attribute("name").as_string();

            for (pugi::xml_node tileNode = layerNode.child("object"); tileNode; tileNode = tileNode.next_sibling("object")) {
                x = tileNode.attribute("x").as_float();
                y = tileNode.attribute("y").as_float();
                width = tileNode.attribute("width").as_float();
                height = tileNode.attribute("height").as_float();

                PhysBody* rect = nullptr;
                auto it = layerNameToId.find(layerName);
                int layerId = (it != layerNameToId.end()) ? it->second : 0;

                switch (layerId) {
                case 1:
                    rect = Engine::GetInstance().physics.get()->CreateRectangleSensor(x + width / 2, y + height / 2, width, height, STATIC);
                    rect->ctype = ColliderType::SENSOR;
                    for (pugi::xml_node prop = tileNode.child("properties").child("property"); prop; prop = prop.next_sibling("property")) {
                        if (std::string(prop.attribute("name").as_string()) == "Sensor") {
                            rect->sensorID = prop.attribute("value").as_string();
                        }
                    }
                    break;
                case 2:
                    rect = Engine::GetInstance().physics.get()->CreateRectangle(x + width / 2, y + height / 2, width, height, STATIC);
                    rect->ctype = ColliderType::PLATFORM;
                    break;
                case 3:
                    rect = Engine::GetInstance().physics.get()->CreateRectangleSensor(x + width / 2, y + height / 2, width, height, STATIC);
                    rect->ctype = ColliderType::DIALOGOS;
                    for (pugi::xml_node prop = tileNode.child("properties").child("property"); prop; prop = prop.next_sibling("property")) {
                        if (std::string(prop.attribute("name").as_string()) == "ID") {
                            rect->ID = prop.attribute("value").as_string();
                            rect->Salio = false;
                        }
                    }
                    break;
                case 4:
                    rect = Engine::GetInstance().physics.get()->CreateRectangleSensor(x + width / 2, y + height / 2, width, height, STATIC);
                    rect->ctype = ColliderType::ASCENSORES;
                    for (pugi::xml_node prop = tileNode.child("properties").child("property"); prop; prop = prop.next_sibling("property")) {
                        if (std::string(prop.attribute("name").as_string()) == "Sensor") {
                            rect->sensorID = prop.attribute("value").as_string();
                        }
                    }
                    break;
                case 5:
                    rect = Engine::GetInstance().physics.get()->CreateRectangleSensor(x + width / 2, y + height / 2, width, height, STATIC);
                    rect->ctype = ColliderType::GIRO;
                    break;
                case 6:
                    rect = Engine::GetInstance().physics.get()->CreateRectangle(x + width / 2, y + height / 2, width, height, STATIC);
                    rect->ctype = ColliderType::PLATFORMICE;
                    break;
                case 7:
                    rect = Engine::GetInstance().physics.get()->CreateRectangleSensor(x + width / 2, y + height / 2, width, height, STATIC);
                    rect->ctype = ColliderType::ABYSS;
                    break;
                case 8:
                    rect = Engine::GetInstance().physics.get()->CreateRectangleSensor(x + width / 2, y + height / 2, width, height, STATIC);
                    rect->ctype = ColliderType::BOSSFLOOR;
                    break;
                default:
                    rect = Engine::GetInstance().physics.get()->CreateRectangle(x + width / 2, y + height / 2, width, height, STATIC);
                    rect->ctype = ColliderType::PLATFORM;
                    break;
                }

                colliders.push_back(rect);
            }
        }

        for (const auto& mapLayer : mapData.layers) {
            if (mapLayer->name == "Collisions") {
                for (int i = 0; i < mapData.width; i++) {
                    for (int j = 0; j < mapData.height; j++) {
                        int gid = mapLayer->Get(i, j);
                        if (gid == 49) {
                            Vector2D mapCoord = MapToWorld(i, j);
                            PhysBody* c1 = Engine::GetInstance().physics.get()->CreateRectangle(
                                mapCoord.getX() + mapData.tileWidth / 2,
                                mapCoord.getY() + mapData.tileHeight / 2,
                                mapData.tileWidth,
                                mapData.tileHeight,
                                STATIC
                            );
                            c1->ctype = ColliderType::PLATFORM;
                        }
                    }
                }
            }
        }

        ret = true;
        if (mapFileXML) mapFileXML.reset();
    }

    mapLoaded = ret;
    return ret;
}

bool Map::LoadProperties(pugi::xml_node& node, Properties& properties)
{
    bool ret = true;

    for (pugi::xml_node prop = node.child("properties").child("property"); prop; prop = prop.next_sibling("property")) {
        Properties::Property* p = new Properties::Property();
        p->name = prop.attribute("name").as_string();
        p->value = prop.attribute("value").as_bool();
        properties.propertyList.push_back(p);
    }

    return ret;
}

Vector2D Map::MapToWorld(int x, int y) const
{
    return Vector2D(x * mapData.tileWidth, y * mapData.tileHeight);
}

Vector2D Map::WorldToMap(int x, int y)
{
    return Vector2D(x / mapData.tileWidth, y / mapData.tileHeight);
}

MapLayer* Map::GetNavigationLayer()
{
    for (const auto& layer : mapData.layers) {
        auto* prop = layer->properties.GetProperty("Navigation");
        if (prop && prop->value) {
            return layer;
        }
    }
    return nullptr;
}

Properties::Property* Properties::GetProperty(const char* name)
{
    for (const auto& property : propertyList) {
        if (property->name == name) {
            return property;
        }
    }
    return nullptr;
}


