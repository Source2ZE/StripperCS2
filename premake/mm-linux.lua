files {
	path.join(SDK_PATH, "public", "tier0", "memoverride.cpp"),
	path.join(SDK_PATH, "tier1", "convar.cpp"),
	path.join(SDK_PATH, "tier1", "generichash.cpp"),
	path.join(MM_PATH, "core", "sourcehook", "sourcehook.cpp"),
	path.join(MM_PATH, "core", "sourcehook", "sourcehook_impl_chookidman.cpp"),
	path.join(MM_PATH, "core", "sourcehook", "sourcehook_impl_chookmaninfo.cpp"),
	path.join(MM_PATH, "core", "sourcehook", "sourcehook_impl_cvfnptr.cpp"),
	path.join(MM_PATH, "core", "sourcehook", "sourcehook_impl_cproto.cpp"),
}

libdirs {
	path.join(SDK_PATH, "lib", "linux64"),
}

linkoptions {
	"-l:libtier0.so",
	"-l:tier1.a",
	"-l:interfaces.a",
	"-l:mathlib.a",
}

includedirs {
	_MAIN_SCRIPT_DIR,
	-- sdk
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
	"_LINUX",
	"LINUX",
	"POSIX",
	"GNUC",
	"COMPILER_GCC",
	"PLATFORM_64BITS",
	"_GLIBCXX_USE_CXX11_ABI=0"
}

characterset "MBCS"