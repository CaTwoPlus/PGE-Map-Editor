#define OLC_PGE_APPLICATION
#define OLC_GFX_OPENGL33

#define OLC_PGEX_DEAR_IMGUI_IMPLEMENTATION
#include "imgui_impl_pge.h"

#define OLC_PGEX_TRANSFORMEDVIEW
#include "olcPGEX_TransformedView.h"

#include <random>
#include <Windows.h>
#include <shobjidl.h> 
#include <numeric>
#include "game_map.h"
#include "config.h"
#include "room.h"
#include "util.h"

class MapEditor : public olc::PixelGameEngine
{
	olc::imgui::PGE_ImGUI pge_imgui;

	public:
		MapEditor() : pge_imgui(true)
		{
			sAppName = "Example";
			vWorldSize = { 5, 5 };
			iNewWorldSizeX = vWorldSize.x, iNewWorldSizeY = vWorldSize.y;
			vTileSize = { 48, 25 };
			iSelectedCells = 1;
			iNumberOfTiles = Tile::TILE_TYPE_NR_ITEMS;
			iLayerEditor = 0;
			iLayerTop = 2;
			iLayerBackground = 1;
			// Where to place worldmap tile (0 ; 0) on screen (in tile size steps)
			vOrigin = { 5, 1 };
			sFileData = { "./maps/test_map.csv" };
			// Sprite object that holds all imagery
			sprIsom = nullptr;
			bInWorldBounds = false;
			bLoadMap = false;
			bLeftMouseClicked = false;
			bIsMapLoaded = false;
			bBrushSizeIncr = false;
			bBrushSizeDecr = false;
			bFlipped = false;
			bDistributionPlotted = false;
			fAngle = 0.0f;
			fFlip_X = 1.0f;
			fFlip_Y = 1.0f;
			iGeneratorTile = 0;
			iTileSelectorObject = 0;
			iPMax = 0;
			iPMin = 0;
		}


	public:
		struct Tile
		{
			enum TileType
			{
				TILE_TYPE_EMPTY,
				TILE_TYPE_DIRT,
				TILE_TYPE_GRASS,
				TILE_TYPE_LONG_GRASS,
				TILE_TYPE_WATER,
				TILE_TYPE_STONE,
				TILE_TYPE_NR_ITEMS = 5 //should always be last 
			};
			olc::vi2d vSize;
			float fAngle;
			float fFlip_X;
			float fFlip_Y;
			bool bFlipped;

			struct Object {
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
				olc::vi2d vSize;
				float fAngle;
				float fFlip_X;
				float fFlip_Y;
				bool bFlipped;
			};
		};

	private:
		olc::vi2d vImageSize;
		olc::vi2d vWorldSize;
		olc::vi2d vTileSize;
		olc::vi2d vOrigin;
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

		std::vector<int> vWorld; // Tiles are the foundation
		std::vector<int> vWorldTemp;
		std::vector<int> vObjects;
		std::vector<int> vObjectsTemp;
		std::vector<int> vCellRotation;
		std::vector<int> vCellRotationTemp;
		std::vector<int> vTileSelector;
		std::vector<int> vObjectSelector;
		std::vector<int> vMapGenTiles;
		std::map<int, int> mDistribution;
		std::map<MapEditor::Tile::TileType, int*> mProbabilitySliders;
		//int* i_pTileSelector;
		//int* i_pObjectSelector;

		int iNewWorldSizeX;
		int iNewWorldSizeY;
		int iTileSelectorTile;
		int iGeneratorTile;
		int iGeneratorEmptyTile;
		int iTileSelectorObject;
		int iSelectedCells;
		int iNumberOfTiles;
		int iRandomizerNumberOfTiles;
		int iNumberOfObjects;
		int iLayerEditor;
		int iLayerBackground;
		int iLayerTop;
		int iPMax;
		int iPMin;
		int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine, int nCmdShow);

		int iPMaxSlider1 = 0;
		int iPMaxSlider2 = 0;
		int iPMaxSlider3 = 0;
		int iPMaxSlider4 = 0;
		int iPMaxSlider5 = 0;

		int* pPMaxSlider1 = &iPMaxSlider1;
		int* pPMaxSlider2 = &iPMaxSlider2;
		int* pPMaxSlider3 = &iPMaxSlider3;
		int* pPMaxSlider4 = &iPMaxSlider4;
		int* pPMaxSlider5 = &iPMaxSlider5;

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
		bool bNewWorldToCreate;
		bool bDistributionPlotted;

	public:
		GameMap* game_map;

	public:
		void TileSelector(int iTileSelectorTile, int iTileSelectorObject)
		{
			vTileSelector.clear();
			switch (iTileSelectorTile)
			{
			case 0:
				vTileSelector.push_back(Tile::TILE_TYPE_EMPTY);
				break;
			case 1:
				vTileSelector.push_back(Tile::TILE_TYPE_DIRT);
				break;
			case 2:
				vTileSelector.push_back(Tile::TILE_TYPE_GRASS);
				break;
			case 3:
				vTileSelector.push_back(Tile::TILE_TYPE_LONG_GRASS);
				break;
			case 4:
				vTileSelector.push_back(Tile::TILE_TYPE_WATER);
				break;
			case 5:
				vTileSelector.push_back(Tile::TILE_TYPE_STONE);
				break;
			default:
				break;
			}

			vObjectSelector.clear();
			switch (iTileSelectorObject)
			{
			case 0:
				vObjectSelector.push_back(Tile::Object::OBJ_TYPE_EMPTY);
				break;
			case 1:
				vObjectSelector.push_back(Tile::Object::OBJ_TYPE_BROWN_ROCK);
				break;
			case 2:
				vObjectSelector.push_back(Tile::Object::OBJ_TYPE_YELLOW_FLOWERS);
				break;
			case 3:
				vObjectSelector.push_back(Tile::Object::OBJ_TYPE_TREE_TRUNK);
				break;
			case 4:
				vObjectSelector.push_back(Tile::Object::OBJ_TYPE_TREE);
				break;
			case 5:
				vObjectSelector.push_back(Tile::Object::OBJ_TYPE_SIGNPOST);
				break;
			default:
				break;
			}
		}

		void SaveMapData()
		{
			std::ofstream MapData;
			MapData.open(sFileData);
			MapData << vWorldSize.x << "\n" << vWorldSize.y << "\n";
			for (int i = 0; i < vWorldSize.x * vWorldSize.y; i++)
				MapData << vWorld[i] << "\n" << vObjects[i] << "\n";
			MapData.close();
		}

		bool LoadMapData()
		{
			std::ifstream MapData(sFileData, std::ios::in); // add std::ios:binary if map data contains info whether tile is solid or not
			if (MapData.is_open())
			{
				MapData >> vWorldSize.x >> vWorldSize.y;
				vWorld.resize((long long)vWorldSize.x * vWorldSize.y);
				vObjects.resize((long long)vWorldSize.x * vWorldSize.y);
				vCellRotation.resize((long long)vWorldSize.x * vWorldSize.y);
				while (!MapData.eof())
				{
					int tempwrld, tempobject, temprotation;
					MapData >> tempwrld >> tempobject >> temprotation;
					vWorld.push_back(tempwrld), vObjects.push_back(tempobject), vCellRotation.push_back(temprotation);
				}
				return true;
			}
			return false;
		}

		void DrawFlippedDecal(int WorldSizeIndex_X, int WorldSizeIndex_Y, int32_t vWorld_X, int32_t vWorld_Y, int32_t vCenter_X, int32_t vCenter_Y, float fAngle, float fFlip_X, float fFlip_Y)
		{
			switch (vObjects[WorldSizeIndex_Y * (long long)vWorldSize.x + WorldSizeIndex_X])
			{
				SetDrawTarget(iLayerTop);
				case Tile::Object::OBJ_TYPE_BROWN_ROCK:
					switch (vCellRotation[WorldSizeIndex_Y * (long long)vWorldSize.x + WorldSizeIndex_X])
					{
					case 0:
						//DrawPartialSprite(vWorld_X + (0.25 * vTileSize.x), vWorld.y - (0.25 * vTileSize.y), sprIsom, 0, 3 * vTileSize.y, 0.5 * vTileSize.x, vTileSize.y, 0, fScale_X);
						tv.DrawPartialDecal({ (float)vWorld_X,  (float)vWorld_Y }, dclIsom, { 0.0f, (float)5.24f * vTileSize.y }, { (float)vTileSize.x, (float)(1.24f * vTileSize.y) });
						break;

					case 1:
						fFlip_X = -0.9f;
						tv.DrawPartialDecal({ (float)vWorld_X + (float)(0.25 * vTileSize.x),  (float)vWorld_Y - (float)(0.25 * vTileSize.y) }, dclIsom,
							{ 0.0f, (float)5.0f * vTileSize.y }, { (float)vTileSize.x, (float)(1.24f * vTileSize.y) }, { fFlip_X, fFlip_Y });
						break;
					}
					break;
				case Tile::Object::OBJ_TYPE_YELLOW_FLOWERS:
					switch (vCellRotation[WorldSizeIndex_Y * (long long)vWorldSize.x + WorldSizeIndex_X])
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
				case Tile::Object::OBJ_TYPE_TREE_TRUNK:
					switch (vCellRotation[WorldSizeIndex_Y * (long long)vWorldSize.x + WorldSizeIndex_X])
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
				case Tile::Object::OBJ_TYPE_TREE:
					tv.DrawPartialDecal({ (float)vWorld_X, (float)vWorld_Y - ((float)2 * vTileSize.y) }, dclIsom,
						{ (float)2 * vTileSize.x, (float)vTileSize.y }, { (float)vTileSize.x, (float)3 * vTileSize.y });
					break;
				default:
					break;
			}

			switch (vWorld[WorldSizeIndex_Y * (long long)vWorldSize.x + WorldSizeIndex_X])
			{
					SetDrawTarget(iLayerBackground);
				case Tile::TILE_TYPE_DIRT:
					tv.DrawPartialDecal({ (float)vWorld_X, (float)vWorld_Y }, dclIsom,
						{ 0, 0 }, { (float)vTileSize.x, (float)vTileSize.y });
					break;
				case Tile::TILE_TYPE_GRASS:
					tv.DrawPartialDecal({ (float)vWorld_X, (float)vWorld_Y }, dclIsom,
						{ (float)vTileSize.x, 0 }, { (float)vTileSize.x, (float)vTileSize.y });
					break;
				case Tile::TILE_TYPE_LONG_GRASS:
					tv.DrawPartialDecal({ (float)vWorld_X, (float)vWorld_Y }, dclIsom,
						{ (float)vTileSize.x, (float)2 * vTileSize.y }, { (float)vTileSize.x, (float)vTileSize.y });
					break;
				case Tile::TILE_TYPE_WATER:
					tv.DrawPartialDecal({ (float)vWorld_X, (float)vWorld_Y }, dclIsom,
						{ 0, (float)vTileSize.y }, { (float)vTileSize.x, (float)vTileSize.y });
					break;
				case Tile::TILE_TYPE_STONE:
					tv.DrawPartialDecal({ (float)vWorld_X, (float)vWorld_Y }, dclIsom,
						{ (float)2 * vTileSize.x, 0 }, { (float)vTileSize.x, (float)vTileSize.y });
					break;
				default:
					break;
			}
		}

		void RandomizeCells() {
			if (!bDistributionPlotted)
				RandomizeTile();

			for (int i = 0; i < vWorld.size(); ++i) {
				if (vWorld[i] == 0)
					vWorld[i] = iGeneratorEmptyTile;
				else {
					findPMax();
					if (iPMax != iPMin) {
						vWorld[i] = equalizeDistribution();
					} else {
						RandomizeTile();
						vWorld[i] = equalizeDistribution();
					}
				}
			}
		}

		void findPMax() {
			auto maxElement = std::max_element(mDistribution.begin(), mDistribution.end(),
				[](const std::pair<int, int>& a, const std::pair<int, int>& b) {
					return a.second < b.second;
				});
			if (maxElement != mDistribution.end()) {
				iPMax = maxElement->second;
			}
		}

		void findPMin() {
			auto minElement = std::min_element(mDistribution.begin(), mDistribution.end(),
				[](const std::pair<int, int>& a, const std::pair<int, int>& b) {
					return a.second < b.second;
				});
			if (minElement != mDistribution.end()) {
				iPMin = minElement->second;
			}
		}

		// nRolls: number of experiments - higher number for more precision
		// nChance number of "chances" for a tile to appear
		void RandomizeTile(int nRolls = 10000, int nChance = 100, double variability = 4.0) {  
			const int nNumberOfTiles = vMapGenTiles.size(); // how many tile types to distribute

			// Determine the range of vMapGenTiles
			int minTile = *std::min_element(vMapGenTiles.begin(), vMapGenTiles.end());
			int maxTile = *std::max_element(vMapGenTiles.begin(), vMapGenTiles.end());

			// Adjust mean and standard deviation for normal distribution
			double mean = (minTile + maxTile) / 2.0;
			double sigma = std::max((maxTile - minTile) / variability, 0.1);

			std::default_random_engine generator(std::random_device{}());
			std::normal_distribution<double> distribution(mean, sigma); // mean, sigma

			// Initialize map with keys from vMapGenTiles and values set to 0
			for (const int& tile : vMapGenTiles) {
				mDistribution[tile] = 0;
			}

			// Populate the map based on random number generation
			for (int i = 0; i < nRolls; ++i) {
				double number = distribution(generator);
				int key = std::round(number); // Use round to get nearest integer
				if (key >= minTile && key <= maxTile) {
					if (mDistribution.find(key) != mDistribution.end()) {
						++mDistribution[key];
					}
				}
			}

			// Normalize the values in the map
			for (auto& pair : mDistribution) {
				pair.second = pair.second * nChance / nRolls;
			}

			findPMax();
			findPMin();
		}

		int equalizeDistribution() {
			int tile_type = -1;
			for (auto& p : mDistribution) {
				if (p.second == iPMax) {
					tile_type = p.first;
					--p.second;
					break;
				}
			}
			if (tile_type != -1) {
				findPMax();
				findPMin();
			}
			return tile_type;
		}

		std::vector<int> convert2DTo1D(const std::vector<std::vector<char>>& input) {
			std::vector<int> output;

			for (const auto& row : input) {
				for (char character : row) {
					int value = character - '0';
					output.push_back(value);
				}
			}

			return output;
		}

		std::vector<std::vector<char>> convert1DTo2D(const std::vector<int>& input, int rows, int cols) {
			std::vector<std::vector<char>> output(rows, std::vector<char>(cols, '1'));

			for (int i = 0; i < rows * cols && i < input.size(); ++i) {
				// Map integer to character (ASCII representation)
				char character = static_cast<char>(input[i]);

				// Determine 2D indices
				int row = i / cols;
				int col = i % cols;

				// Assign character to the 2D vector
				output[row][col] = character;
			}

			return output;
		}

		void MainMenu()
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

				static int toggleTile = 0;

				ImGui::RadioButton("Tile", &toggleTile, 1);
				ImGui::SameLine();
				ImGui::RadioButton("Empty tile", &toggleTile, 0);

				// UV naming convention: from 0 -> inf; 1st tile is uv0-uv2, 2nd uv1-uv3...
				// Order is following TileType's ordering
				std::vector<ImVec2> UVs = {
					// UV coordinates for starting pixels ([0.0,0.0] is upper-left), i.e. draw FROM
					ImVec2(0.0f, 0.0f),
					// UV coordinates for tiles in our image file, i.e. draw TO 
					ImVec2((float)vTileSize.x / (float)vImageSize.x, (float)vTileSize.y / (float)vImageSize.y),

					ImVec2((float)vTileSize.x / (float)vImageSize.x, 0.0f),
					ImVec2(2.0f * (float)vTileSize.x / (float)vImageSize.x, (float)vTileSize.y / (float)vImageSize.y),

					ImVec2((float)vTileSize.x / (float)vImageSize.x, 2.0f * (float)vTileSize.y / (float)vImageSize.y),
					ImVec2(2.0f * (float)vTileSize.x / (float)vImageSize.x, 3.0f * (float)vTileSize.y / (float)vImageSize.y),

					ImVec2(0.0f, (float)vTileSize.y / (float)vImageSize.y),
					ImVec2((float)vTileSize.x / (float)vImageSize.x, 2.0f * (float)vTileSize.y / (float)vImageSize.y),

					ImVec2(2.0f * (float)vTileSize.x / (float)vImageSize.x, 0.0f),
					ImVec2(3.0f * (float)vTileSize.x / (float)vImageSize.x, (float)vTileSize.y / (float)vImageSize.y),

				};
				ImVec2 size = ImVec2((float)vTileSize.x, (float)vTileSize.y);
				if (ImGui::Button("Empty", size)) {
					if (toggleTile == 1)
						vMapGenTiles.push_back(MapEditor::Tile::TILE_TYPE_EMPTY);
					else
						iGeneratorEmptyTile = MapEditor::Tile::TILE_TYPE_EMPTY;
				}
				for (int i = 0; i < UVs.size(); i += 2)
				{
					ImGui::PushID(i);
					if (i + 1 < UVs.size() && ImGui::ImageButton((void*)(intptr_t)dclIsom->id, size, UVs[i], UVs[i + 1], -1)) {
						if (i == 0) {
							if (toggleTile == 1 && std::find(vMapGenTiles.begin(), vMapGenTiles.end(), Tile::TILE_TYPE_DIRT) == vMapGenTiles.end())
								vMapGenTiles.push_back(Tile::TILE_TYPE_DIRT);
							else
								iGeneratorEmptyTile = Tile::TILE_TYPE_DIRT;
						}
						else {
							switch (i / 2)
							{
							case 1:
								if (toggleTile == 1 && std::find(vMapGenTiles.begin(), vMapGenTiles.end(), Tile::TILE_TYPE_GRASS) == vMapGenTiles.end())
									vMapGenTiles.push_back(Tile::TILE_TYPE_GRASS);
								else
									iGeneratorEmptyTile = Tile::TILE_TYPE_GRASS;
								break;
							case 2:
								if (toggleTile == 1 && std::find(vMapGenTiles.begin(), vMapGenTiles.end(), Tile::TILE_TYPE_LONG_GRASS) == vMapGenTiles.end())
									vMapGenTiles.push_back(Tile::TILE_TYPE_LONG_GRASS);
								else
									iGeneratorEmptyTile = Tile::TILE_TYPE_LONG_GRASS;
								break;
							case 3:
								if (toggleTile == 1 && std::find(vMapGenTiles.begin(), vMapGenTiles.end(), Tile::TILE_TYPE_WATER) == vMapGenTiles.end())
									vMapGenTiles.push_back(Tile::TILE_TYPE_WATER);
								else
									iGeneratorEmptyTile = Tile::TILE_TYPE_WATER;
								break;
							case 4:
								if (toggleTile == 1 && std::find(vMapGenTiles.begin(), vMapGenTiles.end(), Tile::TILE_TYPE_STONE) == vMapGenTiles.end())
									vMapGenTiles.push_back(Tile::TILE_TYPE_STONE);
								else
									iGeneratorEmptyTile = Tile::TILE_TYPE_STONE;
								break;
							default:
								break;
							}
						}
					}
					ImGui::PopID();
					ImGui::SameLine();
				}
				ImGui::NewLine();

				// Counter for initial distribution value assigment to sliders
				static int j = 0;

				if (ImGui::Button("Plot distribution") && vMapGenTiles.size() > 0) {
					bDistributionPlotted = true;
					if (j != 0)
						j = 0;
					RandomizeTile();
				}
				if (bDistributionPlotted) {
					ImGui::NewLine();

					int i = 0;
					float* values = new float[mDistribution.size()];

					for (auto const& pair : mDistribution) {
						values[i] = static_cast<float>(pair.second);
						++i;
					}

					ImGui::PlotHistogram("", values, mDistribution.size(), 0, NULL, (float)iPMin, (float)iPMax, ImVec2(0, 80.0f));

					// Lambda function for ceiling the sum of tile occourence rates
					auto adjustSliders = [&]() {
						int sum = *pPMaxSlider1 + *pPMaxSlider2 + *pPMaxSlider3 + *pPMaxSlider4 + *pPMaxSlider5;
						if (sum > 100) {
							float scaleFactor = 100.0f / sum;
							*pPMaxSlider1 = static_cast<int>(*pPMaxSlider1 * scaleFactor);
							*pPMaxSlider2 = static_cast<int>(*pPMaxSlider2 * scaleFactor);
							*pPMaxSlider3 = static_cast<int>(*pPMaxSlider3 * scaleFactor);
							*pPMaxSlider4 = static_cast<int>(*pPMaxSlider4 * scaleFactor);
							*pPMaxSlider5 = static_cast<int>(*pPMaxSlider5 * scaleFactor);
							return true;
						}
						return false;
					};

					for (auto& pair : mDistribution) {
						if (pair.first == Tile::TILE_TYPE_DIRT) {
							if (j == 0) { *pPMaxSlider1 = pair.second; }
							if (ImGui::SliderInt("Dirst", pPMaxSlider1, 0, 100)) {
								pair.second = *pPMaxSlider1;
							}
						}
						if (pair.first == Tile::TILE_TYPE_GRASS) {
							if (j == 0) { *pPMaxSlider2 = pair.second; }
							if (ImGui::SliderInt("Grass", pPMaxSlider2, 0, 100)) {
								pair.second = *pPMaxSlider2;
							}
						}
						if (pair.first == Tile::TILE_TYPE_LONG_GRASS) {
							if (j == 0) { *pPMaxSlider3 = pair.second; }
							if (ImGui::SliderInt("Long grass", pPMaxSlider3, 0, 100)) {
								pair.second = *pPMaxSlider3;
							}
						}
						if (pair.first == Tile::TILE_TYPE_WATER) {
							if (j == 0) { *pPMaxSlider4 = pair.second; }
							if (ImGui::SliderInt("Water", pPMaxSlider4, 0, 100)) {
								pair.second = *pPMaxSlider4;
							}
						}
						if (pair.first == Tile::TILE_TYPE_STONE) {
							if (j == 0) { *pPMaxSlider5 = pair.second; }
							if (ImGui::SliderInt("Stone", pPMaxSlider5, 0, 100)) {
								pair.second = *pPMaxSlider5;
							}
						}
					}
					adjustSliders();

					// After adjusting the sliders, update the map values
					for (auto& pair : mDistribution) {
						switch (pair.first) {
						case Tile::TILE_TYPE_DIRT:
							pair.second = *pPMaxSlider1;
							break;
						case Tile::TILE_TYPE_GRASS:
							pair.second = *pPMaxSlider2;
							break;
						case Tile::TILE_TYPE_LONG_GRASS:
							pair.second = *pPMaxSlider3;
							break;
						case Tile::TILE_TYPE_WATER:
							pair.second = *pPMaxSlider4;
							break;
						case Tile::TILE_TYPE_STONE:
							pair.second = *pPMaxSlider5;
							break;
						}
					}

					++j;
				}
				ImGui::NewLine();
				if (ImGui::Button("Create"))
				{
					ImGui::CloseCurrentPopup();
					bNewWorldToCreate = true;
				}
				ImGui::SameLine();
				if (ImGui::Button("Cancel")) {
					iGeneratorEmptyTile = 0;
					iPMaxSlider1 = 0;
					iPMaxSlider2 = 0;
					iPMaxSlider3 = 0;
					iPMaxSlider4 = 0;
					iPMaxSlider5 = 0;
					vMapGenTiles.erase(vMapGenTiles.begin(), vMapGenTiles.end());
					mDistribution.erase(mDistribution.begin(), mDistribution.end());
					bDistributionPlotted = false;
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}
		}

		void FileMenu()
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
		
		void DrawUI(void)
		{
			//This finishes the Dear ImGui and renders it to the screen
			pge_imgui.ImGui_ImplPGE_Render();
		}

	public:
		bool OnUserCreate() override
		{
			tv = olc::TileTransformedView({ ScreenWidth(), ScreenHeight() }, { 1, 1 });

			// Load sprites 
			sprIsom = new olc::Sprite("./sprites/tiles.png");

			// Creates decals, so sprites are loaded onto the GPU for better performance
			dclIsom = new olc::Decal(sprIsom);
			// Source image size - Width * Height
			vImageSize = { 345, 200 };

			// Create empty world
			vWorld.resize((long long)vWorldSize.x * vWorldSize.y);
			vObjects.resize((long long)vWorldSize.x * vWorldSize.y);
			// Create array to store rotational state of cells 
			vCellRotation.resize((long long)vWorldSize.x * vWorldSize.y);

			// For ImGui
			bOpen = true;

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

			return true;
		}

		bool OnUserUpdate(float fElapsedTime) override
		{
			// called once per frame
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
			olc::vi2d vOffset = { vMouse.x % vTileSize.x, vMouse.y % vTileSize.x };

			// Is selected tile within world space
			// Itt tartottam - currently inaccurate 
			if (vSelected.x >= 0 && vSelected.x < vWorldSize.x && vSelected.y >= 0 && vSelected.y < vWorldSize.y)
				bInWorldBounds = true;
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
					(vOrigin.x* vTileSize.x) + (x - y) * (vTileSize.x / 2), // + (x - y) * (vTileSize.x / 2) is screenspace to worldspace offset
						(vOrigin.y* vTileSize.y) + (x + y) * (vTileSize.y / 2)
				};
			};

			// Change world map size on key press
			/*
			if (GetKey(olc::UP).bPressed)
			{
				int iCurrentTile = vWorld[((long long)vWorldSize.x * vWorldSize.y) - 1], iCurrentRotation = vCellRotation[((long long)vWorldSize.x * vWorldSize.y) - 1];
				++vWorldSize.y;
				for (int i = 0; i < vWorldSize.x; i++) vWorld.push_back(iCurrentTile), vObjects.push_back(0), vCellRotation.push_back(iCurrentRotation);
			}
			if (GetKey(olc::DOWN).bPressed && (vWorldSize.y > 1))
			{
				int iCurrentTile = vWorld[((long long)vWorldSize.x * vWorldSize.y) - 1], iCurrentRotation = vCellRotation[((long long)vWorldSize.x * vWorldSize.y) - 1];
				--vWorldSize.y;
				for (int i = 0; i < vWorldSize.x; i++) vWorld.push_back(iCurrentTile), vObjects.push_back(0), vCellRotation.push_back(iCurrentRotation);
			}
			if (GetKey(olc::RIGHT).bPressed)
			{
				int iCurrentTile = vWorld[((long long)vWorldSize.x * vWorldSize.y) - 1], iCurrentRotation = vCellRotation[((long long)vWorldSize.x * vWorldSize.y) - 1];
				++vWorldSize.x;
				for (int i = 0; i < vWorldSize.y; i++) vWorld.push_back(iCurrentTile), vObjects.push_back(0), vCellRotation.push_back(iCurrentRotation);
			}
			if (GetKey(olc::LEFT).bPressed && (vWorldSize.x > 1))
			{
				int iCurrentTile = vWorld[((long long)vWorldSize.x * vWorldSize.y) - 1], iCurrentRotation = vCellRotation[((long long)vWorldSize.x * vWorldSize.y) - 1];
				--vWorldSize.x;
				for (int i = 0; i < vWorldSize.y; i++) vWorld.push_back(iCurrentTile), vObjects.push_back(0), vCellRotation.push_back(iCurrentRotation);
			}*/

			// Draw World - has binary transperancy so enable masking
			olc::PixelGameEngine::SetPixelMode(olc::Pixel::MASK);

			// Genereate new map
			if (bNewWorldToCreate)
			{
				vWorldSize.x = iNewWorldSizeX, vWorldSize.y = iNewWorldSizeY; 
				vObjects.resize((long long)vWorldSize.x * vWorldSize.y), vWorld.resize((long long)vWorldSize.x* vWorldSize.y), vCellRotation.resize((long long)vWorldSize.x* vWorldSize.y);
				// Current world vector is 1D, but map generator works with 2 dimensions
				std::vector<std::vector<char>> convertedMap = convert1DTo2D(vWorld, vWorldSize.x, vWorldSize.y);
				game_map = new GameMap((int)vWorldSize.x, (int)vWorldSize.y, kMapFillPercentage, (int)vTileSize.x, (int)vTileSize.y, convertedMap);
				game_map->ProcessMap();
				vWorld = convert2DTo1D(game_map->char_map_);
				RandomizeCells();
				//vWorld.assign((long long)vWorldSize.x* vWorldSize.y, iTileSelectorTile);
				bNewWorldToCreate = false, bIsMapLoaded = true, bLoadMap = false, iTileSelectorTile = 0, iTileSelectorObject = 0;
			}

			// Main for loop for tile rendering 
			// (0,0) is at top, defined by vOrigin, so draw from top to bottom
			// to ensure tiles closest to camera are drawn last
			if (bIsMapLoaded || !bNewWorldToCreate)
			{
				for (int y = 0; y < vWorldSize.y; y++)
				{
					for (int x = 0; x < vWorldSize.x; x++)
					{
						// Convert cell coordinate to world space
						olc::vi2d vWorld = ToScreen(x, y);
						/*if (map.bFoliageExists)
						{
							vWorld[(int)x * (int)y] = map.iTileType;
						}*/
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
				if (vSelected.x >= 0 && vSelected.x < vWorldSize.x && vSelected.y >= 0 && vSelected.y < vWorldSize.y 
					&& (olc::PixelGameEngine::GetMouse(0).bPressed || olc::PixelGameEngine::GetMouse(0).bHeld))
				{
					if (vObjectSelector.size() > 0)
						vObjects[vSelected.y * vWorldSize.x + vSelected.x] = vObjectSelector[0];
					if (vTileSelector.size() > 0) {
						vWorld[vSelected.y * vWorldSize.x + vSelected.x] = vTileSelector[0];
						vCellRotation[vSelected.y * vWorldSize.x + vSelected.x] = bFlipped;
					}
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
			DrawString(4, 74, "iTileSelectorTile: " + std::to_string(iTileSelectorTile) + ", iTileSelectorObject: " + std::to_string(iTileSelectorObject), olc::BLACK);
			DrawString(4, 84, "vSelectedWorld: " + std::to_string(vSelectedWorld.x) + ";" + std::to_string(vSelectedWorld.y), olc::BLACK);
			DrawString(4, 94, "vOrigin: " + std::to_string(vOrigin.x) + ";" + std::to_string(vOrigin.y), olc::BLACK);
			//DrawString(4, 104, "vNewOrigin: " + std::to_string(vNewOrigin.x) + ";" + std::to_string(vNewOrigin.y), olc::BLACK);

		// O------------------------------------------------------------------------------O
		// | Interface																	  |
		// O------------------------------------------------------------------------------O

			ImGui::ShowDemoWindow();
			// For debugging
			//ImGui::ShowStackToolWindow();
			if (ImGui::Begin("Tile selector", &bOpen, ImGuiWindowFlags_AlwaysAutoResize)) {
				// -1 == uses default padding (style.FramePadding)
				int frame_padding = -1;
				// Size of the image we want to make visible
				ImVec2 size = ImVec2((float)vTileSize.x, (float)vTileSize.y);
				ImVec2 sizeObj = ImVec2((float)vTileSize.x / 2.0f, (float)vTileSize.y);

				// Vectors for storing UV coordinates
				// Tiles and object selector UI
				// Tiles 
				std::vector<ImVec2> TileUVs = {
					// UV coordinates for starting pixels ([0.0,0.0] is upper-left), i.e. draw FROM
					ImVec2(0.0f, 0.0f),
					// UV coordinates for tiles in our image file, i.e. draw TO 
					ImVec2((float)vTileSize.x / (float)vImageSize.x, (float)vTileSize.y / (float)vImageSize.y),
					ImVec2((float)vTileSize.x / (float)vImageSize.x, 0.0f),
					ImVec2((2.0f * (float)vTileSize.x) / (float)vImageSize.x, (float)vTileSize.y / (float)vImageSize.y),
					ImVec2((2.0f * (float)vTileSize.x) / (float)vImageSize.x, 0.0f),
					ImVec2((3.0f * (float)vTileSize.x) / (float)vImageSize.x, (float)vTileSize.y / (float)vImageSize.y),
				};

				// Objects
				std::vector<ImVec2> ObjUVs = {
					//tv.DrawPartialDecal({ (float)vWorld_X,  (float)vWorld_Y }, dclIsom, { 0.0f, (float)5.24f * vTileSize.y }, { (float)vTileSize.x, (float)(1.24f * vTileSize.y) });
					ImVec2(0.0f, (3.0f * (float)vTileSize.y) / (float)vImageSize.y),
					ImVec2(((float)vTileSize.x / 2) / (float)vImageSize.x, (4.0f * (float)vTileSize.y) / (float)vImageSize.y)
				};

				int iTempTileSelection, iTempObjectSelection;

				for (int i = 0; i < TileUVs.size(); i += 2)
				{
					if (i + 1 >= TileUVs.size())
						break;
					ImGui::PushID(i);
					if (ImGui::ImageButton((void*)(intptr_t)dclIsom->id, size, TileUVs[i], TileUVs[i + 1]))
					{
						if (i == 0)
							iTileSelectorTile = 1;
						else
							iTileSelectorTile = i; // Tile enums start from 0 (empty tile)!
					}
					ImGui::PopID();
					ImGui::SameLine();
				}
				for (int i = 0; i < ObjUVs.size(); i += 2)
				{
					if (i + 1 >= ObjUVs.size())
						break;
					ImGui::PushID(i);
					if (ImGui::ImageButton((void*)(intptr_t)dclIsom->id, sizeObj, ObjUVs[i], ObjUVs[i + 1]));
					{
						if (i == 0)
							iTileSelectorObject = 1;
						else
							iTileSelectorObject = i; // Tile enums start from 0 (empty tile)!
					}
					ImGui::PopID();
					ImGui::SameLine();
				}
				if (bInWorldBounds && (GetMouse(0).bPressed))
					TileSelector(iTileSelectorTile, iTileSelectorObject);
				ImGui::NewLine();
				/*for (int i = 0; i < UVs.size(); i += 2)
				{
					ImGui::Text("uvf = (%f, %f)", UVs[i].x, UVs[i].y);
					ImGui::Text("uvt = (%f, %f)", UVs[i + 1].x, UVs[1 + 1].y);
				}*/
				MainMenu();
				ImGui::End();
			}
			return true;
		}
};

int main()
{
	MapEditor demo;
	if (demo.Construct(1440, 750, 1, 1, 0, 1))
		demo.Start();

	return 0;
}
