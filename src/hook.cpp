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
#include "khook.hpp"
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
extern std::string g_strCurrentMapName;

namespace Hook
{

KHook::Return<CSingleWorldRep*> Detour_CreateWorldInternal(IWorldRendererMgr* pThis, CSingleWorldRep* singleWorld)
{
	// The world can fail to be created and the function will return nullptr.
	if (!KHook::GetOriginalReturn<CSingleWorldRep*>())
		return {KHook::Action::Ignore};

	auto pWorld = singleWorld->m_pCWorld;

	{
		Timer timer;
		auto vecLumpData = (CUtlVector<void*>*)((uint8_t*)pWorld + 0x298);

		FOR_EACH_VEC(*vecLumpData, i)
		{
			auto& lump = (*vecLumpData)[i];
			auto lumpData = *(LumpData**)lump;

			auto vecEntityKeyValues = (CUtlVector<CEntityKeyValues*>*)((uint8_t*)lumpData + 0x1220);
			std::string singleWorldName = singleWorld->m_name.Get();
			std::string lumpDataName = lumpData->m_name.Get();

			std::transform(singleWorldName.begin(), singleWorldName.end(), singleWorldName.begin(), [](unsigned char c) { return std::tolower(c); });
			std::transform(lumpDataName.begin(), lumpDataName.end(), lumpDataName.begin(), [](unsigned char c) { return std::tolower(c); });

			if (g_mapOverrides.find({ singleWorldName, lumpDataName }) != g_mapOverrides.end())
			{
				spdlog::info("Map override applying {} {}", singleWorldName, lumpDataName);
				ApplyMapOverride(g_mapOverrides[{singleWorldName, lumpDataName}], vecEntityKeyValues, lumpData);
			}

			if (lumpDataName == "default_ents" && singleWorldName == g_strCurrentMapName && g_mapOverrides.find({ "GLOBAL_MAP_OVERRIDE", "" }) != g_mapOverrides.end())
			{
				spdlog::info("Map override applying global map rules");
				ApplyMapOverride(g_mapOverrides[{"GLOBAL_MAP_OVERRIDE", ""}], vecEntityKeyValues, lumpData);
			}

			if (g_mapOverrides.find({ "GLOBAL_LUMP_OVERRIDE", "" }) != g_mapOverrides.end())
			{
				spdlog::info("Map override applying global lump rules");
				ApplyMapOverride(g_mapOverrides[{"GLOBAL_LUMP_OVERRIDE", ""}], vecEntityKeyValues, lumpData);
			}
		}
	}

	return {KHook::Action::Ignore};
}

KHook::Function<CSingleWorldRep*, IWorldRendererMgr*, CSingleWorldRep*> createWorldInternalHook(nullptr, Detour_CreateWorldInternal);

bool SetupHook()
{
	CModule worldRendererModule(ROOTBIN, "worldrenderer");

#ifdef WIN32
	const char* sig = "48 89 5C 24 ? 48 89 54 24 ? 55 56 57 48 81 EC";
#else
	const char* sig = "55 48 89 E5 41 56 41 55 41 54 49 89 FC 53 48 89 F3 48 83 EC ? F6 46";
#endif
	auto pCreateWorldInternal = reinterpret_cast<CreateWorldInternal_t>(KHook::LookupSignature(worldRendererModule.m_base, worldRendererModule.m_size, sig));

	if (!pCreateWorldInternal)
	{
		spdlog::critical("Failed to find CWorldRendererMgr::CreateWorldInternal signature");
		return false;
	}

	createWorldInternalHook.Configure(pCreateWorldInternal);

	return true;
}

} // namespace Hook
