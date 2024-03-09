files {
	path.join(SDK_PATH, "public", "tier0", "memoverride.cpp"),
	path.join(SDK_PATH, "tier1", "convar.cpp"),
	path.join(SDK_PATH, "tier1", "generichash.cpp"),
	path.join(SDK_PATH, "entity2", "entitysystem.cpp"),
	path.join(SDK_PATH, "entity2", "entityidentity.cpp"),
	path.join(SDK_PATH, "entity2", "entitykeyvalues.cpp"),
	path.join(SDK_PATH, "tier1", "keyvalues3.cpp"),
	path.join(MM_PATH, "core", "sourcehook", "sourcehook.cpp"),
	path.join(MM_PATH, "core", "sourcehook", "sourcehook_impl_chookidman.cpp"),
	path.join(MM_PATH, "core", "sourcehook", "sourcehook_impl_chookmaninfo.cpp"),
	path.join(MM_PATH, "core", "sourcehook", "sourcehook_impl_cvfnptr.cpp"),
	path.join(MM_PATH, "core", "sourcehook", "sourcehook_impl_cproto.cpp"),
}

links {
	path.join(SDK_PATH, "lib", "public", "win64", "tier0.lib"),
	path.join(SDK_PATH, "lib", "public", "win64", "tier1.lib"),
	path.join(SDK_PATH, "lib", "public", "win64", "interfaces.lib"),
	path.join(SDK_PATH, "lib", "public", "win64", "mathlib.lib")
}

includedirs {
	_MAIN_SCRIPT_DIR,
	-- sdk
	SDK_PATH,
	path.join(SDK_PATH, "thirdparty", "protobuf-3.21.8", "src"),
	path.join(SDK_PATH, "common"),
	path.join(SDK_PATH, "game", "shared"),
	path.join(SDK_PATH, "game", "server"),
	path.join(SDK_PATH, "public"),
	path.join(SDK_PATH, "public", "engine"),
	path.join(SDK_PATH, "public", "mathlib"),
	path.join(SDK_PATH, "public", "tier0"),
	path.join(SDK_PATH, "public", "tier1"),
	path.join(SDK_PATH, "public", "entity2"),
	path.join(SDK_PATH, "public", "game", "server"),
	path.join(SDK_PATH, "public", "public", "entity2"),
	-- metamod
	path.join(MM_PATH, "core"),
	path.join(MM_PATH, "core", "sourcehook"),
}

defines {
	"COMPILER_MSVC",
	"COMPILER_MSVC64",
	"WIN32",
	"WINDOWS",
	"CRT_SECURE_NO_WARNINGS",
	"CRT_SECURE_NO_DEPRECATE",
	"CRT_NONSTDC_NO_DEPRECATE"
}

characterset "MBCS"