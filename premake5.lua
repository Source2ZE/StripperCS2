include("premake/utils")

SDK_PATH = os.getenv("HL2SDKCS2")
MM_PATH = os.getenv("MMSOURCE112")

if(SDK_PATH == nil) then
	error("INVALID HL2SDK PATH")
end

if(MM_PATH == nil) then
	error("INVALID METAMOD PATH")
end

workspace "StripperCS2"
	configurations { "Debug", "Release" }
	platforms {
		"win64",
		"linux64"
	}
	location "build"

project "StripperCS2"
	kind "SharedLib"
	language "C++"
	targetdir "bin/%{cfg.buildcfg}"
	location "build/StripperCS2"
	visibility  "Hidden"
	targetprefix ""

	files { "src/**.h", "src/**.cpp" }

	vpaths {
		["Headers/*"] = "src/**.h",
		["Sources/*"] = "src/**.cpp"
	}

	filter "configurations:Debug"
		defines { "DEBUG" }
		symbols "On"
		libdirs {
			path.join("vendor", "funchook", "lib", "Debug"),
		}

	filter "configurations:Release"
		defines { "NDEBUG" }
		optimize "On"
		libdirs {
			path.join("vendor", "funchook", "lib", "Release"),
		}

	filter "system:windows"
		cppdialect "c++20"
		include("premake/mm-windows.lua")
		links { "psapi" }

	filter "system:linux"
		cppdialect "c++2a"
		include("premake/mm-linux.lua")
		links { "pthread", "z"}

	filter {}

	defines { "META_IS_SOURCE2" }

	flags { "MultiProcessorCompile" }
	pic "On"

	links {
		"funchook",
		"distorm"
	}

	includedirs {
		path.join("vendor", "nlohmann"),
		path.join("vendor", "funchook", "include"),
		path.join("src"),
	}