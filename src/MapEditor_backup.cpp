#define OLC_PGE_APPLICATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "MapEditor.h"
//#include "stb_image_write.h"

// Override base class with your custom functionality
MapEditor::MapEditor()
{
	sAppName = "Map Editor";

	MapEditor* MapEditorObject;

	// Number of tiles in world
	vWorldSize = { 10, 10 };
	// Size of single tile graphic
	vTileSize = { 48, 25 };
	// Where to place tile (0 ; 0) on screen (in tile size steps)
	vOrigin = { 5, 1 };
	// Where to place tile selector interface tile (0 ; 0) on screen (in tile size steps)
	vTileSelectorOrigin = { 0, 17 };
	// Where to place save box tile (0 ; 0) on screen (in tile size steps)
	vSaveBoxOrigin = { 3, -6 };
	vLoadBoxOrigin = { 4, -5 };
	// Bounds checking 
	vTileSelectorBounds = { 5, 18 };
	vSaveBoxBoundsY = { 0, 20 };
	vSaveBoxBoundsX = { 472, 511 };
	vLoadBoxBoundsY = { 20, 40 };
	vLoadBoxBoundsX = { 472, 511 };
	// Size of tile selector 
	vTileSelectorSize = { 244, 52 };
	// Origin point of tile selector for debugging 
	vTileSelectorCell = { 0, 0 };
	// Location of test map 
	sFileData = { "C:/Users/marschag/source/repos/MapEditor/MapEditor/maps/test_map.csv" };
	// Sprite object that holds all imagery
	sprIsom = nullptr;
	// Pointer to create 2D world array
	m_pWorld = nullptr;

	vDynamicWorldSizeX = nullptr;
	vDynamicWorldSizeY = nullptr;

	// For storing 2D tile selector array
	m_pTileSelector = nullptr;
	// Flag for wrldspace bounds checking 
	bInWorldBounds = false;
	// Flag for tile selector interface bounds checking 
	bInTileSelectorBounds = false;
	// Flag for save box bounds checking 
	bInSaveBoxBounds = false;
	bInLoadBoxBounds = false;
	// Flag for checking if tile is selected 
	bLeftMouseClicked = false;
	// Flag for checking if map is loaded in 
	bIsMapLoaded = false;
}

MapEditor::~MapEditor()
{
	//delete[] sprIsom; // Got read access violation when terminating the application
	delete[] WorldMap;
	delete[] m_pWorld;
	delete[] m_pTileSelector;
}

bool MapEditor::OnUserCreate()
{
	//DynamicWorldSize* DynamicWorldSizeObject;

	// Load sprites used in demonstration
	sprIsom = new olc::Sprite("./sprites/tiles_stroke.png");

	// Creates decals, so sprites are loaded onto the GPU for better performance 
	decIsom = new olc::Decal(sprIsom);

	//vDynamicWorldSizeX = new int32_t[vWorldSize.x]{ 0 }; // Should array length be initialized with vWS.x and vWS.y or the assigned value?
	//DynamicWorldSizeY = new int32_t[vWorldSize.y]{ 0 };

	// Create empty world
	m_pWorld = new long long[(long long) vWorldSize.x * vWorldSize.y]{ 0 };
	vDynamicWorldSizeX = &vWorldSize.x;
	vDynamicWorldSizeY = &vWorldSize.y;

	// Create empty tile selector world
	m_pTileSelector = new int[(vTileSelectorSize.x * vTileSelectorSize.y) / (vTileSize.x * vTileSize.y)]{ 0 };

	return true;
}

void MapEditor::TileSelector(int vSelectedInterfaceAreaX, int vSelectedInterfaceAreaY)
{
	if (vSelectedInterfaceAreaX == 0 && vSelectedInterfaceAreaY == 425) //magic numbers; avoid these!!!
		*m_pTileSelector = 0;
	if (vSelectedInterfaceAreaX == -24 && vSelectedInterfaceAreaY == 437)
		*m_pTileSelector = 1;
	if (vSelectedInterfaceAreaX == 24 && vSelectedInterfaceAreaY == 437)
		*m_pTileSelector = 2;
	if (vSelectedInterfaceAreaX == 0 && vSelectedInterfaceAreaY == 449)
		*m_pTileSelector = 3;

	// For debugging 
	//vTileSelectorCell.x = vSelectedInterfaceAreaX,
	//vTileSelectorCell.y = vSelectedInterfaceAreaY;
}

void MapEditor::SaveMapData()
{
	std::ofstream mapData;

	mapData.open(sFileData);

	mapData << vDynamicWorldSizeX << "\n" << *vDynamicWorldSizeY << "\n";

	for (int i = 0; i < *vDynamicWorldSizeX * *vDynamicWorldSizeY; i++)
	{
		mapData << m_pWorld[i] << "\n";
	}

	mapData.close();
}

bool MapEditor::LoadMapData()
{
	std::ifstream MapData(sFileData, std::ios::in); // add std::ios:binary if map data contains info whether tile is solid or not

	if (MapData.is_open())
	{
		MapData >> *vDynamicWorldSizeX >> *vDynamicWorldSizeY;
		
		for (int i = 0; i < *vDynamicWorldSizeX * *vDynamicWorldSizeY; i++)
		{
			MapData >> m_pWorld[i];
		}

		bIsMapLoaded = true;
		return true;
	}

	return false;
}

bool MapEditor::OnUserUpdate(float fElapsedTime)
{
	Clear(olc::WHITE);

	// Get Mouse in world
	olc::vi2d vMouse = { olc::PixelGameEngine::GetMouseX(), olc::PixelGameEngine::GetMouseY() };

	// Work out active cell
	olc::vi2d vCell = { vMouse.x / vTileSize.x, vMouse.y / vTileSize.y };

	// Work out mouse offset into cell
	olc::vi2d vOffset = { vMouse.x % vTileSize.x, vMouse.y % vTileSize.y };

	// Sample into cell offset colour
	//olc::Pixel col = sprIsom->GetPixel(3 * vTileSize.x + vOffset.x, vOffset.y); 
	olc::Pixel col = sprIsom->GetPixel(vOffset.x, 2 * vTileSize.y + vOffset.y); //get pixel value of Cheat_tile

	// Work out selected cell by transforming screen cell
	olc::vi2d vSelected =
	{
		(vCell.y - vOrigin.y) + (vCell.x - vOrigin.x),
		(vCell.y - vOrigin.y) - (vCell.x - vOrigin.x)
	};

	// Work out selected tile selector interface cell by transforming screen cell (OTS: On Tile Selector)
	olc::vi2d vSelectedOTS =
	{
		(vCell.x - vTileSelectorOrigin.x),
		(vCell.y - vTileSelectorOrigin.y)
	};

	// Work out selected save box interface cell by transforming screen cell 
	olc::vi2d vSelectedSaveBox =
	{
		(vCell.x - vSaveBoxOrigin.x),
		(vCell.y - vSaveBoxOrigin.y)
	};

	olc::vi2d vSelectedLoadBox =
	{
		(vCell.x - vLoadBoxOrigin.x),
		(vCell.y - vLoadBoxOrigin.y)
	};

	// Is selected tile within world space
	if (vSelected.x >= 0 && vSelected.x < *vDynamicWorldSizeX && vSelected.y >= 0 && vSelected.y < *vDynamicWorldSizeY)
		bInWorldBounds = true;
	else
		bInWorldBounds = false;

	// Is selected tile within tile selector interface world space
	if (vSelectedOTS.x >= 0 && vSelectedOTS.x < vTileSelectorBounds.x && vSelectedOTS.y >= 0 && vSelectedOTS.y < vTileSelectorBounds.y)
		bInTileSelectorBounds = true;
	else
		bInTileSelectorBounds = false;

	// Is selected tile within save box interface area
	if (vMouse.x >= vSaveBoxBoundsX.x && vMouse.x <= vSaveBoxBoundsX.y && vMouse.y >= vSaveBoxBoundsY.x && vMouse.y <= vSaveBoxBoundsY.y)
		bInSaveBoxBounds = true;
	else
		bInSaveBoxBounds = false;

	if (vMouse.x >= vLoadBoxBoundsX.x && vMouse.x <= vLoadBoxBoundsX.y && vMouse.y >= vLoadBoxBoundsY.x && vMouse.y <= vLoadBoxBoundsY.y)
		bInLoadBoxBounds = true;
	else
		bInLoadBoxBounds = false;

	// "Bodge" selected cell by sampling corners
	// Must be before selection from tile selector interface is assigned, otherwise tiles will not be painted in rhombic shape
	if (col == olc::RED) vSelected += {-1, +0};
	if (col == olc::BLUE) vSelected += {+0, -1};
	if (col == olc::GREEN) vSelected += {+0, +1};
	if (col == olc::YELLOW) vSelected += {+1, +0};

	// Handle mouse click to toggle if a tile is visible or not
	// Selection from tile selector interface is assigned here to the selected worldspace cell 
	if (olc::PixelGameEngine::GetMouse(0).bPressed || olc::PixelGameEngine::GetMouse(0).bHeld)
	{

		switch (*m_pTileSelector)
		{
		case 0:
			if (vSelected.x >= 0 && vSelected.x < *vDynamicWorldSizeX && vSelected.y >= 0 && vSelected.y < *vDynamicWorldSizeY)
				++m_pWorld[vSelected.y * *vDynamicWorldSizeX + vSelected.x] = 0;
			break;
		case 1:
			if (vSelected.x >= 0 && vSelected.x < *vDynamicWorldSizeX && vSelected.y >= 0 && vSelected.y < *vDynamicWorldSizeY)
				++m_pWorld[vSelected.y * *vDynamicWorldSizeX + vSelected.x] = 1;
			break;
		case 2:
			if (vSelected.x >= 0 && vSelected.x < *vDynamicWorldSizeX && vSelected.y >= 0 && vSelected.y < *vDynamicWorldSizeY)
				++m_pWorld[vSelected.y * *vDynamicWorldSizeX + vSelected.x] = 2;
			break;
		case 3:
			if (vSelected.x >= 0 && vSelected.x < *vDynamicWorldSizeX && vSelected.y >= 0 && vSelected.y < *vDynamicWorldSizeY)
				++m_pWorld[vSelected.y * *vDynamicWorldSizeX + vSelected.x] = 3;
			break;
		}
	}

	// Labmda function to convert "world" coordinate into screen space
	auto ToScreen = [&](int x, int y)
	{
		return olc::vi2d
		{
			(vOrigin.x * vTileSize.x) + (x - y) * (vTileSize.x / 2),
			(vOrigin.y * vTileSize.y) + (x + y) * (vTileSize.y / 2)
		};
	};

	// Lambda function for tile selector interface coordinate conversion 
	auto InterfaceToScreen = [&](int x, int y)
	{
		return olc::vi2d
		{
			(vTileSelectorOrigin.x * vTileSize.x) + (x - y) * (vTileSize.x / 2),
			(vTileSelectorOrigin.y * vTileSize.y) + (x + y) * (vTileSize.y / 2)
		};
	};

	// Change world map size on key press
	if (GetKey(olc::UP).bPressed)
		vWorldSize.y += 1;
	if (GetKey(olc::DOWN).bPressed)
		vWorldSize.y -= 1;
	if (GetKey(olc::RIGHT).bPressed)
		vWorldSize.x += 1;
	if (GetKey(olc::LEFT).bPressed)
		vWorldSize.x -= 1;

	// Drag the world map accross the screen 
	if (GetMouse(1).bHeld && bInWorldBounds == false && bInLoadBoxBounds == false && bInSaveBoxBounds == false && bInTileSelectorBounds == false)
	{
		vOrigin = vCell;
	}

	// Draw World - has binary transparancy so enable masking
	olc::PixelGameEngine::SetPixelMode(olc::Pixel::MASK);

	// Load map data in
	if (bInLoadBoxBounds == true && GetMouse(0).bPressed && LoadMapData())
	{	
		for (int y = 0; y < *vDynamicWorldSizeY; y++)
		{
			for (int x = 0; x < *vDynamicWorldSizeX; x++)
			{
				// Convert cell coordinate to world space
				olc::vi2d vWorld = ToScreen(x, y);

				for (int n = 0; n < *vDynamicWorldSizeX * *vDynamicWorldSizeY; n++) 
				{

					switch (m_pWorld[n])
					{
						//Dirt tile
					case 0:
						DrawPartialSprite(vWorld.x, vWorld.y, sprIsom, 0, 0, vTileSize.x, vTileSize.y);
						break;
					case 1:
						//Grass tile
						DrawPartialSprite(vWorld.x, vWorld.y, sprIsom, vTileSize.x, 0, vTileSize.x, vTileSize.y);
						break;
					case 2:
						//Stone
						DrawPartialSprite(vWorld.x, vWorld.y, sprIsom, 2 * vTileSize.x, 0, vTileSize.x, vTileSize.y);
						break;
					case 3:
						//Water
						DrawPartialSprite(vWorld.x, vWorld.y, sprIsom, 0, vTileSize.y, vTileSize.x, vTileSize.y);
						break;
					case 4:
						//Tree
						DrawPartialSprite(vWorld.x, vWorld.y - (2 * vTileSize.y), sprIsom, 2 * vTileSize.x, vTileSize.y, vTileSize.x, 3 * vTileSize.y);
						break;
					case 5:
						//Stone brown 
						DrawPartialSprite(vWorld.x + (0.25 * vTileSize.x), vWorld.y - (0.25 * vTileSize.y), sprIsom, 0, 3 * vTileSize.y, 0.5 * vTileSize.x, vTileSize.y);
						break;
					}
				}
			}
		}
	}

	// Main for loop for tile rendering 
	// (0,0) is at top, defined by vOrigin, so draw from top to bottom
	// to ensure tiles closest to camera are drawn last
	for (int y = 0; y < *vDynamicWorldSizeY; y++)
	{

		for (int x = 0; x < *vDynamicWorldSizeX; x++)
		{
			// Convert cell coordinate to world space
			olc::vi2d vWorld = ToScreen(x, y);

			switch (m_pWorld[y * *vDynamicWorldSizeX + x]) //m_pWorld[y * vWorldSize.x + x]
			{
				//Dirt tile
			case 0:
				DrawPartialSprite(vWorld.x, vWorld.y, sprIsom, 0, 0, vTileSize.x, vTileSize.y);
				break;
			case 1:
				//Grass tile
				DrawPartialSprite(vWorld.x, vWorld.y, sprIsom, vTileSize.x, 0, vTileSize.x, vTileSize.y);
				break;
			case 2:
				//Stone
				DrawPartialSprite(vWorld.x, vWorld.y, sprIsom, 2 * vTileSize.x, 0, vTileSize.x, vTileSize.y);
				break;
			case 3:
				//Water
				DrawPartialSprite(vWorld.x, vWorld.y, sprIsom, 0, vTileSize.y, vTileSize.x, vTileSize.y);
				break;
			case 4:
				//Tree
				DrawPartialSprite(vWorld.x, vWorld.y - (2 * vTileSize.y), sprIsom, 2 * vTileSize.x, vTileSize.y, vTileSize.x, 3 * vTileSize.y);
				break;
			case 5:
				//Stone brown 
				DrawPartialSprite(vWorld.x + (0.25 * vTileSize.x), vWorld.y - (0.25 * vTileSize.y), sprIsom, 0, 3 * vTileSize.y, 0.5 * vTileSize.x, vTileSize.y);
				break;
			}

			// Save map data

			if (bInSaveBoxBounds == true && GetMouse(0).bPressed)
			{
				SaveMapData();
			}
		}
	}

	// Draw Selected Cell - Has varying alpha components
	SetPixelMode(olc::Pixel::ALPHA);

	// Convert selected cell coordinate to world space
	olc::vi2d vSelectedWorld = ToScreen(vSelected.x, vSelected.y);

	// Convert selected tile selector interface cell coordinate to world space
	olc::vi2d vSelectedInterfaceArea = InterfaceToScreen(vSelectedOTS.x, vSelectedOTS.y);

	// Tile selector interface screenspace coordinates is passed in here
	if (bInTileSelectorBounds == true && GetMouse(0).bPressed)
		TileSelector(vSelectedInterfaceArea.x, vSelectedInterfaceArea.y);

	// Draw "highlight" tile
	if (bInWorldBounds == true)
		DrawPartialSprite(vSelectedWorld.x, vSelectedWorld.y, sprIsom, 1 * vTileSize.x, vTileSize.y, vTileSize.x, vTileSize.y);

	// Draw red rectangle around tile selector interface tiles
	if (bInTileSelectorBounds == true)
		DrawRect(vSelectedOTS.x * vTileSize.x, (vSelectedOTS.y + 17) * vTileSize.y, vTileSize.x, vTileSize.y, olc::RED);
	
	// Check if cursor is in bounds of the save box, and if left mouse button clicked, call write to png function
	/*
	if (bInSaveBoxBounds == true && GetMouse(0).bPressed)
	{

		// Makes a screenshot of the whole screen (without interfaces)
		int size = GetDrawTarget()->width * GetDrawTarget()->height;
		uint32_t* imgData = new uint32_t[size];
		for (int i = 0; i < size; i++)
		{
			imgData[i] = GetDrawTarget()->GetData()[i].n;
		}
		stbi_write_png("test.png", GetDrawTarget()->width, GetDrawTarget()->height, 4, imgData, 0);
		delete[] imgData;
	}
	*/
	// Draw tile selector interface 
	DrawRect(0, 424, 240, 52, olc::BLACK);

	// Draw box for save function
	DrawRect(472, 0, 40, 20, olc::BLACK);

	DrawRect(472, 20, 40, 20, olc::BLACK);

	// Draw contents of tile selector interface 
	DrawPartialSprite(0, 425, sprIsom, 0, 0, vTileSize.x, vTileSize.y); //Dirt tile 

	DrawPartialSprite(0, 451, sprIsom, vTileSize.x, 0, vTileSize.x, vTileSize.y); //Grass tile

	DrawPartialSprite(49, 425, sprIsom, 2 * vTileSize.x, 0, vTileSize.x, vTileSize.y); //Stone

	DrawPartialSprite(49, 451, sprIsom, 0, vTileSize.y, vTileSize.x, vTileSize.y); //Water

	// Go back to normal drawing with no expected transparency
	SetPixelMode(olc::Pixel::NORMAL);

	// Draw write to png save string 
	DrawString(478, 6, "Save", olc::BLACK);

	DrawString(478, 26, "Load", olc::BLACK);

	// Draw Debug Info - '+' here is operator overloading (string concatenation) 
	DrawString(4, 4, "Mouse   : " + std::to_string(vMouse.x) + ", " + std::to_string(vMouse.y), olc::BLACK);
	DrawString(4, 14, "Cell    : " + std::to_string(vCell.x) + ", " + std::to_string(vCell.y), olc::BLACK);
	DrawString(4, 24, "Selected: " + std::to_string(vSelected.x) + ", " + std::to_string(vSelected.y), olc::BLACK);
	DrawString(4, 34, "World size: " + std::to_string(vWorldSize.x) + ", " + std::to_string(vWorldSize.y), olc::BLACK);
	//DrawString(4, 34, "Tile selector: " + std::to_string(*m_pTileSelector), olc::BLACK);
	//DrawString(4, 34, "In load box bounds: " + std::to_string(bInLoadBoxBounds), olc::BLACK);
	return true;
}

int main()
{
	MapEditor demo;
	if (demo.Construct(1268, 476, 1, 1, 0)) // 512, 476
		demo.Start();

	return 0;
}