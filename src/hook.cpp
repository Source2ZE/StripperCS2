/**
* ============================================================================ =
* CS2Fixes
* Copyright(C) 2023 - 2024 Source2ZE
* ============================================================================ =
*
*This program is free software; you can redistribute it and /or modify it under
* the terms of the GNU General Public License, version 3.0, as published by the
* Free Software Foundation.
*
* This program is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
* FOR A PARTICULAR PURPOSE.See the GNU General Public License for more
* details.
*
* You should have received a copy of the GNU General Public License along with
* this program.If not, see < http://www.gnu.org/licenses/>.
*/

#include "hook.h"
#include <tier0/logging.h>
#include "utils/module.h"

#ifdef _WIN32
#define ROOTBIN "/bin/win64/"
#define GAMEBIN "/csgo/bin/win64/"
#else
#define ROOTBIN "/bin/linuxsteamrt64/"
#define GAMEBIN "/csgo/bin/linuxsteamrt64/"
#endif

namespace Hook
{

void Detour_WorldInit(void* pWorld, void* pSceneWorld, int a3, int a4)
{
	ConMsg("Detour_WorldInit\n");
	g_pWorldInit(pWorld, pSceneWorld, a3, a4);
}

bool SetupHook()
{
	auto serverModule = new CModule(ROOTBIN, "worldrenderer");

	int err;
#ifdef WIN32
	const byte sig[] = "\x48\x89\x5C\x24\x08\x57\x48\x83\xEC\x20\x48\x8B\x01\x48\x8B\xFA\x48\x8B\xD9\x48\x89\x91\x20\x01\x00\x00";
#else
	const byte sig[] = "";
#endif
	g_pWorldInit = (WorldInit_t)serverModule->FindSignature((byte*)sig, sizeof(sig) - 1, err);

	if (err)
	{
		ConMsg("[StripperCS2] Failed to find CWorld::Init signature: %i\n", err);
		return false;
	}

	auto g_pHook = funchook_create();
	funchook_prepare(g_pHook, (void**)&g_pWorldInit, (void*)Detour_WorldInit);
	funchook_install(g_pHook, 0);

	return true;
}

void Cleanup()
{
	if (g_pHook)
	{
		funchook_uninstall(g_pHook, 0);
		funchook_destroy(g_pHook);
		g_pHook = nullptr;
	}
}

} // namespace Hook