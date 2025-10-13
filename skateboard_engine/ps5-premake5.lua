--include the ps5 premake module
require("premake-ps5")

workspace "Skateboard"
	architecture "x64"
		system "prospero"

	configurations
	{
		"Debug",
		"Release",
		"Ship"
	}
	platforms
	{
		"Prospero"
	}

startproject "GameApp"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
IncludeDir = {}
IncludeDir["ImGui"] = "Skateboard/vendor/imgui"
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
		"%{prj.name}/src/Skateboard/**.h",
		"%{prj.name}/src/Skateboard/**.cpp",
		"%{prj.name}/src/Platform/AGC/**.h",
		"%{prj.name}/src/Platform/AGC/**.cpp"
	}

	includedirs
	{
		"%{prj.name}/src",
		"%{IncludeDir.ImGui}",
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

	filter "system:prospero"
		cppdialect "C++17"
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
	kind "Application"
	language "C++"
	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("intermediate/" .. outputdir .. "/%{prj.name}")
	--shadermodel ("6.5")
	debugdir ("%{wks.location}bin/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/src/**.pssl"
	}

	includedirs
	{
		"Skateboard/src",
		"Skateboard/vendor",
		"Skateboard/vendor/entt/include",
		"Skateboard/vendor/glm"
	}

	links
	{
		"Skateboard"
	}

	postbuildcommands
	{
		('{COPY} "%{wks.location}vendor/bin/microsoft/Windows Kits/10.0.22621.0/x64" %{wks.location}bin/' .. outputdir .. '/%{prj.name}'),
		('{COPY} %{prj.location}assets %{wks.location}bin/' .. outputdir .. '/%{prj.name}/assets')
	}

	filter "system:prospero"
		cppdialect "C++17"
		staticruntime "on"
		systemversion "latest"

		defines
		{
			"SKTBD_PLATFORM_PROSPERO"
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

	filter "files:**_vv.pssl"
		profile "Vertex Shader (-profile sce_vs_vs_prospero)"
	--filter "files:**_gs.hlsl"
		--shadertype "Geometry"
	filter "files:**_pp.pssl"
		profile "Pixel Shader (-profile sce_ps_prospero)"
	filter "files:**_cs.pssl"
		profile "Compute Shader (-profile sce_cs_prospero)"
		

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
			cppdialect "C++17"
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