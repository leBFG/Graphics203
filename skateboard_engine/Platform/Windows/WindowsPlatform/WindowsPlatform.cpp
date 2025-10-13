#include "sktbdpch.h"
#include "WindowsPlatform.h"

#include "Skateboard/Input.h"
#include "Skateboard/Application.h"
#include "Graphics/RHI/D3DGraphicsContext.h"
#include "WindowsDeviceManager.h"

#include "WindowsUserManager.h"
#include "WindowsTimeManager.h"

#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_impl_dx12.h"

#include "imgui/imgui_internal.h"
#include "DirectX12/Assets/D3DAssetManager.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

#define SKTBD_LOG_COMPONENT "WinPlatform"
#include "WindowsLog.h"

namespace Skateboard
{
	IMPLEMENT_PLATFORM(WindowsPlatform)

	WindowsPlatform::WindowsPlatform() : 
		m_MainWindow(NULL),
		m_ActiveWindow(NULL),
		m_WindowRectSave{},
		m_FullscreenMode(false),
		m_ApplicationMaximised(false),
		m_ApplicationMinimised(false),
		m_ApplicationResizing(false)
	{

		m_Logger = std::make_unique<WindowsLog>();
		Logger::RegisterLogger(m_Logger.get());

		SKTBD_MSG_TRACE("Creating platform..");
	}

	WindowsPlatform::~WindowsPlatform()
	{
		SKTBD_MSG_TRACE("Destroying Windows platform..");
	}

	void WindowsPlatform::Init(const PlatformProperties& props)
	{
		SKTBD_MSG_TRACE("Initialising the Windows Platform..");

		// Create and initialise the main application window
		SKTBD_LOG_ASSERT(InitWindowsApp(props), "Could not successfully initialise a Windows application!")

		// Create the API context (this could be changed to Vulkan)
		m_GraphicsContext = std::make_unique<D3DGraphicsContext>(m_MainWindow, props);

		//initialise device libraries
		m_Devices = std::make_unique<WindowsDeviceManager>();
		m_Devices->Init();

		//Login Users
		m_Users = std::make_unique<WindowsUserManager>();
		m_Users->Init();

		// Initialise the time manager
		m_Timer = std::make_unique<WindowsTimeManager>();

		// Init AssetManager
		m_AssetManager = std::make_unique<D3DAssetManager>();

		Platform::Init(props);
	}

	bool WindowsPlatform::Update()
	{
		Platform::Update();

		MSG msg = {};
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			// If a message was found, translate and dispatch
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			// Exit if the corresponding message was received
			// Must be translated and dispatched before to properly close the window
			if (msg.message == WM_QUIT)
				return false;
		}
		return true;
	}

	void WindowsPlatform::Terminate()
	{
		SendMessage(m_MainWindow, WM_CLOSE, NULL, NULL);
		Platform::Terminate();
	}

	void WindowsPlatform::ToggleFullscreen()
	{
		GoFullScreen();
		Platform::ToggleFullscreen();
	}


	void WindowsPlatform::ResizeBackBuffers(WPARAM wParam, LPARAM lParam)
	{
		// This resizing logic is solely deterministic for the graphics renderer
		if (!m_GraphicsContext.get())
			return;

		// Check if the resize tells us that the window is being minimised
		// In that case, we want to pause the application as it will not be displayed anyways
		if (wParam == SIZE_MINIMIZED)
		{
			//SetInactive(true);
			m_ApplicationMinimised = true;
			m_ApplicationMaximised = false;
		}

		// Else, if in contrary it hase been maximized with the button, resize the buffers immediately
		else if (wParam == SIZE_MAXIMIZED)
		{
			//SetInactive(false);
			m_ApplicationMinimised = false;
			m_ApplicationMaximised = true;
			m_GraphicsContext->Resize(LOWORD(lParam), HIWORD(lParam));
			//m_GraphicsContext->OnResized();
		}

		// SIZE_RESTORED dictates that the application has been restored to its orginal size
		// Thus, we must react based on its previous state
		else if (wParam == SIZE_RESTORED)
		{
			// If we are restoring from the minimised state we must resize the buffers immediately
			if (m_ApplicationMinimised)
			{
				//SetInactive(false);
				m_ApplicationMinimised = false;
				m_GraphicsContext->Resize(LOWORD(lParam), HIWORD(lParam));
				//m_GraphicsContext->OnResized();
			}

			// If we are restoring from the maximised state we must also resize the buffers immediately
			else if (m_ApplicationMaximised)
			{
				//SetInactive(false);
				m_ApplicationMaximised = false;
				m_GraphicsContext->Resize(LOWORD(lParam), HIWORD(lParam));
				//m_GraphicsContext->OnResized();
			}

			// If the user is currently modifying the window's size by dragging the resize bars,
			// it would be very inefficient to resize the buffers, as this message occurs everyframe.
			// Instead, the buffers will be resized after the user has finished resizing the window
			else if (m_ApplicationResizing)
			{
				m_GraphicsContext->Resize(LOWORD(lParam), HIWORD(lParam));
			}

			// Finally, A couple of other API calls can trigger here such as SetWindowPos or mSwapChain->SetFullscreenState
			// These require an immediate resize
			else
			{
				m_GraphicsContext->Resize(LOWORD(lParam), HIWORD(lParam));
				//m_GraphicsContext->OnResized();
			}
		}
	}

	void WindowsPlatform::ForwardResizeEvents(WPARAM wParam, LPARAM lParam)
	{
		PlatformData  data =
		{
			L"",
			0,
			0,
			0,
			0,
			LOWORD(lParam), // x
			HIWORD(lParam),	// y
			false,
			false,
			false,
			false
		};

		WindowResizeEvent event(data.Width, data.Height);
		data.EventCallback(event);
	}

	void WindowsPlatform::ForwardMinimiseEvents(bool appMinimised)
	{
		PlatformData data =
		{
			L"",
			0,
			0,
			RECT(),
			0,	//	x
			0,	//	y
			false,
			appMinimised,
			false,
			false
		};

		WindowCloseEvent event;
		data.EventCallback(event);
	}

	void WindowsPlatform::ForwardCloseEvents(bool closeApp)
	{
		PlatformData data =
		{
			L"",
			0,
			0,
			RECT(),
			0,	//	x
			0,	//	y
			closeApp,
			false,
			false,
			false
		};

		WindowCloseEvent event;
		data.EventCallback(event);
	}

	void WindowsPlatform::InitImGui()
	{
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;			// Enable docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		WindowsPlatform& platform = static_cast<WindowsPlatform&>(Platform::GetPlatform());
		ID3D12DescriptorHeap* srvHeap = gD3DContext->GetSRVHeap();
		D3DDescriptorHandle imguiHandle = gD3DContext->GetImGuiDescriptorHandle();

		// TODO: This is still platform dependant!
		ImGui_ImplWin32_Init(platform.GetWindow());
		ImGui_ImplDX12_Init(
			gD3DContext->GetDevice(),
			GRAPHICS_SETTINGS_NUMFRAMERESOURCES,
			gD3DContext->GetBackBufferFormat(),
			srvHeap,
			imguiHandle.GetCPUHandle(),
			imguiHandle.GetGPUHandle()
		);
	}

	void WindowsPlatform::BeginImGuiPass()
	{
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame(); 
		ImGui::NewFrame();
		ImGui::DockSpaceOverViewport(ImGui::GetMainViewport()->ID, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);
	}

	void WindowsPlatform::EndImGuiPass()
	{
		ID3D12GraphicsCommandList* pCommandList = gD3DContext->GetDefaultCommandList();

		ImGui::Render();
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), pCommandList);

		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault(nullptr, (void*)pCommandList);
		}
	}

	void WindowsPlatform::ShutdownImGui()
	{
		ImGui_ImplDX12_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}

	BOOL WindowsPlatform::InitWindowsApp(const PlatformProperties& props)
	{
		SKTBD_MSG_TRACE("Creating the output window..");

		// This is valid as it is an exe (not for DLLs!!)
		HINSTANCE hInstance = GetModuleHandle(NULL);

		// Describe the window's characteristics
		WNDCLASS wc = {};
		wc.style = CS_HREDRAW | CS_VREDRAW;							// Repaint when either the horizontal or vertical when window size is changed
		wc.lpfnWndProc = WindowProcedure;							// Pointer to the window procedure function to associate with this WNDCLASS instance. This is how multiple windows can use the same procedure
		wc.cbClsExtra = 0;											// Extra memory slots for the application (unused -> 0)
		wc.cbWndExtra = 0;											// Extra memory slots for the application (unused -> 0)
		wc.hInstance = hInstance;									// Handle to the application instance
		wc.hIcon = LoadIcon(0, IDI_APPLICATION);					// Handle to an icon to use for the windows created (can be customized)
		wc.hCursor = LoadCursor(0, IDC_ARROW);						// Same idea as the icon but for the cursor
		wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);		// Handle to brush: it specifies the background color of the client area of the window
		wc.lpszMenuName = 0;										// Specifies the window's menu (no menu here -> 0)
		wc.lpszClassName = L"System";								// Specifies the name of the window class structure we are creating

		// Register this WNDCLASS instance with Windows to create a window with it
		if (!RegisterClass(&wc))
		{
			SKTBD_MSG_ERROR("RegisterClass Failed!");
			return FALSE;
		}

		// This is a little hack that will replace the default ImGui_ImplWin32_WndProcHandler_PlatformWindow with our own custom one!
		WNDCLASSEX wcex;
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = ImGuiWindowProcedure;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = GetModuleHandle(nullptr);
		wcex.hIcon = nullptr;
		wcex.hCursor = nullptr;
		wcex.hbrBackground = (HBRUSH)(COLOR_BACKGROUND + 1);
		wcex.lpszMenuName = nullptr;
		wcex.lpszClassName = L"ImGui Platform";	// This is the class name ImGui uses when creating a viewport window
		wcex.hIconSm = nullptr;
		if (!RegisterClassEx(&wcex))
		{
			SKTBD_MSG_ERROR("ImGui RegisterClass Failed!");
			return FALSE;
		}

		// Get the area of the window to which we can draw onto from the application
		// This does not correspond to the window's width and height when in windowed mode,
		// as it does not include the bar at the top of the window. So we need to adjust them
		RECT windowSize = { 0, 0, static_cast<long>(props.BackBufferWidth), static_cast<long>(props.BackBufferHeight) };
		AdjustWindowRect(&windowSize, WS_OVERLAPPEDWINDOW, false);
		const int newWidth = windowSize.right - windowSize.left;
		const int newHeight = windowSize.bottom - windowSize.top;

		// Create a window and verify that it exists
		m_MainWindow = CreateWindow(
			L"System",								// The name of the registered WNDCLASS structure that describes this window
			props.Title.c_str(),					// The name we want to give our window
			WS_OVERLAPPEDWINDOW,					// Style of the window
			CW_USEDEFAULT,							// The X position of the top left corner of the window relative to the screen (CW_USEDEFAULT = Windows chooses an appropriate default)
			CW_USEDEFAULT,							// The Y position of the top left corner of the window relative to the screen
			newWidth,								// The width of the window in pixels
			newHeight,								// The height of the window in pixels
			0,										// Handle to a parent window (no relationship -> 0)
			0,										// Handle to a menu (no menu -> 0)
			hInstance,								// Handle to the application instance this window will be assiociated with
			0										// A pointer to user-defined data that you want to be available to a WM_CREATE message handler (if you want to do smth when it's being created)
		);

		// Verify success and proceed
		if (!m_MainWindow)
		{
			SKTBD_MSG_ERROR("CreateWindow Failed!");
			return FALSE;
		}
		m_ActiveWindow = m_MainWindow;

		// Show the window on the screen
		ShowWindow(m_MainWindow, SW_SHOWDEFAULT);	// The 'show' values determines if the window needs to be maximized, minimized, etc.

		// Refresh the client area of the window window after showing it
		UpdateWindow(m_MainWindow);

		// Initialisation successful
		return TRUE;
	}

	void WindowsPlatform::GoFullScreen()
	{
		if (m_FullscreenMode)
		{
			// Restore the window's attributes and size.
			SetWindowLong(m_MainWindow, GWL_STYLE, WS_OVERLAPPEDWINDOW);

			SetWindowPos(
				m_MainWindow,
				HWND_NOTOPMOST,
				m_WindowRectSave.left,
				m_WindowRectSave.top,
				m_WindowRectSave.right - m_WindowRectSave.left,
				m_WindowRectSave.bottom - m_WindowRectSave.top,
				SWP_FRAMECHANGED | SWP_NOACTIVATE);

			ShowWindow(m_MainWindow, SW_NORMAL);
		}
		else
		{
			// Save the old window rect so we can restore it when exiting fullscreen mode.
			GetWindowRect(m_MainWindow, &m_WindowRectSave);

			// Make the window borderless so that the client area can fill the screen.
			SetWindowLong(m_MainWindow, GWL_STYLE, WS_OVERLAPPEDWINDOW & ~(WS_CAPTION | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_SYSMENU | WS_THICKFRAME));

			RECT fullscreenWindowRect;

			// Get the settings of the primary display
			DEVMODE devMode = {};
			devMode.dmSize = sizeof(DEVMODE);
			EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &devMode);

			fullscreenWindowRect = {
				devMode.dmPosition.x,
				devMode.dmPosition.y,
				devMode.dmPosition.x + static_cast<LONG>(devMode.dmPelsWidth),
				devMode.dmPosition.y + static_cast<LONG>(devMode.dmPelsHeight)
			};

			SetWindowPos(
				m_MainWindow,
				HWND_TOPMOST,
				fullscreenWindowRect.left,
				fullscreenWindowRect.top,
				fullscreenWindowRect.right,
				fullscreenWindowRect.bottom,
				SWP_FRAMECHANGED | SWP_NOACTIVATE);

			ShowWindow(m_MainWindow, SW_MAXIMIZE);
		}

		m_FullscreenMode = !m_FullscreenMode;
	}

	//this should be part of input and issue events instead of being part of the platform

	LRESULT WindowsPlatform::WindowProcedure(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		// Handle some specific messages
		// When a message is being handled, it should return 0
		// CreateWindow() will fail if this function returns something invalid
		// This feels mega dirty but should only be used here as a special case. If it turns out not to, then change this pattern.
		WindowsPlatform& singleton = static_cast<WindowsPlatform&>(Platform::GetPlatform());
		Application* appSingleton = Application::Singleton();
		singleton.m_ActiveWindow = hwnd;

	

		// Handle ImGui Inputs
		if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam))
			return true;

		switch (msg)
		{
		case WM_GETMINMAXINFO:
			// This event allows us to prevent the window to become smaller than a certain size
			((MINMAXINFO*)lParam)->ptMinTrackSize.x = MINIMUM_WINDOW_SIZE_WIDTH;
			((MINMAXINFO*)lParam)->ptMinTrackSize.y = MINIMUM_WINDOW_SIZE_HEIGHT;
			return 0;
		case WM_ENTERSIZEMOVE:
			// This message is sent when the user is starting to drag the resize bars on the edges of the window
			// Therefore, in order to not constantly resize the buffers, pause the application at that point and
			// resize when the user is finished
			singleton.m_ApplicationResizing = true;
			return 0;
		case WM_SIZE:
			// Attempt to resize the application if it authorise us to do so
			singleton.ResizeBackBuffers(wParam, lParam);

			// Test this, based on new event system.
			//ForwardResizeEvents(LOWORD(lParam), HIWORD(lParam));

			return 0;
		case WM_EXITSIZEMOVE:
			// This message is sent when the user has finished dragging the resize bars
			singleton.m_ApplicationResizing = false;
			// Resize the window with the latest recieved dimension in WM_SIZE
			//singleton.m_GraphicsContext->OnResized();

			return 0;
		case WM_KEYDOWN:
			// Handle some special keys for specific behaviours on windows
			/*if (wParam == VK_F1)
			{
				singleton.GoFullScreen();
				return 0;
			}*/
			/*else if (wParam == VK_ESCAPE)
			{
				SendMessage(singleton.m_MainWindow, WM_CLOSE, NULL, NULL);
				return 0;
			}*/
			Input::SetKeyDown(LOBYTE(wParam));
			return 0;
		case WM_KEYUP:
			Input::SetKeyUp(LOBYTE(wParam));
			return 0;
		case WM_LBUTTONDOWN:
			Input::SetKeyDown(VK_LBUTTON);
			return 0;
		case WM_LBUTTONUP:
			Input::SetKeyUp(VK_LBUTTON);
			return 0;
		case WM_RBUTTONDOWN:
			Input::SetKeyDown(VK_RBUTTON);
			return 0;
		case WM_RBUTTONUP:
			Input::SetKeyUp(VK_RBUTTON);
			return 0;
	//	case WM_MOUSEMOVE:
	//		Input::SetMousePos(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
	//		return 0;
		case WM_CLOSE:
			DestroyWindow(hwnd);
			return 0;
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
		default:
			// Forward any unhandled messages to the default window procedure
			return DefWindowProc(hwnd, msg, wParam, lParam);
		}
	}

	LRESULT WindowsPlatform::ImGuiWindowProcedure(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		// This window procedure is pretty much copy/pasted from ImGui_ImplWin32_WndProcHandler_PlatformWindow
		// but I added the input code so that we could still receive custom inputs!
		WindowsPlatform& singleton = static_cast<WindowsPlatform&>(Platform::GetPlatform());
		singleton.m_ActiveWindow = hwnd;

		if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam))
			return true;

		if (ImGuiViewport* viewport = ImGui::FindViewportByPlatformHandle((void*)hwnd))
		{
			switch (msg)
			{
			case WM_CLOSE:
				viewport->PlatformRequestClose = true;
				return 0;
			case WM_MOVE:
				viewport->PlatformRequestMove = true;
				break;
			case WM_SIZE:
				viewport->PlatformRequestResize = true;
				break;
			case WM_MOUSEACTIVATE:
				if (viewport->Flags & ImGuiViewportFlags_NoFocusOnClick)
					return MA_NOACTIVATE;
				break;
			case WM_NCHITTEST:
				// Let mouse pass-through the window. This will allow the backend to call io.AddMouseViewportEvent() correctly. (which is optional).
				// The ImGuiViewportFlags_NoInputs flag is set while dragging a viewport, as want to detect the window behind the one we are dragging.
				// If you cannot easily access those viewport flags from your windowing/event code: you may manually synchronize its state e.g. in
				// your main loop after calling UpdatePlatformWindows(). Iterate all viewports/platform windows and pass the flag to your windowing system.
				if (viewport->Flags & ImGuiViewportFlags_NoInputs)
					return HTTRANSPARENT;
				break;
			case WM_KEYDOWN:
				// Handle some special keys for specific behaviours on windows
				Input::SetKeyDown(LOBYTE(wParam));
				return 0;
			case WM_KEYUP:
				Input::SetKeyUp(LOBYTE(wParam));
				return 0;
			case WM_LBUTTONDOWN:
				Input::SetKeyDown(VK_LBUTTON);
				return 0;
			case WM_LBUTTONUP:
				Input::SetKeyUp(VK_LBUTTON);
				return 0;
			case WM_RBUTTONDOWN:
				Input::SetKeyDown(VK_RBUTTON);
				return 0;
			case WM_RBUTTONUP:
				Input::SetKeyUp(VK_RBUTTON);
				return 0;
//			case WM_MOUSEMOVE:
//				Input::SetMousePos(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
//				return 0;
			}
		}
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
}