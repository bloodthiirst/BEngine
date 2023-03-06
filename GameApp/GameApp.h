#pragma once
#include <string>

struct __declspec(dllexport) GameStartup
{
public:
	int width;
	int height;
	int x;
	int y;
};


struct __declspec(dllexport) GameState
{
public:
	bool isRunning;

};

class __declspec(dllexport) GameApp
{
public:
	virtual void Initialize () const = 0;
	virtual void OnUpdate ( float deltaTime ) = 0;
	virtual void OnRender () = 0;

	virtual std::string GetName () const = 0;
	GameStartup gameStartup = { 0 };
	GameState gameState = { 0 };
};