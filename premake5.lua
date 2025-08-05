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
		symbols "On"
		optimize "On"
		libdirs {
			path.join("vendor", "funchook", "lib", "Release"),
		}

	filter "system:windows"
		cppdialect "c++20"
		include("premake/mm-windows.lua")
		links { "psapi" }
		staticruntime "On"

	filter "system:linux"
		defines { "stricmp=strcasecmp" }
		cppdialect "c++2a"
		include("premake/mm-linux.lua")
		links { "pthread", "z"}
		linkoptions { '-static-libstdc++', '-static-libgcc' }
		disablewarnings { "register" }

	filter {}

	defines { "META_IS_SOURCE2", "PCRE2_CODE_UNIT_WIDTH=8", "PCRE2_STATIC" }

	flags { "MultiProcessorCompile" }
	pic "On"

	links {
		"funchook",
		"distorm",
		"pcre",
		"spdlog"
	}

	includedirs {
		path.join("vendor", "nlohmann"),
		path.join("vendor", "funchook", "include"),
		path.join("vendor", "spdlog", "include"),
		path.join("vendor"),
		path.join("src"),
	}

include "vendor/pcre"
include "premake/spdlog"