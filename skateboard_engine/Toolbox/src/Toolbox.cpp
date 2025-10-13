#include <Skateboard.h>
#include "Skateboard/EntryPoint.h"

#include "AssetEditorLayer.h"

class ToolboxApp : public Skateboard::Application
{
public:
	ToolboxApp()
	{
		PushLayer(new Toolbox::AssetEditorLayer());
	}
};

Skateboard::Application* Skateboard::CreateApplication(int argc, char** argv)
{
	return new ToolboxApp();
}