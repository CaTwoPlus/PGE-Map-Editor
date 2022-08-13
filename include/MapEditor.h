#ifndef MAP_EDITOR_DEF
#define MAP_EDITOR_DEF
#define OLC_GFX_OPENGL33

#define OLC_PGEX_DEAR_IMGUI_IMPLEMENTATION
#include "imgui_impl_pge.h"

#define OLC_PGEX_TRANSFORMEDVIEW
#include "olcPGEX_TransformedView.h"

#include <random>

class MapEditor : public olc::PixelGameEngine
{
	olc::imgui::PGE_ImGUI pge_imgui;
	
	// When you add new stuff, always add them after the last existing enum tile/obj type and create new vi2d tile/obj type too, update nr of existing items.
	// During rendering, objects are coming on top of tiles.
	public:
		enum TileType
		{
			TILE_TYPE_EMPTY,
			TILE_TYPE_DIRT,
			TILE_TYPE_GRASS,
			TILE_TYPE_LONG_GRASS,
			TILE_TYPE_WATER,
			TILE_TYPE_STONE,
			TILE_TYPE_NR_ITEMS = 5//should always be last 
		};

		enum Objects
		{
			OBJ_TYPE_EMPTY,
			OBJ_TYPE_BROWN_ROCK = TILE_TYPE_NR_ITEMS + 1,
			OBJ_TYPE_YELLOW_FLOWERS = TILE_TYPE_NR_ITEMS + 2,
			OBJ_TYPE_TREE_TRUNK = TILE_TYPE_NR_ITEMS + 3,
			OBJ_TYPE_TREE = TILE_TYPE_NR_ITEMS + 4,
			OBJ_TYPE_SIGNPOST = TILE_TYPE_NR_ITEMS + 5,
			OBJ_TYPE_NR_ITEMS = 4
		};

	public:
		MapEditor();
		~MapEditor();

	private:
		olc::vi2d vImageSize;
		olc::vi2d vWorldSize;
		olc::vi2d vTileSize;
		olc::vi2d vOrigin;
		olc::vi2d vTileSelectorOrigin;
		olc::vi2d vObjectSelectorOrigin;
		olc::vi2d vTileSelectorFrameX;
		olc::vi2d vObjectSelectorFrameX;
		olc::vi2d vTileSelectorFrameY;
		olc::vi2d vObjectSelectorFrameY;
		olc::vi2d vSaveBoxBoundsY;
		olc::vi2d vSaveBoxBoundsX;
		olc::vi2d vLoadBoxBoundsY;
		olc::vi2d vLoadBoxBoundsX;
		olc::vi2d vTileSelectorSize;
		olc::vi2d vSelected;
		olc::vi2d vSelectedInterfaceCell;

		olc::vi2d vPosTileTypeEmpty = { 0, 0 };
		olc::vi2d vPosTileType1;
		olc::vi2d vPosTileType2;
		olc::vi2d vPosTileType3;
		olc::vi2d vPosTileType4;
		olc::vi2d vPosTileType5;

		olc::vi2d vPosObjTypeEmpty = { 0, 0 };
		olc::vi2d vPosObjType1;
		olc::vi2d vPosObjType2;
		olc::vi2d vPosObjType3;
		olc::vi2d vPosObjType4;
		olc::vi2d vPosObjType5;

		std::string sFileData;

		olc::Sprite* sprIsom;
		olc::Decal* dclIsom;

		olc::TileTransformedView tv;
		olc::TransformedView tv1;

		std::vector<int> m_vWorld; // Tiles are the foundation
		std::vector<int> m_vWorldTemp;
		std::vector<int> m_vObjects;
		std::vector<int> m_vObjectsTemp;
		std::vector<int> m_vCellRotation;
		std::vector<int> m_vCellRotationTemp;
		int* i_pTileSelector;
		int* i_pObjectSelector;
	
		int iNewWorldSizeX;
		int iNewWorldSizeY;
		int iSelectedBaseTile;
		int iSelectedTile;
		int iSelectedObject;
		int iSelectedCells;
		int iNumberOfTiles;
		int iNumberOfObjects;
		int iTileSelectorNumberOfRows;
		int iObjectSelectorNumberOfRows;
		int iTileSelectorNumberOfColumns; 
		int iObjectSelectorNumberOfColumns;

		int iLayerEditor;
		int iLayerBackground;
		int iLayerTop;

		float fAngle;
		float fFlip_X;
		float fFlip_Y;

		bool bOpen;
		bool* b_pOpen;
		bool bInWorldBounds;
		bool bInTileSelectorBounds;
		bool bInObjectSelectorBounds;
		bool bLoadMap;
		bool bLeftMouseClicked;
		bool bIsMapLoaded;
		bool bBrushSizeIncr;
		bool bBrushSizeDecr;
		bool bFlipped;
		bool bNewWorldCreation;

	public:

		void TileSelector(int vCellX, int vCellY);
		void DrawFlippedDecal(int WorldSizeIndex_X, int WorldSizeIndex_Y, int32_t vWorld_X, int32_t vWorld_Y, int32_t vCenter_X, int32_t vCenter_Y, float fAngle, float fFlip_X, float fFlip_Y);
		void SaveMapData();
		bool LoadMapData();
		bool OnUserCreate() override;
		bool OnUserUpdate(float fElapsedTime) override;

		void DrawUI(void);
		void MainMenu();
		void FileMenu();
};

#endif