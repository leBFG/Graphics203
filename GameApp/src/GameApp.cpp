#include <Skateboard.h>
#include "Skateboard/EntryPoint.h"

#include "DefaultGameLayer.h"

class GameApp : public Skateboard::Application
{
public:
	GameApp()
	{
		PushLayer(new DefaultGameLayer());
	}
};

Skateboard::Application* Skateboard::CreateApplication(int argc, char** argv)
{
	return new GameApp();
}