
#include "Engine.h"
#include "Render.h"
#include "Textures.h"
#include "Map.h"
#include "Log.h"
#include "Physics.h"
#include <math.h>
#include <unordered_map>
#include <string>
#include "SceneLoader.h"

Map::Map() : Module(), mapLoaded(false), nextSensorId(1)
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
        // iterate all tiles in a layer
        for (const auto& mapLayer : mapData.layers) {
            //Check if the property Draw exist get the value, if it's true draw the lawyer
            if (mapLayer->properties.GetProperty("Draw") != NULL && mapLayer->properties.GetProperty("Draw")->value == true) {

                Vector2D camPos = Vector2D(Engine::GetInstance().render->camera.x * -1, Engine::GetInstance().render->camera.y * -1);
                if (camPos.getX() < 0) camPos.setX(0);
                if (camPos.getY() < 0) camPos.setY(0);
                Vector2D camPosTile = WorldToMap(camPos.getX(), camPos.getY());

                Vector2D camSize = Vector2D(Engine::GetInstance().render->camera.w, Engine::GetInstance().render->camera.h);
                Vector2D camSizeTile = WorldToMap(camSize.getX(), camSize.getY());

                Vector2D limits = Vector2D(camPosTile.getX() + camSizeTile.getX(), camPosTile.getY() + camSizeTile.getY());
                if (limits.getX() > mapData.width) limits.setX(mapData.width);
                if (limits.getY() > mapData.height) limits.setY(mapData.height);

                for (int i = camPosTile.getX(); i < limits.getX(); i++) {
                    for (int j = camPosTile.getY(); j < limits.getY(); j++) {

                        //Get the gid from tile
                        int gid = mapLayer->Get(i, j);
                        //Check if the gid is different from 0 - some tiles are empty
                        if (gid != 0) {
                            //L09: TODO 3: Obtain the tile set using GetTilesetFromTileId
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

    sensorsList.clear();
    dsensorsList.clear();

    valor.clear();
    nextSensorId = 1;
    currentId = 0;
    sensorValue.clear();

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

        // Define un mapa para convertir nombres de capas en enteros
        std::unordered_map<std::string, int> layerNameToId = {
            {"Sensores", 1},
            {"Colisiones", 2},
            {"Dialogos", 3}
        };
               
        float x = 0.0f;
        float y = 0.0f;
        float width = 0.0f;
        float height = 0.0f;
        for (pugi::xml_node layerNode = mapFileXML.child("map").child("objectgroup"); layerNode != NULL; layerNode = layerNode.next_sibling("objectgroup")) {

            // Get the name of the object group (PLATFORM, SPIKE, CHECKPOINT, etc.)
            std::string layerName = layerNode.attribute("name").as_string();
            LOG("!!!!!!!!!layer del Mapa cargado, Nombre: %s !!!!!!!!", layerName.c_str());

            for (pugi::xml_node tileNode = layerNode.child("object"); tileNode != NULL; tileNode = tileNode.next_sibling("object")) {
                LOG("!!!!!!!!!layer del TileNode cargado, Nombre: %d !!!!!!!!", tileNode);
                LOG("!!!!!!!!!layer del TileNode cargado, Nombre: %d !!!!!!!!", layerName.c_str());
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
                    LOG("!!!!!!!!!Sensor Creado!!!!!!!!");
                    rect->ctype = ColliderType::SENSOR;
                    pasarx = x;//posicion x del sensor
                    pasary = y;//posicion y del sensor
                    currentId = nextSensorId++; //Obtiene ID para los sensores
                    SensorScene newSensor;
                    newSensor.id = currentId;
                    newSensor.x = pasarx;
                    newSensor.y = pasary;
                    sensorsList.push_back(newSensor);//Lo guardamos todo en la lista
                    LOG("!!!!!!!!!Datos Sensor, ID: %d, X: %d, Y: %d!!!!!!!!!", currentId, pasarx, pasary);
                    LoadProperties(tileNode, mapLayer->properties);//guardar propiedades de los sensores cambio de escena
                    break;
                case 2: // Layer objetos llamada colisiones (en el tmx de scene 2)
                    rect = Engine::GetInstance().physics.get()->CreateRectangleSensor(x + width / 2, y + height / 2, width, height, STATIC);
                    rect->ctype = ColliderType::PLATFORM; //por ahora lo dejo asi, para que haga la colision como con el resto, si se necesita cambiar par algo que se cambie, su tipo ya esta creado.
                    break;
                case 3: // Sensor cambio de escena
                    rect = Engine::GetInstance().physics.get()->CreateRectangleSensor(x + width / 2, y + height / 2, width, height, STATIC);
                    rect->ctype = ColliderType::DIALOGOS;//guardar propiedades de los sensores para dialogos
                    dpasarx = x;//posicion x del sensor de dialogos
                    dpasary = y;//posicion y del sensor de dialogos
                    LoadProperties(tileNode, mapLayer->properties);
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
                            PhysBody* c1 = Engine::GetInstance().physics.get()->CreateRectangle(mapCoord.getX()+ mapData.tileWidth/2, mapCoord.getY()+ mapData.tileHeight/2, mapData.tileWidth, mapData.tileHeight, STATIC);
                            c1->ctype = ColliderType::PLATFORM;
                        }
                    }
                }
            }
        }

        ret = true;

        // LOG all the data loaded iterate all tilesetsand LOG everything
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

// Load a group of properties from a node and fill a list with it
bool Map::LoadProperties(pugi::xml_node& node, Properties& properties)
{
   
    bool ret = false;
    for (pugi::xml_node propertieNode = node.child("properties").child("property"); propertieNode; propertieNode = propertieNode.next_sibling("property"))
    {
       
        Properties::Property* p = new Properties::Property();
        p->name = propertieNode.attribute("name").as_string();
        LOG("!!!!Nombre propiedad, Nombre:  %s !!!!", p->name.c_str());
       
        if(p->name == "Draw") {
            p->value = propertieNode.attribute("value").as_bool(); // (!!) I'm assuming that all values are bool !!
            LOG("!!!!propiedad value guardada, tipo:  %d !!!!", p->value);
       }else if (p->name == "Sensor"){
            p->sensor = propertieNode.attribute("value").as_string();
            LOG("!!!!propiedad sensor guardada, tipo:  %s !!!!", p->sensor.c_str());
            // añadir el ID al sensor
            p->id = currentId;
            LOG("!!!!ID sensor guardada, ID:  %d !!!!", p->id);
       }
       else if (p->name == "Navigation"){
           p->value = propertieNode.attribute("value").as_bool(); // (!!) I'm assuming that all values are bool !!
            LOG("!!!!propiedad value guardada (Navigation), tipo:  %d !!!!", p->value);

       }else if (p->name == "Dialogo") {
            p->dialogo = propertieNode.attribute("value").as_int();//id para los dialogos
           
            SensorDialogos dnewSensor;//guardamos los datos del dialogo en una lista
            dnewSensor.id = p->dialogo;//guardamos su Id
            dnewSensor.x = dpasarx;//guardamos su x
            dnewSensor.y = dpasary;//guardamos su y
            dsensorsList.push_back(dnewSensor);//Lo guardamos todo en la lista
            LOG("!!!!propiedad sensor Dialogos guardada, tipo:  %d !!!!", p->dialogo);
            LOG("!!!!Id guardada en lista de dialogos, ID:  %d !!!!", dnewSensor.id);
       }
          
              
       
        


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


// Conseguir el valor del sensor pasandole la id
std::string Properties::GetPropertyWID( int id)
{
   for (const auto& property : propertyList) { 
        if (property->id == id) { 

            LOG("Propiedad encontrada, nombre: %s", property->name.c_str()); // Motstrar el nombre de la propiedad
            LOG("Propiedad encontrada, nombre: %s", property->sensor.c_str()); // Motstrar el valor de la propiedad
            LOG("Propiedad enncontrada, propiedad: %s", property);
            return property->sensor;
          
        }
    }

   LOG("Propiedad no encontrada - ID: %d", id);
   return ""; // String vacío si no se encuentra
}


// Implement a method to get the value of a custom property
Properties::Property* Properties::GetProperty(const char* name)
{
    for (const auto& property : propertyList) {
        if (property->name == name) {
           
			return property;
            
		}
    }

    return nullptr;
}


int Map::GetSensorId(float px, float py) const {//Pilar la id del sensor con la posicion del player
    const float MargenDeError = 30.0f;  // Martgen de error para la colision

    for (const auto& sensor : sensorsList) {
        float Xposconmargen1 = px + MargenDeError;
        float Xposconmargen2 = px - MargenDeError;

        float Yposconmargen1 = py + MargenDeError;
        float Yposconmargen2 = py - MargenDeError;

        if ((sensor.x <= Xposconmargen1 && sensor.x >= Xposconmargen2)||(sensor.y <= Yposconmargen1 && sensor.y >= Yposconmargen2)) {
            LOG("Sensor encontrado, ID: %d", sensor.id);
            return sensor.id;//devuelve la id del sensor qu esta en esa posicion 
           
        }
    }
    LOG("Sensor no encontrado");
    return -1;  // duevuelve -1 si no encuentra sensor en esa posicion 
   
}

std::string Map::ValorPorId(int id) {
   LOG("!!!!!!!!!!!!Entro en ValorPorId!!!!!!!!!!!");
    /*for (auto& mapLayer : mapData.layers) {
        if (mapLayer->name == "Sensores") {
            return mapLayer->properties.GetPropertyWID(id).c_str();
        }
    }
    return ""; // Si no encuentra la capa
    */
    sensorValue.clear();
  /* for (auto& layer : this->mapData.layers) {//recorre las capas del mapa
        if (layer->name == "Sensores") {//busca la capa llamada sensores
            // 3. Llamar a tu función
            std::string sensorValue = layer->properties.GetPropertyWID(id);//Busca el valor de sensor con la id

            if (!sensorValue.empty()) {
                LOG("Valor del sensor (ID %d): %s", id, sensorValue.c_str());
                return sensorValue;
            }
            break;
        }
    }

    LOG("No se encontró sensor con ID %d", id);
    return "";*/
    for (const auto& mapLayer : mapData.layers) {
        if (mapLayer->properties.GetProperty("Sensor") != NULL) {
            sensorValue = mapLayer->properties.GetProperty("Sensor")->sensor;
            LOG("!!!!!!!!!!!!Valor sensor: %d!!!!!!!!!!!", sensorValue);
            return sensorValue;
        }
    }

    LOG("No se encontró sensor con ID %d", id);
    return "";
}
