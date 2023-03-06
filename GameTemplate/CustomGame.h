#pragma once
#include <GameApp.h>

class __declspec(dllexport)  CustomGame : public GameApp
{
	std::string GetName () const override
	{
		return "CustomGame";
	}

	// Inherited via GameApp
	virtual void Initialize () const override;
	virtual void OnUpdate ( float deltaTime ) override;
	virtual void OnRender () override;
};


__declspec(dllexport) GameApp* GenerateGame ()
{
	CustomGame* g = new CustomGame();
	g->gameStartup.height = 500;
	g->gameStartup.width = 500;
	g->gameStartup.x = 250;
	g->gameStartup.y = 300;

	return g;
}
