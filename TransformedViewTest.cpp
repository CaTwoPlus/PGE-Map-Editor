#include "olcPGEX_TransformedView.h"

// Override base class with your custom functionality
class Example : public olc::PixelGameEngine
{
public:
	Example()
	{
		// Name your application
		sAppName = "Example";
	}

public:
	olc::vi2d vTileSize = { 48, 25 };
	olc::vi2d vImageSize = { 345, 200 };
	olc::Sprite* sprIsom = nullptr;
	olc::Decal* dclIsom = nullptr;
	olc::TileTransformedView tv;
	olc::TransformedView tv1;
	
public:
	bool OnUserCreate() override
	{
		// Called once at the start, so create things here
		tv = olc::TileTransformedView({ ScreenWidth(), ScreenHeight() }, vTileSize);
		tv1 = olc::TransformedView();
		sprIsom = new olc::Sprite("./sprites/tiles.png");
		dclIsom = new olc::Decal(sprIsom);
		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		// Handle pan & zoom
		if (GetMouse(2).bPressed) tv.StartPan(GetMousePos());
		if (GetMouse(2).bHeld) tv.UpdatePan(GetMousePos());
		if (GetMouse(2).bReleased) tv.EndPan(GetMousePos());
		if (GetKey(olc::PGDN).bPressed) tv.ZoomAtScreenPos(2.0f, GetMousePos());
		if (GetKey(olc::PGUP).bPressed) tv.ZoomAtScreenPos(0.5f, GetMousePos());

		Clear(olc::VERY_DARK_BLUE);

		tv.DrawDecal({ 0,0 }, dclIsom);

		return true;
	}
};

/*
int main()
{
	Example demo;
	if (demo.Construct(256, 240, 4, 4))
		demo.Start();
	return 0;
}
*/