#include <GameApp.h>
#include "Application/Application.h"
#include "Logger/Logger.h"
#include "Logger/Types/ConsoleLogger.h"
#include "Defines/Defines.h"
#include "Maths/Matrix4x4.h"

#define DllImport  __declspec( dllimport )

DllImport GameApp* GenerateGame ();

int main ()
{
	Logger::Initialize ();

	Logger::Add<ConsoleLogger> ();

	Logger::Log ( (std::string("Welcome to ") + Defines::EngineName).c_str() );

	Logger::Log ( "Logging Tests" );
	Logger::Log ( "Log" );
	Logger::Info ( "Info" );
	Logger::Warning ( "Warning" );
	Logger::Error ( "Error" );
	Logger::Fatal ( "Fatal" );

	GameApp* clientGame = GenerateGame ();

	Application* app = new Application ( clientGame );

	app->Startup ();

	clientGame->Initialize ();

	app->Run ();
	app->Destroy ();


	delete clientGame;

	Logger::Destroy ();
	return 0;
}