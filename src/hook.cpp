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

struct LumpData
{
	CUtlString m_name;
	char pad[0x20];
	CKeyValues3Context* m_allocatorContext;
};

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

			auto vecEntityKeyValues = (CUtlVector<CEntityKeyValues*>*)((uint8_t*)lumpData + 0x658);

			if (g_mapOverrides.find({ singleWorld->m_name.Get(), lumpData->m_name.Get() }) != g_mapOverrides.end())
			{
				spdlog::info("Map override applying {} {}", singleWorld->m_name.Get(), lumpData->m_name.Get());
				for (const auto& action : g_mapOverrides[{singleWorld->m_name.Get(), lumpData->m_name.Get()}])
				{
					if (action->GetType() == ActionType_t::Filter)
					{
						auto filterAction = (FilterAction*)action.get();
						FOR_EACH_VEC(*vecEntityKeyValues, j)
						{
							if (DoesEntityMatch((*vecEntityKeyValues)[j], filterAction->m_vecMatches))
							{
								spdlog::critical("ENTITY MATCHED");
								vecEntityKeyValues->Remove(j);
								j--;
							}
						}
					}

					if (action->GetType() == ActionType_t::Add)
					{
						auto addAction = (AddAction*)action.get();

						auto keyValues = new CEntityKeyValues(lumpData->m_allocatorContext, EKV_ALLOCATOR_EXTERNAL);

						for (const auto& insert : addAction->m_vecInsertions)
							AddEntityInsert(keyValues, insert);

						keyValues->AddRef(); // this shit cost me like 3 hours :)
						vecEntityKeyValues->AddToTail(keyValues);
					}

					if (action->GetType() == ActionType_t::Modify)
					{
						auto modifyAction = (ModifyAction*)action.get();
						FOR_EACH_VEC(*vecEntityKeyValues, j)
						{
							auto keyValues = (*vecEntityKeyValues)[j];
							if (!DoesEntityMatch(keyValues, modifyAction->m_vecMatches))
								continue;

							for (const auto& replace : modifyAction->m_vecReplacements)
							{
								if (auto io = std::get_if<IOConnection>(&replace.m_Value))
								{
								}
								else if (auto str = std::get_if<std::string>(&replace.m_Value))
								{
									keyValues->SetString(replace.m_strName.c_str(), str->c_str());
								}
							}

							for (const auto& _delete : modifyAction->m_vecDeletions)
							{
								if (auto io = std::get_if<IOConnection>(&_delete.m_Value))
								{
								}
								else if (!std::holds_alternative<std::monostate>(_delete.m_Value))
									if(keyValues->HasValue(_delete.m_strName.c_str()) && DoesValueMatch(keyValues->GetString(_delete.m_strName.c_str()), _delete.m_Value))
										keyValues->RemoveKeyValue(_delete.m_strName.c_str());
							}

							for (const auto& insert : modifyAction->m_vecInsertions)
								AddEntityInsert(keyValues, insert);
						}
					}
				}
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