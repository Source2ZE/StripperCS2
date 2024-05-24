/**
* ============================================================================ =
* StripperCS2
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
#include <vector>
#include <map>
#include <memory>
#include "actions/actions.h"
#include "extension.h"
#include <spdlog/spdlog.h>

#ifdef _WIN32
#define ROOTBIN "/bin/win64/"
#define GAMEBIN "/csgo/bin/win64/"
#else
#define ROOTBIN "/bin/linuxsteamrt64/"
#define GAMEBIN "/csgo/bin/linuxsteamrt64/"
#endif

#include <chrono>

class Timer
{
public:
	Timer()
	{
		m_StartTime = std::chrono::high_resolution_clock::now();
	}

	~Timer()
	{
		Stop();
	}

	void Stop()
	{
		auto endTime = std::chrono::high_resolution_clock::now();
		auto start = std::chrono::time_point_cast<std::chrono::microseconds>(m_StartTime).time_since_epoch().count();
		auto end = std::chrono::time_point_cast<std::chrono::microseconds>(endTime).time_since_epoch().count();

		auto duration = end - start;
		double ms = duration * 0.001;

		spdlog::info("Took {} us ({} ms)", duration, ms);
	}
private:
	std::chrono::time_point<std::chrono::high_resolution_clock> m_StartTime;
};

extern std::map<std::pair<std::string, std::string>, std::vector<std::unique_ptr<BaseAction>>> g_mapOverrides;

namespace Hook
{

void Detour_CreateWorldInternal(IWorldRendererMgr* pThis, CSingleWorldRep* singleWorld)
{
	g_pCreateWorldInternal(pThis, singleWorld);

	auto pWorld = singleWorld->m_pCWorld;

	{
		Timer timer;
		auto vecLumpData = (CUtlVector<void*>*)((uint8_t*)pWorld + 0x1B8);;

		FOR_EACH_VEC(*vecLumpData, i)
		{
			auto& lump = (*vecLumpData)[i];
			auto lumpData = *(LumpData**)lump;

			auto vecEntityKeyValues = (CUtlVector<CEntityKeyValues*>*)((uint8_t*)lumpData + 0x650);

			if (g_mapOverrides.find({ singleWorld->m_name.Get(), lumpData->m_name.Get() }) != g_mapOverrides.end())
			{
				spdlog::info("Map override applying {} {}", singleWorld->m_name.Get(), lumpData->m_name.Get());
				ApplyMapOverride(g_mapOverrides[{singleWorld->m_name.Get(), lumpData->m_name.Get()}], vecEntityKeyValues, lumpData);
			}

			if (g_mapOverrides.find({ "GLOBALOVERRIDE", "" }) != g_mapOverrides.end())
			{
				spdlog::info("Map override applying global rules");
				ApplyMapOverride(g_mapOverrides[{"GLOBALOVERRIDE", ""}], vecEntityKeyValues, lumpData);
			}
		}
	}
}

bool SetupHook()
{
	auto serverModule = new CModule(ROOTBIN, "worldrenderer");

	int err;
#ifdef WIN32
	const byte sig[] = "\x48\x89\x54\x24\x10\x53\x55\x56\x57\x48\x81\xEC\x98\x00\x00\x00";
#else
	const byte sig[] = "\x55\x48\x89\xE5\x41\x56\x41\x55\x41\x54\x49\x89\xFC\x53\x48\x89\xF3";
#endif
	g_pCreateWorldInternal = (CreateWorldInternal_t)serverModule->FindSignature((byte*)sig, sizeof(sig) - 1, err);

	if (err)
	{
		spdlog::critical("Failed to find CWorldRendererMgr::CreateWorld_Internal signature: {}", err);
		return false;
	}

	auto g_pHook = funchook_create();
	funchook_prepare(g_pHook, (void**)&g_pCreateWorldInternal, (void*)Detour_CreateWorldInternal);
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
