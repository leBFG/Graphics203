#include "sktbdpch.h"

#define	SKTBD_LOG_COMPONENT "CORE_APP"
#include "Skateboard/Log.h"

#include "Application.h"

#include "Graphics/RHI/RenderCommand.h"
#include "Skateboard/Memory/MemoryManager.h"

namespace Skateboard
{
	Application::Application() :
		m_Platform(Platform::GetPlatform()),
		p_ImGuiMasterOverlay(new ImGuiLayer())	// Ownership is transferred to the layer stack, no need to worry about delete
	{
		// Set the platforms event call back function
		m_Platform.SetOnEventCallback(BIND_EVENT(Application::OnEvent));
		m_Platform.Init();

		Skateboard::GraphicsContext::Reset();

		// Initialise the render interfaces
		RenderCommand::RegisterRenderCommand(GraphicsContext::Context->GetAPI());
		ResourceFactory::RegisterResourceFactory(GraphicsContext::Context->GetResourceFactory());

		//TODO: Need a more graceful solution, for now should work.


		SKTBD_LOG_ASSERT(!s_Instance, "Cannot create two application instances. Consider using layers to create other windows!");
		s_Instance = this;

		PushOverlay(p_ImGuiMasterOverlay);
	}

	void Application::Run()
	{
		// Execute any GPU initialisation required from these layers
		SKTBD_MSG_INFO("Flushing GPU for post-initialisation");
		Skateboard::GraphicsContext::Context->Flush();

		bool running = true;
		while (running)
		{
			// Release some CPU consumption if the application is not used
			/*if (m_ApplicationPaused)
			{
				Sleep(100);
				continue;
			}*/

			// Update the platform (could be window messages, controller inputs, ...)
			// We will avoid uncessary rendering by quitting immediately if the context is destroyed
			if (!m_Platform.Update())
				break;

			// Get delta time from the platform (the procedure of getting time may differ!)
			TimeManager* timeManager = Platform::GetTimeManager();

			// Generic game loop
			for (Layer* layer : m_LayerStack)
				running &= layer->OnHandleInput(timeManager);
			for (Layer* layer : m_LayerStack)
				running &= layer->OnUpdate(timeManager);

			Skateboard::GraphicsContext::Update();

			// Prepare internal engine for rendering scene.
			Skateboard::GraphicsContext::Context->BeginFrame();

			// Render all the layers main graphics
			for (Layer* layer : m_LayerStack)
				layer->OnRender();

			// Render all the user interface
			p_ImGuiMasterOverlay->Begin();
			for (Layer* layer : m_LayerStack)
				layer->OnImGuiRender();
			p_ImGuiMasterOverlay->End();

			// Execute the rendering work recorded on the GPU
			Skateboard::GraphicsContext::Context->EndFrame();

			//next frame
			Skateboard::GraphicsContext::Context->NextFrame();
		}

		// When exitting the app, ensure all GPU work has concluded
		SKTBD_MSG_INFO("Exiting App, GPU idle expected..");
		Skateboard::GraphicsContext::Context->WaitUntilIdle();
	}

	void Application::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);

		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT(Application::OnWindowClose));
		dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT(Application::OnWindowResize));

		Platform::GetPlatform().OnEvent(e);

		// Go through each layer on the stack and pass the event forward to them
		for (auto it = m_LayerStack.end(); it != m_LayerStack.begin(); )
		{
			(*--it)->OnEvent(e);

			if (e.IsHandled())
				break;
		}
	}

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		return false;
	}

	bool Application::OnWindowResize(WindowResizeEvent& e)
	{
		return false;
	}
}