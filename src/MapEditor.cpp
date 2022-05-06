#define OLC_PGE_APPLICATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "MapEditor.h"
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
	iTileSelectorNumberOfRows = 3; // has to change these value manually 
	iObjectSelectorNumberOfColumns = 5; //
	iTileSelectorNumberOfColumns = 10; // 
	iObjectSelectorNumberOfRows = 3;

	//iTileSelectorNumberOfColumns = (1 + ((iNumberOfTiles - 1) / iTileSelectorNumberOfRows) / iTileSelectorNumberOfColumns); // for rounding up
	iLayerEditor = 0;
	iLayerTop = 2;
	iLayerBackground = 1;

	// Where to place worldmap tile (0 ; 0) on screen (in tile size steps)
	vOrigin = { 5, 1 };
	// Size of tile selector, just for initialization (it will change later on)
	vTileSelectorSize = { 0, 0 };
	// Location of test map 
	sFileData = { "C:/Users/ReBorn/source/repos/MapEditor/MapEditor/maps/test_map.csv" };
	// Sprite object that holds all imagery
	sprIsom = nullptr;
	// Pointer to create 2D world array
	//m_vWorld = nullptr;
	//m_vObjects = nullptr;
	// For storing 2D selector arrays
	i_pTileSelector = nullptr;
	i_pObjectSelector = nullptr;
	// Flag for wrldspace bounds checking 
	bInWorldBounds = false;
	// Flag for selector interface bounds checking 
	bInTileSelectorBounds = false;
	bInObjectSelectorBounds = false;
	// Flag for save box bounds checking 
	bInSaveBoxBounds = false;
	bInLoadBoxBounds = false;
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
	delete[] i_pTileSelector;
	delete[] i_pObjectSelector;
}

bool MapEditor::OnUserCreate()
{
	// Load sprites 
	sprIsom = new olc::Sprite("./sprites/tiles.png");

	// Creates decals, so sprites are loaded onto the GPU for better performance
	dclIsom = new olc::Decal(sprIsom);
	// Source image size - Width * Height
	vImageSize = { 345, 200 };

	// Create empty world
	m_vWorld.resize((long long)vWorldSize.x * vWorldSize.y), m_vWorld = { 0 };
	m_vObjects = m_vWorld;

	// Create empty selector worlds
	i_pTileSelector = new int[(vTileSelectorSize.x * vTileSelectorSize.y) / (vTileSize.x * vTileSize.y)]{ 0 };
	i_pObjectSelector = new int[(vTileSelectorSize.x * vTileSelectorSize.y) / (vTileSize.x * vTileSize.y)]{ 0 };

	// Create array to store rotational state of cells 
	m_vCellRotation = m_vWorld;
	
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

void MapEditor::TileSelector(int vCellX, int vCellY)
{
	// Draw contents of tile selector interface. These must be decals, as sprites cannot be shrunken down. Tried with sprites, but that just complicated things...  
	// Order of cells: From L -> R
	// Rules:	Remember to check how many columns and rows there are, as the postioning is still manual. The fisrt vCell on the screen is at (0; 0), 
	//			so vCell will be at ScreenHeight() / vTileSize.y - iTileSelectorNumberOfRows when vCell hits it on the Y axis. 
	//			On Y axis, totally there are ScreenHeight() / (vTileSize.y - 1) amount of cells. 
	//			Tile types that fill 2 cells up in height should always be put into the last column. 
	//			Tile types that are similiar in nature (grass, long grass) should be below each other, in specific column.
	//			Tile types that take up half a cell in width should take up the last available empty cells.

	//DrawRect(0 + (vTileSize.x * iTileSelectorNumberOfColumns) + vTileSize.x, ScreenHeight() - (iTileSelectorNumberOfRows * vTileSize.y),
	//vTileSize.x* iObjectSelectorNumberOfColumns, ScreenHeight(), olc::BLACK);

	// Brown rock 
	DrawPartialDecal({ static_cast<float>(vObjectSelectorFrameX.x), static_cast<float>(vObjectSelectorFrameY.x) }, dclIsom,
		{ (float)0.0f, (float)5.28f * vTileSize.y }, { (float)1.0f * vTileSize.x, (float)vTileSize.y }, { 1.0f, 1.0f });
	vPosObjType1.x = iTileSelectorNumberOfColumns + 1;
	vPosObjType1.y = ((ScreenHeight() / vTileSize.y) - 1) - (iObjectSelectorNumberOfRows - 1);
	// Yellow flowers
	DrawPartialDecal({ static_cast<float>((vObjectSelectorFrameX.x + vTileSize.x)), static_cast<float>(vObjectSelectorFrameY.x) }, dclIsom,
		{ (float)1.5f * vTileSize.x, (float)3.0f * vTileSize.y }, { (float)0.5f * vTileSize.x, (float)vTileSize.y });
	vPosObjType2.x = iTileSelectorNumberOfColumns + 2;
	vPosObjType2.y = ((ScreenHeight() / vTileSize.y) - 1) - (iObjectSelectorNumberOfRows - 1);
	// Tree trunk
	DrawPartialDecal({ static_cast<float>((vObjectSelectorFrameX.x + 2 * vTileSize.x)), static_cast<float>(vObjectSelectorFrameY.x) }, dclIsom,
		{ (float)1.0f * vTileSize.x, (float)3.0f * vTileSize.y }, { (float)0.5f * vTileSize.x, (float)vTileSize.y });
	vPosObjType3.x = iTileSelectorNumberOfColumns + 3;
	vPosObjType3.y = ((ScreenHeight() / vTileSize.y) - 1) - (iObjectSelectorNumberOfRows - 1);
	// Signpost 
	DrawPartialDecal({ static_cast<float>(vObjectSelectorFrameX.x + 3 * vTileSize.x), static_cast<float>(vObjectSelectorFrameY.x) }, dclIsom,
		{ (float)0.5f * vTileSize.x, (float)3.0f * vTileSize.y }, { (float)0.5f * vTileSize.x, (float)vTileSize.y });
	vPosObjType4.x = iTileSelectorNumberOfColumns + 4;
	vPosObjType4.y = ((ScreenHeight() / vTileSize.y) - 1) - (iObjectSelectorNumberOfRows - 1);
	// Tree  
	DrawPartialDecal({ static_cast<float>((vObjectSelectorFrameX.x + 4 * vTileSize.x)), static_cast<float>(vObjectSelectorFrameY.x) }, dclIsom,
		{ (float)2.0f * vTileSize.x, (float)vTileSize.y }, { (float)vTileSize.x, (float)3.0f * vTileSize.y }, { 1.0f, 0.7f });
	vPosObjType5.x = iTileSelectorNumberOfColumns + 5;
	vPosObjType5.y = ((ScreenHeight() / vTileSize.y) - 1) - (iObjectSelectorNumberOfRows - 1);

	// Dirt tile 
	DrawPartialDecal({ static_cast<float>(vTileSelectorFrameX.x), static_cast<float>(vTileSelectorFrameY.x) }, dclIsom,
		{ (float)0.0f, (float)0.0f }, { (float)vTileSize.x, (float)vTileSize.y });
	vPosTileType1.x = 0;
	vPosTileType1.y = ((ScreenHeight() / vTileSize.y) - 1) - (iTileSelectorNumberOfRows - 1);
	// Grass tile
	DrawPartialDecal({ static_cast<float>(vTileSelectorFrameX.x + vTileSize.x), static_cast<float>(vTileSelectorFrameY.x) }, dclIsom,
		{ (float)vTileSize.x, (float)0.0f }, { (float)vTileSize.x, (float)vTileSize.y });
	vPosTileType2.x = 1;
	vPosTileType2.y = ((ScreenHeight() / vTileSize.y) - 1) - (iTileSelectorNumberOfRows - 1);
	// Long grass
	DrawPartialDecal({ static_cast<float>(vTileSelectorFrameX.x + vTileSize.x), static_cast<float>(vTileSelectorFrameY.x + vTileSize.y) }, dclIsom,
		{ (float)vTileSize.x, (float)2.0f * vTileSize.y }, { (float)vTileSize.x, (float)vTileSize.y });
	vPosTileType3.x = 1;
	vPosTileType3.y = ((ScreenHeight() / vTileSize.y) - 1) - (iTileSelectorNumberOfRows - 2);
	// Water tile
	DrawPartialDecal({ static_cast<float>((vTileSelectorFrameX.x + 2 * vTileSize.x)), static_cast<float>(vTileSelectorFrameY.x) }, dclIsom,
		{ (float)0.0f, (float)vTileSize.y }, { (float)vTileSize.x, (float)vTileSize.y });
	vPosTileType4.x = 2;
	vPosTileType4.y = ((ScreenHeight() / vTileSize.y) - 1) - (iTileSelectorNumberOfRows - 1);
	// Stone tile
	DrawPartialDecal({ static_cast<float>((vTileSelectorFrameX.x + 3 * vTileSize.x)), static_cast<float>(vTileSelectorFrameY.x) }, dclIsom,
		{ (float)2.0f * vTileSize.x, (float)0.0f }, { (float)vTileSize.x, (float)vTileSize.y });
	vPosTileType5.x = 3;
	vPosTileType5.y = ((ScreenHeight() / vTileSize.y) - 1) - (iTileSelectorNumberOfRows - 1);

	if (GetMouse(0).bPressed && bInTileSelectorBounds == true)
	{
		++iSelectedTile;
		if (iSelectedTile > 2)
			iSelectedTile = 1;
		if (iSelectedTile == 2)
			iSelectedTile = 0, *i_pTileSelector = TILE_TYPE_EMPTY;
		if (iSelectedTile == 1)
		{
			if (vPosTileType1.x == vCellX && vPosTileType1.y == vCellY)
				*i_pTileSelector = TILE_TYPE_DIRT;
			if (vPosTileType2.x == vCellX && vPosTileType2.y == vCellY)
				*i_pTileSelector = TILE_TYPE_GRASS;
			if (vPosTileType3.x == vCellX && vPosTileType3.y == vCellY)
				*i_pTileSelector = TILE_TYPE_LONG_GRASS;
			if (vPosTileType4.x == vCellX && vPosTileType4.y == vCellY)
				*i_pTileSelector = TILE_TYPE_WATER;
			if (vPosTileType5.x == vCellX && vPosTileType5.y == vCellY)
				*i_pTileSelector = TILE_TYPE_STONE;
		}
	}

	if (GetMouse(0).bPressed && bInObjectSelectorBounds == true)
	{
		++iSelectedObject;
		if (iSelectedObject > 2)
			iSelectedObject = 1;
		if (iSelectedObject == 2)
			iSelectedObject = 0, *i_pObjectSelector = OBJ_TYPE_EMPTY;
		if (iSelectedObject == 1)
		{
			if (vPosObjType1.x == vCellX && vPosObjType1.y == vCellY)
				*i_pObjectSelector = OBJ_TYPE_BROWN_ROCK;
			if (vPosObjType2.x == vCellX && vPosObjType2.y == vCellY)
				*i_pObjectSelector = OBJ_TYPE_YELLOW_FLOWERS;
			if (vPosObjType3.x == vCellX && vPosObjType3.y == vCellY)
				*i_pObjectSelector = OBJ_TYPE_TREE_TRUNK;
			if (vPosObjType4.x == vCellX && vPosObjType4.y == vCellY)
				*i_pObjectSelector = OBJ_TYPE_SIGNPOST;
			if (vPosObjType5.x == vCellX && vPosObjType5.y == vCellY)
				*i_pObjectSelector = OBJ_TYPE_TREE;
		}
	}

	// For debugging - changed the variable (name was taken already)
	//TileSelectorCell.x = vSelectedInterfaceAreaX,
	//vTileSelectorCell.y = vSelectedInterfaceAreaY;
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
			int tempwrld, tempobject;
			MapData >> tempwrld >> tempobject;
			m_vWorld.push_back(tempwrld), m_vObjects.push_back(tempobject);
		}
		bIsMapLoaded = false;
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
			DrawPartialDecal({ (float)vWorld_X,  (float)vWorld_Y}, dclIsom, { 0.0f, (float)5.24f * vTileSize.y }, { (float)vTileSize.x, (float)(1.24f * vTileSize.y) });
			break;

		case 1:
			fFlip_X = -0.9f;
			DrawPartialDecal({ (float)vWorld_X + (float)(0.25 * vTileSize.x),  (float)vWorld_Y - (float)(0.25 * vTileSize.y) }, dclIsom,
				{ 0.0f, (float)5.0f * vTileSize.y }, { (float)vTileSize.x, (float)(1.24f * vTileSize.y) }, { fFlip_X, fFlip_Y });
			break;
		}
		break;
	case OBJ_TYPE_YELLOW_FLOWERS:
		switch (m_vCellRotation[WorldSizeIndex_Y * (long long)vWorldSize.x + WorldSizeIndex_X])
		{
		case 0:
			DrawPartialDecal({ ((float)vWorld_X + 0.25f * vTileSize.x), ((float)vWorld_Y - 0.125f * vTileSize.y) }, dclIsom, { (float)1.5f * vTileSize.x, (float)3.0f * vTileSize.y }, { (float)0.5f * vTileSize.x, (float)vTileSize.y });
			break;
		case 1:
			fFlip_X = -0.9f;
			DrawPartialDecal({ ((float)vWorld_X + 0.25f * vTileSize.x), ((float)vWorld_Y + 0.5f * vTileSize.y) }, dclIsom, { (float)1.5f * vTileSize.x, (float)3.0f * vTileSize.y }, { (float)0.5f * vTileSize.x, (float)vTileSize.y }, { fFlip_X, fFlip_Y });
			break;
		}
		break;
	case OBJ_TYPE_TREE_TRUNK:
		switch (m_vCellRotation[WorldSizeIndex_Y * (long long)vWorldSize.x + WorldSizeIndex_X])
		{
		case 0:
			DrawPartialDecal({ (float)vWorld_X, (float)vWorld_Y }, dclIsom, { (float)1.0f * vTileSize.x, (float)3.0f * vTileSize.y }, { (float)0.5f * vTileSize.x, (float)vTileSize.y });
			break;
		case 1:
			fFlip_X = -0.9f;
			DrawPartialDecal({ (float)vWorld_X, (float)vWorld_Y }, dclIsom, { (float)1.0f * vTileSize.x, (float)3.0f * vTileSize.y }, { (float)0.5f * vTileSize.x, (float)vTileSize.y }, { fFlip_X, fFlip_Y });
			break;
		}
		break;
	case OBJ_TYPE_TREE:
		DrawPartialDecal({ (float)vWorld_X, (float)vWorld_Y - ((float)2 * vTileSize.y) }, dclIsom,
			{ (float)2 * vTileSize.x, (float)vTileSize.y }, { (float)vTileSize.x, (float)3 * vTileSize.y });
		break;
	}

	switch (m_vWorld[WorldSizeIndex_Y * (long long)vWorldSize.x + WorldSizeIndex_X])
	{
	SetDrawTarget(iLayerBackground);
	case TILE_TYPE_DIRT:
		DrawPartialDecal({ (float)vWorld_X, (float)vWorld_Y }, dclIsom,
			{ 0, 0 }, { (float)vTileSize.x, (float)vTileSize.y });
		break;
	case TILE_TYPE_GRASS:
		DrawPartialDecal({ (float)vWorld_X, (float)vWorld_Y }, dclIsom,
			{ (float)vTileSize.x, 0 }, { (float)vTileSize.x, (float)vTileSize.y });
		break;
	case TILE_TYPE_LONG_GRASS:
		DrawPartialDecal({ (float)vWorld_X, (float)vWorld_Y }, dclIsom,
			{ (float)vTileSize.x, (float)2 * vTileSize.y }, { (float)vTileSize.x, (float)vTileSize.y });
		break;
	case TILE_TYPE_WATER:
		DrawPartialDecal({ (float)vWorld_X, (float)vWorld_Y }, dclIsom,
			{ 0, (float)vTileSize.y }, { (float)vTileSize.x, (float)vTileSize.y });
		break;
	case TILE_TYPE_STONE:
		DrawPartialDecal({ (float)vWorld_X, (float)vWorld_Y }, dclIsom,
			{ (float)2 * vTileSize.x, 0 }, { (float)vTileSize.x, (float)vTileSize.y });
		break;
	}
}

//Main loop//
bool MapEditor::OnUserUpdate(float fElapsedTime)
{
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

	// Handle mouse click to toggle if a tile is visible or not
	// Selection from tile selector interface is assigned here to the selected worldspace cell 
	if (olc::PixelGameEngine::GetMouse(0).bPressed || olc::PixelGameEngine::GetMouse(0).bHeld)
	{
		for (int i = 1; i <= iSelectedCells; i++) {
			for (int j = 1; j <= iSelectedCells; j++) {

				int index_x = vSelected.x + j;
				int index_y = vSelected.y + i;

				vOffset.x += index_x;
				vOffset.y += index_y;

				if ( i != 1 || j != 1 )
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

				if (index_x >= 0 && index_x < vWorldSize.x && index_y >= 0 && index_y < vWorldSize.y)
				{
					m_vObjects[index_y * (long long)vWorldSize.x + index_x] = *i_pObjectSelector;
					m_vWorld[index_y * (long long)vWorldSize.x + index_x] = *i_pTileSelector;
					m_vCellRotation[index_y * (long long)vWorldSize.x + index_x] = bFlipped;
				}

			}
		}
		/*
		if (vSelected.x >= 0 && vSelected.x < vWorldSize.x && vSelected.x >= 0 && vSelected.y < vWorldSize.y)
		{
			m_vObjects[vSelected.y * vWorldSize.x + vSelected.x] = *i_pObjectSelector;
			m_vWorld[vSelected.y * vWorldSize.x + vSelected.x] = *i_pTileSelector;
			m_vCellRotation[vSelected.y * vWorldSize.x + vSelected.x] = bFlipped;
		}*/
	}

	// Sample into cell offset colour
	//olc::Pixel col = sprIsom->GetPixel(3 * vTileSize.x + vOffset.x, vOffset.y); 
	olc::Pixel col = sprIsom->GetPixel(vOffset.x, 2 * vTileSize.y + vOffset.y); //get pixel value of Cheat_tile

// O------------------------------------------------------------------------------O
// | Interface																	  |
// O------------------------------------------------------------------------------O

	// Draw tile selector interface (black rectangle) 
	DrawRect(0, ScreenHeight() - (iTileSelectorNumberOfRows * vTileSize.y), vTileSize.x * iTileSelectorNumberOfColumns, ScreenHeight(), olc::BLACK);

	// Where to place tile selector interface tile (0 ; 0) on screen (in tile size steps). 
	vTileSelectorOrigin = { 0, (ScreenHeight() / vTileSize.y) - iTileSelectorNumberOfRows };
	// How big the frame
	vTileSelectorFrameX = { 0, vTileSize.x * iTileSelectorNumberOfColumns };
	vTileSelectorFrameY = { ScreenHeight() - (iTileSelectorNumberOfRows * vTileSize.y),  ScreenHeight() };

	// Draw object selector interface
	DrawRect(0 + (vTileSize.x * iTileSelectorNumberOfColumns) + vTileSize.x, ScreenHeight() - (iTileSelectorNumberOfRows * vTileSize.y),
		vTileSize.x * iObjectSelectorNumberOfColumns, ScreenHeight(), olc::BLACK);

	// Same for object selector interface
	vObjectSelectorOrigin = { (vTileSize.x * iTileSelectorNumberOfColumns) + vTileSize.x, ScreenHeight() - (iTileSelectorNumberOfRows * vTileSize.y) };
	vObjectSelectorFrameX = { 0 + (vTileSize.x * iTileSelectorNumberOfColumns) + vTileSize.x, vTileSize.x * iObjectSelectorNumberOfColumns };
	vObjectSelectorFrameY = { ScreenHeight() - (iTileSelectorNumberOfRows * vTileSize.y), ScreenHeight() };

	// Work out selector interface cell by transforming screen cell 
	vSelectedInterfaceCell =
	{
		(vCell.x - vTileSelectorOrigin.x),
		(vCell.y - vTileSelectorOrigin.y)
	};

	// Is selected tile within world space
	if (vSelected.x >= 0 && vSelected.x < vWorldSize.x && vSelected.y >= 0 && vSelected.y < vWorldSize.y)
		bInWorldBounds = true;
	else
		bInWorldBounds = false;

	// Are we within tile selector bounds?
	if (vMouse.x < vTileSelectorFrameX.y && vMouse.y >= vTileSelectorFrameY.x)
		bInTileSelectorBounds = true;
	else
		bInTileSelectorBounds = false;
	if (vMouse.x > vObjectSelectorFrameX.x && vMouse.x <= (vObjectSelectorFrameX.x + vObjectSelectorFrameX.y) && vMouse.y >= vObjectSelectorFrameY.x)
		bInObjectSelectorBounds = true;
	else
		bInObjectSelectorBounds = false;
	
	// Draw red rectangle within selector interface 
	if (bInTileSelectorBounds == true)
		DrawRect(vSelectedInterfaceCell.x * vTileSize.x, (vSelectedInterfaceCell.y * vTileSize.y) + (ScreenHeight() - (iTileSelectorNumberOfRows * vTileSize.y)), vTileSize.x, vTileSize.y, olc::RED);
	if (bInObjectSelectorBounds == true)
		DrawRect(vSelectedInterfaceCell.x * vTileSize.x, (vSelectedInterfaceCell.y * vTileSize.y) + (ScreenHeight() - (iObjectSelectorNumberOfRows * vTileSize.y)), vTileSize.x, vTileSize.y, olc::RED);
	if ((iSelectedTile == 1 || iSelectedObject == 1) && (bInTileSelectorBounds == true || bInObjectSelectorBounds == true))
	{
		olc::vi2d vSavevSelectedInterfaceCell = { vSelectedInterfaceCell.x,  vSelectedInterfaceCell.y };
		DrawRect(vSavevSelectedInterfaceCell.x * vTileSize.x, (vSavevSelectedInterfaceCell.y * vTileSize.y) + (ScreenHeight() - (iTileSelectorNumberOfRows * vTileSize.y)), vTileSize.x, vTileSize.y, olc::GREEN);
	}

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

	// For tile selector interface coordinate conversion 
	auto InterfaceToScreenTiles = [&](int x, int y)
	{
		return olc::vi2d	
		{
			(vTileSelectorOrigin.x * vTileSize.x) + (x - y) * (vTileSize.x / 2),
			(vTileSelectorOrigin.y * vTileSize.y) + (x + y) * (vTileSize.y / 2)
		};
	};

	// Change world map size on key press
	if (GetKey(olc::UP).bPressed)
		m_vWorld.resize((long long)vWorldSize.x * ++vWorldSize.y), m_vObjects.resize((long long)vWorldSize.x * vWorldSize.y), m_vCellRotation.resize((long long)vWorldSize.x * vWorldSize.y);
	if (GetKey(olc::DOWN).bPressed && (vWorldSize.y > 1)) 
	{
		/*m_vWorldTemp = new int[vWorldSize.x * (long long)--vWorldSize.y]{1};
		m_vObjectsTemp = new int[vWorldSize.x * (long long)vWorldSize.y]{ 0 };
		m_vCellRotationTemp = new int[vWorldSize.x * (long long)vWorldSize.y]{ 0 };
		for (int i = 0; i < vWorldSize.y * vWorldSize.x; i++)
		{
			m_vWorldTemp[i] = m_vWorld[i];
			m_vObjectsTemp[i] = m_vObjects[i];
			m_vCellRotationTemp[i] = m_vCellRotation[i];
		}
		delete[] m_vWorld;
		m_vWorld = m_vWorldTemp;
		delete[] m_vObjects;
		m_vObjects = m_vObjectsTemp;
		delete[] m_vCellRotation;
		m_vCellRotation = m_vCellRotationTemp;*/

	}
	if (GetKey(olc::RIGHT).bPressed)
	{
		/*m_vWorldTemp = new int[(long long)++vWorldSize.x * vWorldSize.y]{1};
		m_vObjectsTemp = new int[(long long)vWorldSize.x * vWorldSize.y]{ 0 };
		m_vCellRotationTemp = new int[vWorldSize.x * (long long)vWorldSize.y]{ 0 };
		for (int i = 0; i < vWorldSize.y * vWorldSize.x; i++)
		{
			m_vWorldTemp[i] = m_vWorld[i];
			m_vObjectsTemp[i] = m_vObjects[i];
			m_vCellRotationTemp[i] = m_vCellRotation[i];
		}
		delete[] m_vWorld;
		m_vWorld = m_vWorldTemp;
		delete[] m_vObjects;
		m_vObjects = m_vObjectsTemp;
		delete[] m_vCellRotation;
		m_vCellRotation = m_vCellRotationTemp;*/
	}
	if (GetKey(olc::LEFT).bPressed && (vWorldSize.x > 1)) 
	{
		/*m_vWorldTemp = new int[(long long)--vWorldSize.x * vWorldSize.y]{1};
		m_vObjectsTemp = new int[(long long)vWorldSize.x * vWorldSize.y]{ 0 };
		m_vCellRotationTemp = new int[vWorldSize.x * (long long)vWorldSize.y]{ 0 };
		for (int i = 0; i < vWorldSize.y * vWorldSize.x; i++)
		{
			m_vWorldTemp[i] = m_vWorld[i];
			m_vObjectsTemp[i] = m_vObjects[i];
			m_vCellRotationTemp[i] = m_vCellRotation[i];
		}
		delete[] m_vWorld;
		m_vWorld = m_vWorldTemp;
		delete[] m_vObjects;
		m_vObjects = m_vObjectsTemp;
		delete[] m_vCellRotation;
		m_vCellRotation = m_vCellRotationTemp;*/
	}

	// Drag the world map accross the screen 
	if (GetMouse(1).bHeld && bInWorldBounds == false && bInTileSelectorBounds == false)
		vOrigin = vCell;

	// Draw World - has binary transperancy so enable masking
	olc::PixelGameEngine::SetPixelMode(olc::Pixel::MASK);

	// Load map data in or create new map
	if (bInLoadBoxBounds == true || bNewWorldCreation == true)
	{
		if (bNewWorldCreation && (iNewWorldSizeX != vWorldSize.x || iNewWorldSizeY != vWorldSize.y))
		{
			vWorldSize.x = iNewWorldSizeX, vWorldSize.y = iNewWorldSizeY, m_vObjects.resize((long long)vWorldSize.x * vWorldSize.y);
			m_vWorld.assign((long long)vWorldSize.x* vWorldSize.y, iSelectedBaseTile);
		}
		for (int y = 0; y < vWorldSize.y; y++)
		{
			for (int x = 0; x < vWorldSize.x; x++)
			{
				// Convert cell coordinate to world space
				olc::vi2d vWorld = ToScreen(x, y);

				for (int n = 0; n < vWorldSize.x * vWorldSize.y; n++) 
				{
					switch (m_vObjects[n])
					{
					case OBJ_TYPE_BROWN_ROCK:
						DrawDecal({ (float)vWorld.x, (float)vWorld.y }, dclIsom);
						break;
					case OBJ_TYPE_YELLOW_FLOWERS:
						DrawDecal({ (float)vWorld.x, (float)vWorld.y }, dclIsom);
						break;
					case OBJ_TYPE_TREE_TRUNK:
						DrawDecal({ (float)vWorld.x, (float)vWorld.y }, dclIsom);
						break;
					case OBJ_TYPE_TREE:
						DrawDecal({ (float)vWorld.x, (float)vWorld.y }, dclIsom);
						break;
					case OBJ_TYPE_SIGNPOST:
						DrawDecal({ (float)vWorld.x, (float)vWorld.y }, dclIsom);
						break;
					}
					switch (m_vWorld[n])
					{
					case TILE_TYPE_DIRT:
						DrawDecal({ (float)vWorld.x, (float)vWorld.y }, dclIsom);
						break;
					case TILE_TYPE_GRASS:
						DrawDecal({ (float)vWorld.x, (float)vWorld.y }, dclIsom);
						break;
					case TILE_TYPE_LONG_GRASS:
						DrawDecal({ (float)vWorld.x, (float)vWorld.y }, dclIsom);
						break;
					case TILE_TYPE_WATER:
						DrawDecal({ (float)vWorld.x, (float)vWorld.y }, dclIsom);
						break;
					case TILE_TYPE_STONE:
						DrawDecal({ (float)vWorld.x, (float)vWorld.y }, dclIsom);
						break;
					}
				}
			}
		}
		bIsMapLoaded = true, bInLoadBoxBounds = false, bNewWorldCreation == false;
	}

	// Main for loop for tile rendering 
	// (0,0) is at top, defined by vOrigin, so draw from top to bottom
	// to ensure tiles closest to camera are drawn last
	if (bIsMapLoaded == true)
	{
		for (int y = 0; y < vWorldSize.y; y++)
		{

			for (int x = 0; x < vWorldSize.x; x++)
			{
				// Convert cell coordinate to world space
				olc::vi2d vWorld = ToScreen(x, y);
				DrawFlippedDecal(x, y, vWorld.x, vWorld.y, vCell.x, vCell.y, fAngle, fFlip_X, fFlip_Y);
			}
		}
	}
		
	// Draw Selected Cell - Has varying alpha components
	SetPixelMode(olc::Pixel::ALPHA);

	// Convert selected cell coordinate to world space
	olc::vi2d vSelectedWorld = ToScreen(vSelected.x, vSelected.y);
	// Convert selector interface cell coordinate to world space
	olc::vi2d vSelectedInterfaceAreaTiles = InterfaceToScreenTiles(vSelectedInterfaceCell.x, vSelectedInterfaceCell.y);

	// Draw tile selection
	TileSelector(vCell.x, vCell.y);

	// Draw "highlight" tile
	if (bInWorldBounds == true)
		//DrawPartialSprite(vSelectedWorld.x, vSelectedWorld.y, sprIsom, 1 * vTileSize.x, vTileSize.y, vTileSize.x, vTileSize.y, iSelectedCells);
		DrawPartialDecal({ (float)vSelectedWorld.x, (float)vSelectedWorld.y }, dclIsom, { (float)1 * vTileSize.x, (float)vTileSize.y }, { (float)vTileSize.x, (float)vTileSize.y }, { (float)iSelectedCells, (float)iSelectedCells });
		
	// Draw box for save function
	DrawRect(ScreenWidth() - 40, 0, 40, 20, olc::BLACK);
	DrawRect(ScreenWidth() - 40, 20, 40, 20, olc::BLACK);

	// Go back to normal drawing with no expected transparency
	SetPixelMode(olc::Pixel::NORMAL);

	// Draw write to png save string 
	DrawString(ScreenWidth() - 35, ScreenHeight() - (ScreenHeight() - 7), "Save", olc::BLACK);
	DrawString(ScreenWidth() - 35, ScreenHeight() - (ScreenHeight() - 27), "Load", olc::BLACK);

	// Draw Debug Info - '+' here is operator overloading (string concatenation) 
	DrawString(4, 4, "Mouse   : " + std::to_string(vMouse.x) + ", " + std::to_string(vMouse.y), olc::BLACK);
	DrawString(4, 14, "Cell    : " + std::to_string(vCell.x) + ", " + std::to_string(vCell.y), olc::BLACK);
	DrawString(4, 24, "Selected: " + std::to_string(vSelected.x) + ", " + std::to_string(vSelected.y), olc::BLACK);
	DrawString(4, 34, "World size: " + std::to_string(vWorldSize.x) + ", " + std::to_string(vWorldSize.y), olc::BLACK);
	DrawString(4, 44, "In world bounds: " + std::to_string(bInWorldBounds), olc::BLACK);
	DrawString(4, 64, "iSelectedTile: " + std::to_string(iSelectedTile) + ", iSelectedObject: " + std::to_string(iSelectedObject) , olc::BLACK);
	DrawString(4, 74, "vObjectSelectorOrigin: " + std::to_string(vObjectSelectorOrigin.x) + ";" + std::to_string(vObjectSelectorOrigin.y), olc::BLACK);

	// Do ImGui stuff here.
	ImGui::ShowDemoWindow();
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
		ImVec2 uv0 = ImVec2(0.0f, 0.0f); UVs.push_back(uv0);
		// UV coordinates for tiles in our image file, i.e. draw TO 
		ImVec2 uv1 = ImVec2((float)vTileSize.x / (float)vImageSize.x, (float)vTileSize.y / (float)vImageSize.y); UVs.push_back(uv1);

		ImVec2 uv2 = ImVec2((2.0f * (float)vTileSize.x) / (float)vImageSize.x, 0.0f); UVs.push_back(uv2);
		ImVec2 uv3 = ImVec2((3.0f * (float)vTileSize.x) / (float)vImageSize.x, (float)vTileSize.y / (float)vImageSize.y); UVs.push_back(uv3);

		ImVec2 uv4 = ImVec2((3.0f * (float)vTileSize.x) / (float)vImageSize.x, 0.0f); UVs.push_back(uv4);
		ImVec2 uv5 = ImVec2((4.0f * (float)vTileSize.x) / (float)vImageSize.x, (float)vTileSize.y / (float)vImageSize.y); UVs.push_back(uv5);

		for (int i = 0; i < UVs.size() / 2; i++)
		{
			ImGui::PushID(i);
			if (ImGui::ImageButton((void*)(intptr_t)dclIsom->id, size, UVs[i], UVs[i + 1]));
			if (i == 0)
				iSelectedBaseTile = 1;
			else
				iSelectedBaseTile = i + 1; // Tile enums start from 0 (empty tile)!
			ImGui::PopID();
			ImGui::SameLine();
		}
		ImGui::NewLine();
		if (ImGui::Button("Create"))
		{
			ImGui::CloseCurrentPopup();
			bNewWorldCreation = true;
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
			ImGui::CloseCurrentPopup();
		ImGui::EndPopup();
	}
}

void MapEditor::FileMenu()
{
	if (ImGui::MenuItem("Open", "Ctrl+O")) { bInLoadBoxBounds = true, LoadMapData(); }
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
	MapEditor demo;
	if (demo.Construct(1440, 750, 1, 1, 0)) // Remainder for ScreenHeight() * 25 (vTileSize.y) must equal to 0, or at the edge of the screen a cell will be cut off.  
		demo.Start();

	return 0;
}