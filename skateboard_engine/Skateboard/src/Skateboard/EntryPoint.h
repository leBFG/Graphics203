#pragma once
// The goal of this file is to have the engine handle the entry point for the client
// This is mainly because of the different platforms
// We can ignore the errors (if any)

namespace Skateboard
{
	class Application;
}

extern Skateboard::Application* Skateboard::CreateApplication(int argc, char** argv);

int main(int argc, char** argv)
{
	//Create Platform
	Skateboard::Platform::GetPlatform();

	std::unique_ptr<Skateboard::Application> app(Skateboard::CreateApplication(argc, argv));
	app->Run();
	return 0;
}