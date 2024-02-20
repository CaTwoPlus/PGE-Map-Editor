#define OLC_PGE_APPLICATION
#include "MapEditor.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
//#include "stb_image_write.h"

// Override base class with your custom functionality
MapEditor::MapEditor() : pge_imgui(true)
{
	sAppName = "Map Editor";
	
	// Number of tiles in world
	vWorldSize = { 5, 5 };
	iNewWorldSizeX = vWorldSize.x, iNewWorldSizeY = vWorldSize.y;
	// Size of single tile graphic
	vTileSize = { 48, 25 };

	iSelectedCells = 1;
	iNumberOfTiles = TILE_TYPE_NR_ITEMS;

	iLayerEditor = 0;
	iLayerTop = 2;
	iLayerBackground = 1;

	// Where to place worldmap tile (0 ; 0) on screen (in tile size steps)
	vOrigin = { 5, 1 };
	// Location of test map 
	sFileData = { "C:/Users/ReBorn/source/repos/MapEditor/MapEditor/maps/test_map.csv" };
	// Sprite object that holds all imagery
	sprIsom = nullptr;
	// Flag for wrldspace bounds checking 
	bInWorldBounds = false;
	// Flag for loading/saving map
	bLoadMap = false;
	// Flag for checking if tile is selected 
	bLeftMouseClicked = false;
	// Flag for checking if map is loaded in 
	bIsMapLoaded = false;

	bBrushSizeIncr = false;
	bBrushSizeDecr = false;
	bFlipped = false;

	fAngle = 0.0f;
	fFlip_X = 1.0f;
	fFlip_Y = 1.0f;

	iSelectedTile = 0;
	iSelectedObject = 0;
}

MapEditor::~MapEditor()
{
}

class cMapGenerate
{
public:
	cMapGenerate(uint32_t x, uint32_t y)
	{
		nLehmer = (x & 0xFFFF) << 16 | (y & 0xFFFF); //16 bit coordinate resolution on each axis. Increase if bigger map is needed. 

		bFoliageExists = (RndInt(0, 10) == 0);
		if (!bFoliageExists) return;

		//If exists, determine the foliage type, and object type on it 
		iTileType = RndInt(2, 5);
		//iObjectType = RndInt(1, 4);
	}
public:
	bool bFoliageExists = true;
	int iLayerNumber = 0;
	int iTileType = MapEditor::TILE_TYPE_DIRT;
	int iObjectType = MapEditor::OBJ_TYPE_EMPTY;

public:
	uint32_t nLehmer = 0;
	uint32_t Lehmer32() //Random number generation
	{
		nLehmer += 0xe120fc15;
		uint64_t tmp;
		tmp = (uint64_t)nLehmer * 0x4a39b70d;
		uint32_t m1 = (tmp >> 32) ^ tmp;
		tmp = (uint64_t)m1 * 0x12fad5c9;
		uint32_t m2 = (tmp >> 32) ^ tmp;
		return m2;
	}

	int RndInt(int min, int max) //Return random int
	{
		return (Lehmer32() % (max - min)) + min;
	}

	double RndDouble(double min, double max) //Return random double
	{
		return ((double)Lehmer32() / double(0x7FFFFFFF)) * (max - min) + min;
	}
};

bool MapEditor::OnUserCreate()
{
	tv = olc::TileTransformedView({ ScreenWidth(), ScreenHeight() }, {1, 1});

	// Load sprites 
	sprIsom = new olc::Sprite("./sprites/tiles.png");

	// Creates decals, so sprites are loaded onto the GPU for better performance
	dclIsom = new olc::Decal(sprIsom);
	// Source image size - Width * Height
	vImageSize = { 345, 200 };

	// Create empty world
	m_vWorld.resize((long long)vWorldSize.x * vWorldSize.y, 0);
	m_vObjects.resize((long long)vWorldSize.x * vWorldSize.y, 0);

	// For ImGui
	bOpen = true;

	// Create array to store rotational state of cells 
	m_vCellRotation.resize((long long)vWorldSize.x * vWorldSize.y), m_vCellRotation = { 0 };
	
	// Order of layers: editor (foreground) -> top -> base (background)
	iLayerEditor = CreateLayer();
	EnableLayer(iLayerEditor, true);
	iLayerTop = CreateLayer();
	EnableLayer(iLayerTop, true);
	iLayerBackground = CreateLayer();
	EnableLayer(iLayerBackground, true);

	//Set a custom render function on layer 0.  Since DrawUI is a member of
	//our class, we need to use std::bind
	//If the pge_imgui was constructed with _register_handler = true, this line is not needed
	SetLayerCustomRenderFunction(0, std::bind(&MapEditor::DrawUI, this));

	Clear(olc::BLANK);

	// Set a custom render function on layer 0.  Since DrawUI is a member of
	//our class, we need to use std::bind
	//If the pge_imgui was constructed with _register_handler = true, this line is not needed
	SetLayerCustomRenderFunction(0, std::bind(&MapEditor::DrawUI, this));

	return true;
}

void MapEditor::TileSelector(int iSelectedBaseTile, int iSelectedObject)
{
	m_vTileSelector.clear();
	switch (iSelectedBaseTile)
	{
	case 0:
		m_vTileSelector.push_back(TILE_TYPE_EMPTY);
		break;
	case 1:
		m_vTileSelector.push_back(TILE_TYPE_DIRT);
		break;
	case 2:
		m_vTileSelector.push_back(TILE_TYPE_GRASS);
		break;
	case 3:
		m_vTileSelector.push_back(TILE_TYPE_LONG_GRASS);
		break;
	case 4:
		m_vTileSelector.push_back(TILE_TYPE_WATER);
		break;
	case 5:
		m_vTileSelector.push_back(TILE_TYPE_STONE);
		break;
	default:
		break;
	}

	m_vObjectSelector.clear();
	switch (iSelectedObject)
	{
	case 0:
		m_vObjectSelector.push_back(OBJ_TYPE_EMPTY);
		break;
	case 1:
		m_vObjectSelector.push_back(OBJ_TYPE_BROWN_ROCK);
		break;
	case 2:
		m_vObjectSelector.push_back(OBJ_TYPE_YELLOW_FLOWERS);
		break;
	case 3:
		m_vObjectSelector.push_back(OBJ_TYPE_TREE_TRUNK);
		break;
	case 4:
		m_vObjectSelector.push_back(OBJ_TYPE_TREE);
		break;
	case 5:
		m_vObjectSelector.push_back(OBJ_TYPE_SIGNPOST);
		break;
	default:
		break;
	}
}

void MapEditor::SaveMapData()
{
	std::ofstream MapData;
	MapData.open(sFileData);
	MapData << vWorldSize.x << "\n" << vWorldSize.y << "\n";
	for (int i = 0; i < vWorldSize.x * vWorldSize.y; i++)
		MapData << m_vWorld[i] << "\n" << m_vObjects[i] << "\n";
	MapData.close();
}

bool MapEditor::LoadMapData()
{
	std::ifstream MapData(sFileData, std::ios::in); // add std::ios:binary if map data contains info whether tile is solid or not
	if (MapData.is_open())
	{
		MapData >> vWorldSize.x >> vWorldSize.y;
		m_vWorld.resize((long long)vWorldSize.x * vWorldSize.y), m_vWorld = { 0 };
		m_vObjects.resize((long long)vWorldSize.x * vWorldSize.y), m_vObjects = { 0 };
		m_vCellRotation.resize((long long)vWorldSize.x * vWorldSize.y), m_vCellRotation = { 0 };
		while (!MapData.eof())
		{
			int tempwrld, tempobject,temprotation;
			MapData >> tempwrld >> tempobject >> temprotation;
			m_vWorld.push_back(tempwrld), m_vObjects.push_back(tempobject), m_vCellRotation.push_back(temprotation);
		}
		return true;
	}
	return false;
}

void MapEditor::DrawFlippedDecal(int WorldSizeIndex_X, int WorldSizeIndex_Y, int32_t vWorld_X, int32_t vWorld_Y, int32_t vCenter_X, int32_t vCenter_Y, float fAngle, float fFlip_X, float fFlip_Y)
{
	switch (m_vObjects[WorldSizeIndex_Y * (long long)vWorldSize.x + WorldSizeIndex_X])
	{
	SetDrawTarget(iLayerTop);
	case OBJ_TYPE_BROWN_ROCK:
		switch (m_vCellRotation[WorldSizeIndex_Y * (long long)vWorldSize.x + WorldSizeIndex_X])
		{
		case 0:
			//DrawPartialSprite(vWorld_X + (0.25 * vTileSize.x), vWorld.y - (0.25 * vTileSize.y), sprIsom, 0, 3 * vTileSize.y, 0.5 * vTileSize.x, vTileSize.y, 0, fScale_X);
			tv.DrawPartialDecal({ (float)vWorld_X,  (float)vWorld_Y}, dclIsom, { 0.0f, (float)5.24f * vTileSize.y }, { (float)vTileSize.x, (float)(1.24f * vTileSize.y) });
			break;
			
		case 1:
			fFlip_X = -0.9f;
			tv.DrawPartialDecal({ (float)vWorld_X + (float)(0.25 * vTileSize.x),  (float)vWorld_Y - (float)(0.25 * vTileSize.y) }, dclIsom,
				{ 0.0f, (float)5.0f * vTileSize.y }, { (float)vTileSize.x, (float)(1.24f * vTileSize.y) }, { fFlip_X, fFlip_Y });
			break;
		}
		break;
	case OBJ_TYPE_YELLOW_FLOWERS:
		switch (m_vCellRotation[WorldSizeIndex_Y * (long long)vWorldSize.x + WorldSizeIndex_X])
		{
		case 0:
			tv.DrawPartialDecal({ ((float)vWorld_X + 0.25f * vTileSize.x), ((float)vWorld_Y - 0.125f * vTileSize.y) }, dclIsom, { (float)1.5f * vTileSize.x, (float)3.0f * vTileSize.y }, { (float)0.5f * vTileSize.x, (float)vTileSize.y });
			break;
		case 1:
			fFlip_X = -0.9f;
			tv.DrawPartialDecal({ ((float)vWorld_X + 0.25f * vTileSize.x), ((float)vWorld_Y + 0.5f * vTileSize.y) }, dclIsom, { (float)1.5f * vTileSize.x, (float)3.0f * vTileSize.y }, { (float)0.5f * vTileSize.x, (float)vTileSize.y }, { fFlip_X, fFlip_Y });
			break;
		}
		break;
	case OBJ_TYPE_TREE_TRUNK:
		switch (m_vCellRotation[WorldSizeIndex_Y * (long long)vWorldSize.x + WorldSizeIndex_X])
		{
		case 0:
			tv.DrawPartialDecal({ (float)vWorld_X, (float)vWorld_Y }, dclIsom, { (float)1.0f * vTileSize.x, (float)3.0f * vTileSize.y }, { (float)0.5f * vTileSize.x, (float)vTileSize.y });
			break;
		case 1:
			fFlip_X = -0.9f;
			tv.DrawPartialDecal({ (float)vWorld_X, (float)vWorld_Y }, dclIsom, { (float)1.0f * vTileSize.x, (float)3.0f * vTileSize.y }, { (float)0.5f * vTileSize.x, (float)vTileSize.y }, { fFlip_X, fFlip_Y });
			break;
		}
		break;
	case OBJ_TYPE_TREE:
		tv.DrawPartialDecal({ (float)vWorld_X, (float)vWorld_Y - ((float)2 * vTileSize.y) }, dclIsom,
			{ (float)2 * vTileSize.x, (float)vTileSize.y }, { (float)vTileSize.x, (float)3 * vTileSize.y });
		break;
	}

	switch (m_vWorld[WorldSizeIndex_Y * (long long)vWorldSize.x + WorldSizeIndex_X])
	{
	SetDrawTarget(iLayerBackground);
	case TILE_TYPE_DIRT:
		tv.DrawPartialDecal({ (float)vWorld_X, (float)vWorld_Y }, dclIsom,
			{ 0, 0 }, { (float)vTileSize.x, (float)vTileSize.y });
		break;
	case TILE_TYPE_GRASS:
		tv.DrawPartialDecal({ (float)vWorld_X, (float)vWorld_Y }, dclIsom,
			{ (float)vTileSize.x, 0 }, { (float)vTileSize.x, (float)vTileSize.y });
		break;
	case TILE_TYPE_LONG_GRASS:
		tv.DrawPartialDecal({ (float)vWorld_X, (float)vWorld_Y }, dclIsom,
			{ (float)vTileSize.x, (float)2 * vTileSize.y }, { (float)vTileSize.x, (float)vTileSize.y });
		break;
	case TILE_TYPE_WATER:
		tv.DrawPartialDecal({ (float)vWorld_X, (float)vWorld_Y }, dclIsom,
			{ 0, (float)vTileSize.y }, { (float)vTileSize.x, (float)vTileSize.y });
		break;
	case TILE_TYPE_STONE:
		tv.DrawPartialDecal({ (float)vWorld_X, (float)vWorld_Y }, dclIsom,
			{ (float)2 * vTileSize.x, 0 }, { (float)vTileSize.x, (float)vTileSize.y });
		break;
	}
}



//Main loop//
bool MapEditor::OnUserUpdate(float fElapsedTime)
{
	if (GetKey(olc::ESCAPE).bPressed)
		return 0;

	SetDrawTarget(iLayerEditor);
	Clear(olc::WHITE);

	POINT pt;
	RECT rc;

	if (GetMouseWheel() > 1) // Now just have implement how many cells are selected with each change of iSelectedCells's value
	{
		++iSelectedCells;
		bBrushSizeIncr = true;
	}
	if (GetMouseWheel() < 0 && iSelectedCells > 1)
	{
		--iSelectedCells;
		bBrushSizeDecr = true;
	}
	if (GetKey(olc::F).bPressed)
	{
		fAngle += 0.25f;
		bFlipped = true;
	}
	if (GetKey(olc::G).bPressed)
	{
		fAngle -= 0.25f;
		bFlipped = false;
	}

	HWND hwnd = FindWindow(NULL, (LPCWSTR)"Map Editor");

	// Get Mouse in world
	olc::vi2d vMouse = { (olc::PixelGameEngine::GetMouseX()), (olc::PixelGameEngine::GetMouseY()) };
	// Work out active cell
	olc::vi2d vCell = { vMouse.x / vTileSize.x,  vMouse.y / vTileSize.y };

	vSelected =
	{
		(vCell.y - vOrigin.y) + (vCell.x - vOrigin.x),
		(vCell.y - vOrigin.y) - (vCell.x - vOrigin.x)
	};

	// Work out mouse offset into cell
	olc::vi2d vOffset = { vMouse.x % vTileSize.x, vMouse.y % vTileSize.x};

	// Is selected tile within world space
	if (vSelected.x >= 0 && vSelected.x < vWorldSize.x && vSelected.y >= 0 && vSelected.y < vWorldSize.y)
	{
		bInWorldBounds = true;
			/*
			if (vSelected.x >= 0 && vSelected.x < vWorldSize.x && vSelected.x >= 0 && vSelected.y < vWorldSize.y)
			{
				m_vObjects[vSelected.y * vWorldSize.x + vSelected.x] = *m_vObjectSelector;
				m_vWorld[vSelected.y * vWorldSize.x + vSelected.x] = *m_vTileSelector;
				m_vCellRotation[vSelected.y * vWorldSize.x + vSelected.x] = bFlipped;
			}*/
	}
	else
		bInWorldBounds = false;

	// Sample into cell offset colour
	//olc::Pixel col = sprIsom->GetPixel(3 * vTileSize.x + vOffset.x, vOffset.y); 
	olc::Pixel col = sprIsom->GetPixel(vOffset.x, 2 * vTileSize.y + vOffset.y); //get pixel value of Cheat_tile

	// "Bodge" selected cell by sampling corners
	// Must be before selection from tile selector interface is assigned, otherwise tiles will not be painted in rhombic shape
	// https://docs.microsoft.com/en-us/windows/win32/menurc/using-cursors#using-the-keyboard-to-move-the-cursor
	if (col == olc::RED) vSelected += {-1, +0};
	if (GetKey(olc::A).bPressed)
	{
		GetCursorPos(&pt);
		ScreenToClient(hwnd, &pt);
		pt.x -= (vSelected.x * 2);
		ClientToScreen(hwnd, &pt);
		SetCursorPos(pt.x, pt.y);
	}
	if (col == olc::BLUE) vSelected += {+0, -1};
	if (GetKey(olc::W).bPressed)
	{
		GetCursorPos(&pt);
		ScreenToClient(hwnd, &pt);
		pt.y -= (vSelected.y * 2);
		ClientToScreen(hwnd, &pt);
		SetCursorPos(pt.x, pt.y);
	}
	if (col == olc::GREEN) vSelected += {+0, +1};
	if (GetKey(olc::S).bPressed)
	{
		GetCursorPos(&pt);
		ScreenToClient(hwnd, &pt);
		pt.y += (vSelected.y * 2);
		ClientToScreen(hwnd, &pt);
		SetCursorPos(pt.x, pt.y);
	}
	if (col == olc::YELLOW) vSelected += {+1, +0};
	if (GetKey(olc::D).bPressed)
	{
		GetCursorPos(&pt);
		ScreenToClient(hwnd, &pt);
		pt.x += (vSelected.y * 2);
		ClientToScreen(hwnd, &pt);
		SetCursorPos(pt.x, pt.y);
	}
	
	// Labmda function to convert "world" coordinate into screen space
	auto ToScreen = [&](int x, int y)
	{
		return olc::vi2d
		{
			(vOrigin.x * vTileSize.x) + (x - y) * (vTileSize.x / 2), // + (x - y) * (vTileSize.x / 2) is screenspace to worldspace offset
			(vOrigin.y * vTileSize.y) + (x + y) * (vTileSize.y / 2)
		};
	};

	// Change world map size on key press
	if (GetKey(olc::UP).bPressed)
	{	
		int iCurrentTile = m_vWorld[((long long)vWorldSize.x * vWorldSize.y) - 1], iCurrentRotation = m_vCellRotation[((long long)vWorldSize.x * vWorldSize.y) - 1];
		++vWorldSize.y;
		for (int i = 0; i < vWorldSize.x; i++) m_vWorld.push_back(iCurrentTile), m_vObjects.push_back(0), m_vCellRotation.push_back(iCurrentRotation);
	}
	if (GetKey(olc::DOWN).bPressed && (vWorldSize.y > 1))
	{
		int iCurrentTile = m_vWorld[((long long)vWorldSize.x * vWorldSize.y) - 1], iCurrentRotation = m_vCellRotation[((long long)vWorldSize.x * vWorldSize.y) - 1];
		--vWorldSize.y;
		for (int i = 0; i < vWorldSize.x; i++) m_vWorld.push_back(iCurrentTile), m_vObjects.push_back(0), m_vCellRotation.push_back(iCurrentRotation);
	}
	if (GetKey(olc::RIGHT).bPressed)
	{
		int iCurrentTile = m_vWorld[((long long)vWorldSize.x * vWorldSize.y) - 1], iCurrentRotation = m_vCellRotation[((long long)vWorldSize.x * vWorldSize.y) - 1];
		++vWorldSize.x;
		for (int i = 0; i < vWorldSize.y; i++) m_vWorld.push_back(iCurrentTile), m_vObjects.push_back(0), m_vCellRotation.push_back(iCurrentRotation);
	}
	if (GetKey(olc::LEFT).bPressed && (vWorldSize.x > 1))
	{
		int iCurrentTile = m_vWorld[((long long)vWorldSize.x * vWorldSize.y) - 1], iCurrentRotation = m_vCellRotation[((long long)vWorldSize.x * vWorldSize.y) - 1];
		--vWorldSize.x;
		for (int i = 0; i < vWorldSize.y; i++) m_vWorld.push_back(iCurrentTile), m_vObjects.push_back(0), m_vCellRotation.push_back(iCurrentRotation);
	}

	// Draw World - has binary transperancy so enable masking
	olc::PixelGameEngine::SetPixelMode(olc::Pixel::MASK);

	// Draw map data or draw new map
	if (bLoadMap == true || bNewWorldToCreate == true)
	{
		if (bNewWorldToCreate)
		{
			vWorldSize.x = iNewWorldSizeX, vWorldSize.y = iNewWorldSizeY, m_vObjects.resize((long long)vWorldSize.x * vWorldSize.y);
			m_vWorld.assign((long long)vWorldSize.x* vWorldSize.y, iSelectedBaseTile), bNewWorldToCreate = false;
		}

		/*
		for (int y = 0; y < vWorldSize.y; y++)
		{
			for (int x = 0; x < vWorldSize.x; x++)
			{
				// Convert cell coordinate to world space
				olc::vi2d vWorld = ToScreen(vCell.x, vCell.y);
				
				for (int n = 0; n < vWorldSize.x * vWorldSize.y; n++) 
				{
					switch (m_vObjects[n])
					{
					case OBJ_TYPE_BROWN_ROCK:
						tv.DrawDecal({ (float)vWorld.x, (float)vWorld.y }, dclIsom);
						break;
					case OBJ_TYPE_YELLOW_FLOWERS:
						tv.DrawDecal({ (float)vWorld.x, (float)vWorld.y }, dclIsom);
						break;
					case OBJ_TYPE_TREE_TRUNK:
						tv.DrawDecal({ (float)vWorld.x, (float)vWorld.y }, dclIsom);
						break;
					case OBJ_TYPE_TREE:
						tv.DrawDecal({ (float)vWorld.x, (float)vWorld.y }, dclIsom);
						break;
					case OBJ_TYPE_SIGNPOST:
						tv.DrawDecal({ (float)vWorld.x, (float)vWorld.y }, dclIsom);
						break;
					}
					
					
					cMapGenerate map(vWorldSize.x, vWorldSize.y);
					if (map.bFoliageExists) //Draw the map here with existing tiles
					{
						m_vWorld[n] = map.iTileType;
						switch (m_vWorld[n])
						{
						case TILE_TYPE_DIRT:
							tv.DrawDecal({ (float)vWorld.x, (float)vWorld.y }, dclIsom);
							break;
						case TILE_TYPE_GRASS:
							tv.DrawDecal({ (float)vWorld.x, (float)vWorld.y }, dclIsom);
							break;
						case TILE_TYPE_LONG_GRASS:
							tv.DrawDecal({ (float)vWorld.x, (float)vWorld.y }, dclIsom);
							break;
						case TILE_TYPE_WATER:
							tv.DrawDecal({ (float)vWorld.x, (float)vWorld.y }, dclIsom);
							break;
						case TILE_TYPE_STONE:
							tv.DrawDecal({ (float)vWorld.x, (float)vWorld.y }, dclIsom);
							break;
						}
					}
					
				}
			}
		}
		*/
		bIsMapLoaded = true, bLoadMap = false, iSelectedBaseTile = 0, iSelectedObject = 0;
	}

	// Main for loop for tile rendering 
	// (0,0) is at top, defined by vOrigin, so draw from top to bottom
	// to ensure tiles closest to camera are drawn last
	if (bIsMapLoaded == true || bNewWorldToCreate == false)
	{
		for (int y = 0; y < vWorldSize.y; y++)
		{
			for (int x = 0; x < vWorldSize.x; x++)
			{
				// Convert cell coordinate to world space
				olc::vi2d vWorld = ToScreen(x, y);
				cMapGenerate map(vWorldSize.x, vWorldSize.y);
				if (map.bFoliageExists)
				{
					m_vWorld[(int)x*(int)y] = map.iTileType;
				}
				DrawFlippedDecal(x, y, vWorld.x, vWorld.y, vCell.x, vCell.y, fAngle, fFlip_X, fFlip_Y);
			}
		}
	}

	// Handle mouse click to toggle if a tile is visible or not
	// Selection from tile selector interface is assigned here to the selected worldspace cell 
	if (bInWorldBounds)
	{
		for (int i = 1; i <= iSelectedCells; i++) {
			for (int j = 1; j <= iSelectedCells; j++) {

				int index_x = vSelected.x + j;
				int index_y = vSelected.y + i;

				vOffset.x += index_x;
				vOffset.y += index_y;

				if (i != 1 || j != 1)
				{
					int index_x = vSelected.x + j;
					int index_y = vSelected.y + i;

					vOffset.x += index_x;
					vOffset.y += index_y;
				}

				else
				{
					int index_x = vSelected.x;
					int index_y = vSelected.y;

					vOffset.x += index_x;
					vOffset.y += index_y;
				}

			}
		}
		if (vSelected.x >= 0 && vSelected.x < vWorldSize.x && vSelected.x >= 0 && vSelected.y < vWorldSize.y)
		{
			if (m_vObjectSelector.size() > 0)
				m_vObjects[vSelected.y * (long long)vWorldSize.x + vSelected.x] = m_vObjectSelector[0];
			if (m_vTileSelector.size() > 0 && (olc::PixelGameEngine::GetMouse(0).bPressed || olc::PixelGameEngine::GetMouse(0).bHeld))
				m_vWorld[vSelected.y * (long long)vWorldSize.x + vSelected.x] = m_vTileSelector[0];
			m_vCellRotation[vSelected.y * (long long)vWorldSize.x + vSelected.x] = bFlipped;
		}
	}

	// Draw Selected Cell - Has varying alpha components
	SetPixelMode(olc::Pixel::ALPHA);

	// Convert selected cell coordinate to world space
	olc::vi2d vSelectedWorld = ToScreen(vSelected.x, vSelected.y);
	// Convert selector interface cell coordinate to world space
	//olc::vi2d vSelectedInterfaceAreaTiles = InterfaceToScreenTiles(vSelectedInterfaceCell.x, vSelectedInterfaceCell.y);

	// Handle pan & zoom
	if (GetMouse(2).bHeld) tv.UpdatePan(vSelectedWorld);
	if (GetMouse(2).bReleased) tv.EndPan(vSelectedWorld), vOrigin = vCell;
	if (GetKey(olc::PGDN).bPressed) tv.ZoomAtScreenPos(2.0f, GetMousePos());
	if (GetKey(olc::PGUP).bPressed) tv.ZoomAtScreenPos(0.5f, GetMousePos());

	// Draw "highlight" tile
	//if (bInWorldBounds == true)
		//DrawPartialSprite(vSelectedWorld.x, vSelectedWorld.y, sprIsom, 1 * vTileSize.x, vTileSize.y, vTileSize.x, vTileSize.y, iSelectedCells);
		DrawPartialDecal({ (float)vSelectedWorld.x, (float)vSelectedWorld.y }, dclIsom, { (float)1 * vTileSize.x, (float)vTileSize.y }, { (float)vTileSize.x, (float)vTileSize.y }, { (float)iSelectedCells, (float)iSelectedCells });

	// Draw Debug Info - '+' here is operator overloading (string concatenation) 
	DrawString(4, 24, "Mouse   : " + std::to_string(vMouse.x) + ", " + std::to_string(vMouse.y), olc::BLACK);
	DrawString(4, 34, "Cell    : " + std::to_string(vCell.x) + ", " + std::to_string(vCell.y), olc::BLACK);
	DrawString(4, 44, "Selected: " + std::to_string(vSelected.x) + ", " + std::to_string(vSelected.y), olc::BLACK);
	DrawString(4, 54, "World size: " + std::to_string(vWorldSize.x) + ", " + std::to_string(vWorldSize.y), olc::BLACK);
	DrawString(4, 64, "In world bounds: " + std::to_string(bInWorldBounds), olc::BLACK);
	DrawString(4, 74, "iSelectedBaseTile: " + std::to_string(iSelectedBaseTile) + ", iSelectedObject: " + std::to_string(iSelectedObject) , olc::BLACK);
	DrawString(4, 84, "vSelectedWorld: " + std::to_string(vSelectedWorld.x) + ";" + std::to_string(vSelectedWorld.y), olc::BLACK);
	DrawString(4, 94, "vOrigin: " + std::to_string(vOrigin.x) + ";" + std::to_string(vOrigin.y), olc::BLACK);
	//DrawString(4, 104, "vNewOrigin: " + std::to_string(vNewOrigin.x) + ";" + std::to_string(vNewOrigin.y), olc::BLACK);

// O------------------------------------------------------------------------------O
// | Interface																	  |
// O------------------------------------------------------------------------------O

	//Itt tartottam: imgui.cpp ->  IM_ASSERT((g.IO.DeltaTime > 0.0f || g.FrameCount == 0)              && "Need a positive DeltaTime!");
	//https://discord.com/channels/380484403458998276/380798602265493505/881277058095939585
	ImGui::ShowDemoWindow();
	
	// For debugging
	ImGui::ShowStackToolWindow(); 
	if (ImGui::Begin("Tile selector", &bOpen, ImGuiWindowFlags_AlwaysAutoResize))
		ImGui::End();
	// UV naming convention: from 0 -> inf; 1st tile is uv0-uv2, 2nd uv1-uv3...
	// Vector for storing UV coordinates
	std::vector<ImVec2> TileUVs, ObjUVs;
	// -1 == uses default padding (style.FramePadding)
	int frame_padding = -1;
	// Size of the image we want to make visible
	ImVec2 size = ImVec2((float)vTileSize.x, (float)vTileSize.y);
	ImVec2 sizeObj = ImVec2((float)vTileSize.x / 2.0f, (float)vTileSize.y);

	// Tiles and object selector UI
	// Tiles 
	// UV coordinates for starting pixels ([0.0,0.0] is upper-left), i.e. draw FROM
	ImVec2 uv0 = ImVec2(0.0f, 0.0f); TileUVs.push_back(uv0);
	// UV coordinates for tiles in our image file, i.e. draw TO 
	ImVec2 uv1 = ImVec2((float)vTileSize.x / (float)vImageSize.x, (float)vTileSize.y / (float)vImageSize.y); TileUVs.push_back(uv1);

	ImVec2 uv2 = ImVec2((float)vTileSize.x / (float)vImageSize.x, 0.0f); TileUVs.push_back(uv2);
	ImVec2 uv3 = ImVec2((2.0f * (float)vTileSize.x) / (float)vImageSize.x, (float)vTileSize.y / (float)vImageSize.y); TileUVs.push_back(uv3);

	ImVec2 uv4 = ImVec2((2.0f * (float)vTileSize.x) / (float)vImageSize.x, 0.0f); TileUVs.push_back(uv4);
	ImVec2 uv5 = ImVec2((3.0f * (float)vTileSize.x) / (float)vImageSize.x, (float)vTileSize.y / (float)vImageSize.y); TileUVs.push_back(uv5);

	// Objects
	//tv.DrawPartialDecal({ (float)vWorld_X,  (float)vWorld_Y }, dclIsom, { 0.0f, (float)5.24f * vTileSize.y }, { (float)vTileSize.x, (float)(1.24f * vTileSize.y) });
	ImVec2 uvobj0 = ImVec2(0.0f, (3.0f * (float)vTileSize.y) / (float)vImageSize.y); ObjUVs.push_back(uvobj0);
	ImVec2 uvobj1 = ImVec2(((float)vTileSize.x / 2 ) / (float)vImageSize.x, (4.0f * (float)vTileSize.y) / (float)vImageSize.y); ObjUVs.push_back(uvobj1);

	int iTempTileSelection, iTempObjectSelection;

	for (int i = 0; i < TileUVs.size(); i += 2)
	{
		ImGui::PushID(i);
		if (ImGui::ImageButton((void*)(intptr_t)dclIsom->id, size, TileUVs[i], TileUVs[i + 1]))
		{
			if (i == 0)
				iSelectedBaseTile = 1;
			else
				iSelectedBaseTile = i; // Tile enums start from 0 (empty tile)!
		}
		ImGui::PopID();
		ImGui::SameLine();
	}
	for (int i = 0; i < ObjUVs.size(); i += 2)
	{
		ImGui::PushID(i);
		if (ImGui::ImageButton((void*)(intptr_t)dclIsom->id, sizeObj, ObjUVs[i], ObjUVs[i + 1]));
		{
			if (i == 0)
				iSelectedObject = 1;
			else
				iSelectedObject = i; // Tile enums start from 0 (empty tile)!
		}
		ImGui::PopID();
		ImGui::SameLine();
	}
	if (bInWorldBounds && (GetMouse(0).bPressed))
		TileSelector(iSelectedBaseTile, iSelectedObject);
	ImGui::NewLine();
	/*for (int i = 0; i < UVs.size(); i += 2)
	{
		ImGui::Text("uvf = (%f, %f)", UVs[i].x, UVs[i].y);
		ImGui::Text("uvt = (%f, %f)", UVs[i + 1].x, UVs[1 + 1].y);
	}*/
	MainMenu();
	return true;
}

void MapEditor::MainMenu()
{
	bNewWorldCreation = false;
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("New map"))
				bNewWorldCreation = true;
			FileMenu();
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Edit"))
		{
			if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
			if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
			ImGui::Separator();
			if (ImGui::MenuItem("Cut", "CTRL+X")) {}
			if (ImGui::MenuItem("Copy", "CTRL+C")) {}
			if (ImGui::MenuItem("Paste", "CTRL+V")) {}
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

	if (bNewWorldCreation)
		ImGui::OpenPopup("Create new map");

	// Always center this window when appearing
	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

	if (ImGui::BeginPopupModal("Create new map", NULL, ImGuiWindowFlags_MenuBar))
	{
		ImGui::InputInt("Size of x axis", &iNewWorldSizeX);
		ImGui::InputInt("Size of y axis", &iNewWorldSizeY);
		ImGui::TextWrapped("Choose base tile type:");
		
		// UV naming convention: from 0 -> inf; 1st tile is uv0-uv2, 2nd uv1-uv3...
		// Vector for storing UV coordinates
		std::vector<ImVec2> UVs;
		// -1 == uses default padding (style.FramePadding)
		int frame_padding = -1;
		// Size of the image we want to make visible
		ImVec2 size = ImVec2((float)vTileSize.x, (float)vTileSize.y);

		// UV coordinates for starting pixels ([0.0,0.0] is upper-left), i.e. draw FROM
		ImVec2 uv0 = ImVec2(0.0f, 0.0f);
		// UV coordinates for tiles in our image file, i.e. draw TO 
		ImVec2 uv1 = ImVec2((float)vTileSize.x / (float)vImageSize.x, (float)vTileSize.y / (float)vImageSize.y); UVs.push_back(uv1);

		ImVec2 uv2 = ImVec2((float)vTileSize.x / (float)vImageSize.x, 0.0f); UVs.push_back(uv2);
		ImVec2 uv3 = ImVec2((2.0f * (float)vTileSize.x) / (float)vImageSize.x, (float)vTileSize.y / (float)vImageSize.y); UVs.push_back(uv3);

		ImVec2 uv4 = ImVec2((2.0f * (float)vTileSize.x) / (float)vImageSize.x, 0.0f); UVs.push_back(uv4);
		ImVec2 uv5 = ImVec2((3.0f * (float)vTileSize.x) / (float)vImageSize.x, (float)vTileSize.y / (float)vImageSize.y); UVs.push_back(uv5);

		if (ImGui::Button("Empty", size))
			iSelectedBaseTile = 0;

		for (int i = 0; i < UVs.size(); i++)
		{
			ImGui::PushID(i);
			if (i + 1 >= UVs.size())
				break;

			
			switch (i)
			{
			case 1:
				uv0 = uv1;
				break;
			case 2:
				uv0 = uv2;
				break;
			case 3:
				uv0 = uv3;
				break;
			case 4:
				uv0 = uv4;
				break;
			default:
				break;
			}

			if (ImGui::ImageButton((void*)(intptr_t)dclIsom->id, size, uv0, UVs[i]))
				iSelectedBaseTile = i + 1;
			ImGui::PopID();
			ImGui::SameLine();
		}
		ImGui::NewLine();
		if (ImGui::Button("Create"))
		{
			ImGui::CloseCurrentPopup();
			bNewWorldToCreate = true;
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
			ImGui::CloseCurrentPopup();
		ImGui::EndPopup();
	}
}

void MapEditor::FileMenu()
{
	if (ImGui::MenuItem("Open", "Ctrl+O")) { bLoadMap = true, LoadMapData(); }
	if (ImGui::BeginMenu("Open Recent"))
	{
		ImGui::MenuItem("fish_hat.c");
		ImGui::MenuItem("fish_hat.inl");
		ImGui::MenuItem("fish_hat.h");
		if (ImGui::BeginMenu("More.."))
		{
			ImGui::MenuItem("Hello");
			ImGui::MenuItem("Sailor");
			if (ImGui::BeginMenu("Recurse.."))
			{
				FileMenu();
				ImGui::EndMenu();
			}
			ImGui::EndMenu();
		}
		ImGui::EndMenu();
	}
	if (ImGui::MenuItem("Save", "Ctrl+S")) { SaveMapData(); }
}

void MapEditor::DrawUI(void) 
{
	//This finishes the Dear ImGui and renders it to the screen
	pge_imgui.ImGui_ImplPGE_Render();
}


int main()
{
	_CrtSetDbgFlag(_CRTDBG_CHECK_ALWAYS_DF);
	MapEditor demo;
	Tile tile;
	if (demo.Construct(1440, 750, 1, 1, false, true)) // Remainder for ScreenHeight() * 25 (vTileSize.y) must equal to 0, or at the edge of the screen a cell will be cut off.  
		demo.Start();
	return 0;
}
