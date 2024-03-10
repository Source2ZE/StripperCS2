project "spdlog"
	kind "StaticLib"
	language "C++"

	includedirs {
		path.join(_MAIN_SCRIPT_DIR, "vendor", "spdlog", "include")
	}

	defines {
		"SPDLOG_COMPILED_LIB",
	}

	files {
		path.join(_MAIN_SCRIPT_DIR, "vendor", "spdlog", "src", "**.cpp")
	}

	pic "On"