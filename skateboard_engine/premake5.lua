workspace "Skateboard"
	architecture "x64"

	configurations
	{
		"Debug",
		"Release",
		"Ship"
	}
	platforms
	{
		"Windows"
	}

startproject "GameApp"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
IncludeDir = {}
IncludeDir["ImGui"] = "Skateboard/vendor/imgui"
IncludeDir["SpdLog"] = "Skateboard/vendor/spdlog/include"
IncludeDir["FL"] = "Skateboard/vendor/frankluna"
IncludeDir["Microsoft"] = "Skateboard/vendor/microsoft"
IncludeDir["Entt"] = "Skateboard/vendor/entt/include"
IncludeDir["Glm"] = "Skateboard/vendor/glm"

project "Skateboard"
	location "Skateboard"
	kind "StaticLib"
	language "C++"
	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("intermediate/" .. outputdir .. "/%{prj.name}")
	pchheader "sktbdpch.h"
	pchsource "Skateboard/src/sktbdpch.cpp"
	
	

	disablewarnings
	{
		"4006"
	}

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
		"%{IncludeDir.FL}/**.h",
		"%{IncludeDir.FL}/**.cpp",
		"%{IncludeDir.Microsoft}/**.h",
		"%{IncludeDir.Microsoft}/**.cpp"
	}

	includedirs
	{
		"%{prj.name}/src",
		"%{IncludeDir.SpdLog}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.FL}",
		"%{IncludeDir.Microsoft}",
		"%{IncludeDir.Entt}",
		"%{IncludeDir.Glm}"
	}

	links
	{
		"ImGui"
	}

	filter "files:Skateboard/vendor/**.cpp"
		flags
		{
			"NoPCH"
		}

	filter "system:windows"
		cppdialect "C++20"
		staticruntime "on"
		systemversion "latest"

		defines
		{
			"SKTBD_PLATFORM_WINDOWS"
		}

	filter "configurations:Debug"
		defines "SKTBD_DEBUG"
		symbols "on"

	filter "configurations:Release"
		defines "SKTBD_RELEASE"
		optimize "on"

	filter "configurations:Ship"
		defines "SKTBD_SHIP"
		optimize "on"

project "GameApp"
	location "GameApp"
	kind "ConsoleApp"
	language "C++"
	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("intermediate/" .. outputdir .. "/%{prj.name}")
	shadermodel ("6.5")
	debugdir ("%{wks.location}bin/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/src/**.hlsl"
	}

	includedirs
	{
		"Skateboard/vendor/spdlog/include",
		"Skateboard/src",
		"Skateboard/vendor",
		"Skateboard/vendor/entt/include",
		"Skateboard/vendor/glm"
	}

	links
	{
		"Skateboard",
		"d3d12.lib",
		"dxgi.lib",
		"dxcompiler.lib"
	}

	postbuildcommands
	{
		('{COPY} "%{wks.location}vendor/bin/microsoft/Windows Kits/10.0.22621.0/x64" %{wks.location}bin/' .. outputdir .. '/%{prj.name}'),
		('{COPY} %{prj.location}assets %{wks.location}bin/' .. outputdir .. '/%{prj.name}/assets')
	}

	filter "system:windows"
		cppdialect "C++20"
		staticruntime "on"
		systemversion "latest"

		defines
		{
			"SKTBD_PLATFORM_WINDOWS"
		}

	filter "configurations:Debug"
		defines "SKTBD_DEBUG"
		symbols "on"

	filter "configurations:Release"
		defines "SKTBD_RELEASE"
		optimize "on"

	filter "configurations:Ship"
		defines "SKTBD_SHIP"
		optimize "on"

	filter "files:**_vs.hlsl"
		shadertype "Vertex"
	filter "files:**_hs.hlsl"
		shadertype "Hull"
	filter "files:**_ds.hlsl"
		shadertype "Domain"
	filter "files:**_gs.hlsl"
		shadertype "Geometry"
	filter "files:**_ps.hlsl"
		shadertype "Pixel"
	filter "files:**_cs.hlsl"
		shadertype "Compute"
	filter "files:**_ms.hlsl"
		shadertype "Mesh"
	filter "files:**_as.hlsl"
		shadertype "Amplification"
		

project "ImGui"
		location "Skateboard/src/Skateboard/ImGui"
		kind "StaticLib"
		language "C++"
		targetdir ("bin/" .. outputdir .. "/%{prj.name}")
		objdir ("intermediate/" .. outputdir .. "/%{prj.name}")

		files
		{
			"Skateboard/vendor/imgui/imconfig.h",
			"Skateboard/vendor/imgui/imgui.cpp",
			"Skateboard/vendor/imgui/imgui.h",
			"Skateboard/vendor/imgui/imgui_demo.cpp",
			"Skateboard/vendor/imgui/imgui_draw.cpp",
			"Skateboard/vendor/imgui/imgui_internal.h",
			"Skateboard/vendor/imgui/imgui_tables.cpp",
			"Skateboard/vendor/imgui/imgui_widgets.cpp",
			"Skateboard/vendor/imgui/imstb_rectpack.h",
			"Skateboard/vendor/imgui/imstb_textedit.h",
			"Skateboard/vendor/imgui/imstb_truetype.h"
		}

		filter "system:windows"
			systemversion "latest"
			cppdialect "C++20"
			staticruntime "on"

		filter "configurations:Debug"
			defines "SKTBD_DEBUG"
			symbols "on"
	
		filter "configurations:Release"
			defines "SKTBD_RELEASE"
			optimize "on"
	
		filter "configurations:Ship"
			defines "SKTBD_SHIP"
			optimize "on"