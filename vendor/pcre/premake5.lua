project "pcre"
		kind "StaticLib"
		language "C++"

		defines { "HAVE_CONFIG_H", "PCRE2_CODE_UNIT_WIDTH=8", "PCRE2_STATIC" }
		includedirs { "." }

		vpaths {
			["Headers/*"] = "**.h",
			["Sources/*"] = {"**.c", "**.cc"},
			["*"] = "premake5.lua"
		}

		files {
			"premake5.lua",
			"*.c",
			"*.cc",
			"*.h"
		}

		removefiles {
			"pcre2_jit_misc.c",
			"pcre2_jit_match.c",
			"pcre2_printint.c",
		}

		pic "On"

		filter "system:windows"
			defines { "HAVE_WINDOWS_H" }