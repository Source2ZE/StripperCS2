include("premake/utils")

SDK_PATH = os.getenv("HL2SDKCS2")
MM_PATH = os.getenv("MMSOURCE_DEV")

if(SDK_PATH == nil) then
	error("INVALID HL2SDK PATH")
end

if(MM_PATH == nil) then
	error("INVALID METAMOD PATH")
end

newaction {
	trigger = "package",
	description = "Package StripperCS2",
	execute = function()
		local package_path = path.join(_MAIN_SCRIPT_DIR, "build", "package", "StripperCS2")
		local package_bin_path = path.join(package_path, "addons", "StripperCS2", "bin")
		local package_metamod_path = path.join(package_path, "addons", "metamod")
		local package_example_path = path.join(package_path, "addons", "StripperCS2", "maps", "de_example")
		local bin_path = path.join(_MAIN_SCRIPT_DIR, "bin", "Release")
		local binaries = os.target() == "windows"
			and { "StripperCS2.dll", "StripperCS2.pdb" }
			or { "StripperCS2.so" }
		local function copy_file(source, destination)
			if not os.isfile(source) then
				error("MISSING PACKAGE FILE: " .. source)
			end

			local ok, err = os.copyfile(source, destination)
			if not ok then
				error(err)
			end
		end

		os.mkdir(package_bin_path)
		os.mkdir(package_metamod_path)
		os.mkdir(package_example_path)

		for _, binary in ipairs(binaries) do
			copy_file(path.join(bin_path, binary), path.join(package_bin_path, binary))
		end

		copy_file(
			path.join(_MAIN_SCRIPT_DIR, "package", "StripperCS2.vdf"),
			path.join(package_metamod_path, "StripperCS2.vdf")
		)
		copy_file(
			path.join(_MAIN_SCRIPT_DIR, "package", "default_ents.jsonc"),
			path.join(package_example_path, "default_ents.jsonc")
		)
	end
}

workspace "StripperCS2"
	configurations { "Debug", "Release" }
	platforms {
		"x64"
	}
	location "build"
	filter "system:windows"
		buildoptions { "/utf-8" }
	filter "system:linux"
		toolset "clang"
	filter {}

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

	filter "configurations:Release"
		defines { "NDEBUG" }
		symbols "On"
		optimize "On"

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

	multiprocessorcompile "On"
	pic "On"

	links {
		"pcre",
		"spdlog"
	}

	includedirs {
		path.join("vendor", "nlohmann"),
		path.join("vendor", "spdlog", "include"),
		path.join("vendor"),
		path.join("src"),
	}

include "vendor/pcre"
include "premake/spdlog"