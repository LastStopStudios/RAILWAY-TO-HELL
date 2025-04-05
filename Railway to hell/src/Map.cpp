
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

    if (mapLoaded) {

        for (const auto& mapLayer : mapData.layers) {
            //Check if the property Draw exist get the value, if it's true draw the lawyer
            if (mapLayer->properties.GetProperty("Draw") != NULL && mapLayer->properties.GetProperty("Draw")->value == true) {
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
    }

    return ret;
}

// L09: TODO 2: Implement function to the Tileset based on a tile id
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

            //L09: TODO 6 Call Load Layer Properties
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
           {"Giro", 5}
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
                int layerId = (it != layerNameToId.end()) ? it->second : 0; // Valor por defecto: 0 para plataformas
                MapLayer* mapLayer = new MapLayer();

                switch (layerId) {
                case 1: // Sensor cambio de escena
                    rect = Engine::GetInstance().physics.get()->CreateRectangleSensor(x + width / 2, y + height / 2, width, height, STATIC);
                    rect->ctype = ColliderType::SENSOR;
                    for (pugi::xml_node propertieNode = tileNode.child("properties").child("property"); propertieNode; propertieNode = propertieNode.next_sibling("property"))
                    {
                        if (propertieNode.attribute("name") = "Sensor") {
                            rect->sensorID = propertieNode.attribute("value").as_string();
                            LOG("!!!!!!!!!Sensor, ID: %s,!!!!!!!!!", rect->sensorID);
                        }
                    }
                    break;
                case 2: // Layer objetos llamada colisiones (en el tmx de scene 2)
                    rect = Engine::GetInstance().physics.get()->CreateRectangle(x + width / 2, y + height / 2, width, height, STATIC);
                    rect->ctype = ColliderType::PLATFORM; //por ahora lo dejo asi, para que haga la colision como con el resto, si se necesita cambiar par algo que se cambie, su tipo ya esta creado.
                    break;
                case 3: // Sensor Dialogos
                    rect = Engine::GetInstance().physics.get()->CreateRectangleSensor(x + width / 2, y + height / 2, width, height, STATIC);
                    rect->ctype = ColliderType::DIALOGOS;//guardar propiedades de los sensores para dialogos
                    for (pugi::xml_node propertieNode = tileNode.child("properties").child("property"); propertieNode; propertieNode = propertieNode.next_sibling("property"))
                    {
                        if (propertieNode.attribute("name") = "ID") {
                            rect->ID = propertieNode.attribute("value").as_string();
                            LOG("!!!!!!!!!Sensor, ID: %s,!!!!!!!!!", rect->ID);
                        }
                    }
                    break;
                case 4: // Sensor ascensores
                    rect = Engine::GetInstance().physics.get()->CreateRectangleSensor(x + width / 2, y + height / 2, width, height, STATIC);
                    rect->ctype = ColliderType::ASCENSORES;
                    for (pugi::xml_node propertieNode = tileNode.child("properties").child("property"); propertieNode; propertieNode = propertieNode.next_sibling("property"))
                    {
                        if (propertieNode.attribute("name") = "Sensor") {
                            rect->sensorID = propertieNode.attribute("value").as_string();
                            LOG("!!!!!!!!!Sensor, ID: %s,!!!!!!!!!", rect->sensorID);
                        }
                    }
                    break;

                case 5: // Sensor para patrullaje enemigos
                    rect = Engine::GetInstance().physics.get()->CreateRectangleSensor(x + width / 2, y + height / 2, width, height, STATIC);
                    rect->ctype = ColliderType::GIRO;
                    break;
                
                default: // Plataformas
                    rect = Engine::GetInstance().physics.get()->CreateRectangle(x + width / 2, y + height / 2, width, height, STATIC);
                    rect->ctype = ColliderType::PLATFORM;
                    break;
                }
                //Call Load Layer Properties

                    // Añade el collider a la lista
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

        // L06: TODO 5: LOG all the data loaded iterate all tilesetsand LOG everything
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

// L09: TODO 6: Load a group of properties from a node and fill a list with it
bool Map::LoadProperties(pugi::xml_node& node, Properties& properties)
{
    bool ret = false;

    for (pugi::xml_node propertieNode = node.child("properties").child("property"); propertieNode; propertieNode = propertieNode.next_sibling("property"))
    {
        Properties::Property* p = new Properties::Property();
        p->name = propertieNode.attribute("name").as_string();
        p->value = propertieNode.attribute("value").as_bool(); // (!!) I'm assuming that all values are bool !!

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

// L09: TODO 7: Implement a method to get the value of a custom property
Properties::Property* Properties::GetProperty(const char* name)
{
    for (const auto& property : propertyList) {
        if (property->name == name) {
			return property;
		}
    }

    return nullptr;
}



