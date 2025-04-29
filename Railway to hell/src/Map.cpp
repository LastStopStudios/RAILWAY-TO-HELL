
#include "Engine.h"
#include "Render.h"
#include "Textures.h"
#include "Map.h"
#include "Log.h"
#include "Physics.h"
#include <math.h>
#include <unordered_map>
#include "SceneLoader.h"

Map::Map() : Module(), mapLoaded(false)
{
    name = "map";
}

// Destructor
Map::~Map()
{}

// Called before render is available
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
    int tilesRendered = 0;
    if (mapLoaded) {
        for (const auto& mapLayer : mapData.layers) {
            if (mapLayer->properties.GetProperty("Draw") != NULL && mapLayer->properties.GetProperty("Draw")->value == true) {
                // Get camera position and dimensions without parallax (for debugging)
                int camX = Engine::GetInstance().render->camera.x;
                int camY = Engine::GetInstance().render->camera.y;
                int camW = Engine::GetInstance().render->camera.w;
                int camH = Engine::GetInstance().render->camera.h;

                // Calculate which tiles are visible in the camera view
                // Convert camera position to tile coordinates (notice we use absolute value)
                int startTileX = abs(camX) / mapData.tileWidth;
                int startTileY = abs(camY) / mapData.tileHeight;

                // Calculate how many tiles fit in the camera view
                int tilesWide = (camW / mapData.tileWidth) + 2; // Add extra tiles for partial visibility
                int tilesHigh = (camH / mapData.tileHeight) + 2;

                // Calculate end points
                int endTileX = startTileX + tilesWide;
                int endTileY = startTileY + tilesHigh;

                // Clamp to map boundaries
                if (endTileX > mapData.width) endTileX = mapData.width;
                if (endTileY > mapData.height) endTileY = mapData.height;
                if (startTileX < 0) startTileX = 0;
                if (startTileY < 0) startTileY = 0;

                // Debug output (remove in production)
                // SDL_Log("Camera: %d,%d,%d,%d | Tiles: %d,%d to %d,%d", 
                //         camX, camY, camW, camH, startTileX, startTileY, endTileX, endTileY);

                // Draw only tiles in view
                for (int x = startTileX; x < endTileX; x++) {
                    for (int y = startTileY; y < endTileY; y++) {
                        int gid = mapLayer->Get(x, y);
                        if (gid != 0) {
                            tilesRendered++;
                            TileSet* tileSet = GetTilesetFromTileId(gid);
                            if (tileSet != nullptr) {
                                SDL_Rect tileRect = tileSet->GetRect(gid);
                                Vector2D worldPos = MapToWorld(x, y);
                                Engine::GetInstance().render->DrawTexture(tileSet->texture, worldPos.getX(), worldPos.getY(), &tileRect);

                            }
                        }
                    }
                }
            }
        }
    }
    //LOG("Tiles rendereizadas: %d de %d posibles", tilesRendered, mapData.width * mapData.height); To prove that frustum culling works
    return ret;
}

//Implement function to the Tileset based on a tile id
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

bool Map::PostUpdate() {
    for (const auto& mapLayer : mapData.layers) {
        //Check if the property Draw exist get the value, if it's true draw the lawyer
        if (mapLayer->properties.GetProperty("Prof1") != NULL && mapLayer->properties.GetProperty("Prof1")->value == true) {
            for (int i = 0; i < mapData.width; i++) {
                for (int j = 0; j < mapData.height; j++) {
                    //Get the gid from tile
                    int gid = mapLayer->Get(i, j);
                    //Check if the gid is different from 0 - some tiles are empty
                    if (gid != 0) {

                        TileSet* tileSet = GetTilesetFromTileId(gid);
                        if (tileSet != nullptr) {
                            //Get the Rect from the tileSetTexture;
                            SDL_Rect tileRect = tileSet->GetRect(gid);
                            //Get the screen coordinates from the tile coordinates
                            Vector2D mapCoord = MapToWorld(i, j);
                            //Draw the texture
                            Engine::GetInstance().render->DrawTexture(tileSet->texture, mapCoord.getX(), mapCoord.getY(), &tileRect);
                        }
                    }
                }
            }
        }
    }
    return true;
}

// Called before quitting
bool Map::CleanUp()
{
    LOG("Unloading map");

    for (PhysBody* colliders : colliders) {
        Engine::GetInstance().physics.get()->DeletePhysBody(colliders);
    }
    colliders.clear();

    for (const auto& tileset : mapData.tilesets) {
        delete tileset;
    }
    mapData.tilesets.clear();

    for (const auto& layer : mapData.layers)
    {
        delete layer;
    }
    mapData.layers.clear();

    return true;
}

// Load new map
bool Map::Load(std::string path, std::string fileName)
{
    bool ret = false;

    // Assigns the name of the map file and the path
    mapFileName = fileName;
    mapPath = path;
    std::string mapPathName = mapPath + mapFileName;

    pugi::xml_document mapFileXML;
    pugi::xml_parse_result result = mapFileXML.load_file(mapPathName.c_str());

    if(result == NULL)
	{
		LOG("Could not load map xml file %s. pugi error: %s", mapPathName.c_str(), result.description());
		ret = false;
    }
    else {

        mapData.width = mapFileXML.child("map").attribute("width").as_int();
        mapData.height = mapFileXML.child("map").attribute("height").as_int();
        mapData.tileWidth = mapFileXML.child("map").attribute("tilewidth").as_int();
        mapData.tileHeight = mapFileXML.child("map").attribute("tileheight").as_int();
       
        //Iterate the Tileset
        for(pugi::xml_node tilesetNode = mapFileXML.child("map").child("tileset"); tilesetNode!=NULL; tilesetNode = tilesetNode.next_sibling("tileset"))
		{
            //Load Tileset attributes
			TileSet* tileSet = new TileSet();
            tileSet->firstGid = tilesetNode.attribute("firstgid").as_int();
            tileSet->name = tilesetNode.attribute("name").as_string();
            tileSet->tileWidth = tilesetNode.attribute("tilewidth").as_int();
            tileSet->tileHeight = tilesetNode.attribute("tileheight").as_int();
            tileSet->spacing = tilesetNode.attribute("spacing").as_int();
            tileSet->margin = tilesetNode.attribute("margin").as_int();
            tileSet->tileCount = tilesetNode.attribute("tilecount").as_int();
            tileSet->columns = tilesetNode.attribute("columns").as_int();

			//Load the tileset image
			std::string imgName = tilesetNode.child("image").attribute("source").as_string();
            tileSet->texture = Engine::GetInstance().textures->Load((mapPath+imgName).c_str());

			mapData.tilesets.push_back(tileSet);
		}

        for (pugi::xml_node layerNode = mapFileXML.child("map").child("layer"); layerNode != NULL; layerNode = layerNode.next_sibling("layer")) {

            //Load the attributes and saved in a new MapLayer
            MapLayer* mapLayer = new MapLayer();
            mapLayer->id = layerNode.attribute("id").as_int();
            mapLayer->name = layerNode.attribute("name").as_string();
            mapLayer->width = layerNode.attribute("width").as_int();
            mapLayer->height = layerNode.attribute("height").as_int();

            //Call Load Layer Properties
            LoadProperties(layerNode, mapLayer->properties);

            //Iterate over all the tiles and assign the values in the data array
            for (pugi::xml_node tileNode = layerNode.child("data").child("tile"); tileNode != NULL; tileNode = tileNode.next_sibling("tile")) {
                mapLayer->tiles.push_back(tileNode.attribute("gid").as_int());
            }

            //add the layer to the map
            mapData.layers.push_back(mapLayer);
        }
        std::unordered_map<std::string, int> layerNameToId = {
           {"Sensores", 1},
           {"Colisiones", 2},
		   {"Dialogos", 3},
		   {"Ascensores", 4},
           {"Giro", 5},
           {"Ice", 6}
        };
        float x = 0.0f;
        float y = 0.0f;
        float width = 0.0f;
        float height = 0.0f;
        for (pugi::xml_node layerNode = mapFileXML.child("map").child("objectgroup"); layerNode != NULL; layerNode = layerNode.next_sibling("objectgroup")) {

            // Get the name of the object group (PLATFORM, SPIKE, CHECKPOINT, etc.)
            std::string layerName = layerNode.attribute("name").as_string();

            for (pugi::xml_node tileNode = layerNode.child("object"); tileNode != NULL; tileNode = tileNode.next_sibling("object")) {

                // Assign the correct values from the XML
                x = tileNode.attribute("x").as_float();
                y = tileNode.attribute("y").as_float();
                width = tileNode.attribute("width").as_float();
                height = tileNode.attribute("height").as_float();

                PhysBody* rect = nullptr;
                auto it = layerNameToId.find(layerName);
                int layerId = (it != layerNameToId.end()) ? it->second : 0; // Default value: 0 for platforms
                MapLayer* mapLayer = new MapLayer();

                switch (layerId) {
                case 1: // Scene change sensor
                    rect = Engine::GetInstance().physics.get()->CreateRectangleSensor(x + width / 2, y + height / 2, width, height, STATIC);
                    rect->ctype = ColliderType::SENSOR;
                    for (pugi::xml_node propertieNode = tileNode.child("properties").child("property"); propertieNode; propertieNode = propertieNode.next_sibling("property"))
                    {
                        if (propertieNode.attribute("name") = "Sensor") {
                            rect->sensorID = propertieNode.attribute("value").as_string();
                        }
                    }
                    break;
                case 2: // Layer objects called collisions (in tmx of scene 2 )
                    rect = Engine::GetInstance().physics.get()->CreateRectangle(x + width / 2, y + height / 2, width, height, STATIC);
                    rect->ctype = ColliderType::PLATFORM; //for now I leave it like this, to make the collision as with the rest, if it needs to be changed for something to be changed, its type is already created.
                    break;
                case 3: // Sensor Dialogs
                    rect = Engine::GetInstance().physics.get()->CreateRectangleSensor(x + width / 2, y + height / 2, width, height, STATIC);
                    rect->ctype = ColliderType::DIALOGOS;//save sensor properties for dialogs
                    for (pugi::xml_node propertieNode = tileNode.child("properties").child("property"); propertieNode; propertieNode = propertieNode.next_sibling("property"))
                    {
                        if (propertieNode.attribute("name") = "ID") {
                            rect->ID = propertieNode.attribute("value").as_string();
                            rect->Salio = false;
                        }
                    }
                    break;
                case 4: // Elevator sensor
                    rect = Engine::GetInstance().physics.get()->CreateRectangleSensor(x + width / 2, y + height / 2, width, height, STATIC);
                    rect->ctype = ColliderType::ASCENSORES;
                    for (pugi::xml_node propertieNode = tileNode.child("properties").child("property"); propertieNode; propertieNode = propertieNode.next_sibling("property"))
                    {
                        if (propertieNode.attribute("name") = "Sensor") {
                            rect->sensorID = propertieNode.attribute("value").as_string();
                        }
                    }
                    break;

                case 5: // Sensor for enemy patrolling
                    rect = Engine::GetInstance().physics.get()->CreateRectangleSensor(x + width / 2, y + height / 2, width, height, STATIC);
                    rect->ctype = ColliderType::GIRO;
                    break;

                case 6: // Ice Platforms
                    rect = Engine::GetInstance().physics.get()->CreateRectangle(x + width / 2, y + height / 2, width, height, STATIC);
                    rect->ctype = ColliderType::PLATFORMICE;
                    break;
                
                default: // Platforms
                    rect = Engine::GetInstance().physics.get()->CreateRectangle(x + width / 2, y + height / 2, width, height, STATIC);
                    rect->ctype = ColliderType::PLATFORM;
                    break;
                }
                //Call Load Layer Properties

                    // Add the collider to the list
                colliders.push_back(rect);
            }

        }
        //Iterate the layer and create colliders
        for (const auto& mapLayer : mapData.layers) {
            if (mapLayer->name == "Collisions") {
                for (int i = 0; i < mapData.width; i++) {
                    for (int j = 0; j < mapData.height; j++) {
                        int gid = mapLayer->Get(i, j);
                        if (gid == 49) {
                            Vector2D mapCoord = MapToWorld(i, j);
                            PhysBody* c1 = Engine::GetInstance().physics.get()->CreateRectangle(mapCoord.getX() + mapData.tileWidth / 2, mapCoord.getY() + mapData.tileHeight / 2, mapData.tileWidth, mapData.tileHeight, STATIC);
                            c1->ctype = ColliderType::PLATFORM;
                        }
                    }
                }
            }
        }

        ret = true;

        //LOG all the data loaded iterate all tilesetsand LOG everything
        if (ret == true)
        {
            LOG("Successfully parsed map XML file :%s", fileName.c_str());
            LOG("width : %d height : %d", mapData.width, mapData.height);
            LOG("tile_width : %d tile_height : %d", mapData.tileWidth, mapData.tileHeight);

            LOG("Tilesets----");

            //iterate the tilesets
            for (const auto& tileset : mapData.tilesets) {
                LOG("name : %s firstgid : %d", tileset->name.c_str(), tileset->firstGid);
                LOG("tile width : %d tile height : %d", tileset->tileWidth, tileset->tileHeight);
                LOG("spacing : %d margin : %d", tileset->spacing, tileset->margin);
            }
            			
            LOG("Layers----");

            for (const auto& layer : mapData.layers) {
                LOG("id : %d name : %s", layer->id, layer->name.c_str());
				LOG("Layer width : %d Layer height : %d", layer->width, layer->height);
            }   
        }
        else {
            LOG("Error while parsing map file: %s", mapPathName.c_str());
        }

        if (mapFileXML) mapFileXML.reset();

    }

    mapLoaded = ret;
    return ret;
}

//Load a group of properties from a node and fill a list with it
bool Map::LoadProperties(pugi::xml_node& node, Properties& properties)
{
    bool ret = false;

    for (pugi::xml_node propertieNode = node.child("properties").child("property"); propertieNode; propertieNode = propertieNode.next_sibling("property"))
    {
        Properties::Property* p = new Properties::Property();
        p->name = propertieNode.attribute("name").as_string();
        p->value = propertieNode.attribute("value").as_bool();

        properties.propertyList.push_back(p);
    }

    return ret;
}

Vector2D Map::MapToWorld(int x, int y) const
{
    Vector2D ret;

    ret.setX(x * mapData.tileWidth);
    ret.setY(y * mapData.tileHeight);



    return ret;
}

Vector2D Map::WorldToMap(int x, int y) {

    Vector2D ret(0, 0);

    ret.setX(x / mapData.tileWidth);
    ret.setY(y / mapData.tileHeight);

    return ret;
}

MapLayer* Map::GetNavigationLayer() {
    for (const auto& layer : mapData.layers) {
		if (layer->properties.GetProperty("Navigation") != NULL && 
            layer->properties.GetProperty("Navigation")->value) {
			return layer;
		}
	}

	return nullptr;
}

//Implement a method to get the value of a custom property
Properties::Property* Properties::GetProperty(const char* name)
{
    for (const auto& property : propertyList) {
        if (property->name == name) {
			return property;
		}
    }

    return nullptr;
}



