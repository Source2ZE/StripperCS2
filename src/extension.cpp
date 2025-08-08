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

#include <stdio.h>
#include "extension.h"
#include "iserver.h"
#include <funchook.h>
#include "hook.h"
#include <filesystem>
#include "providers/json_provider.h"
#include <spdlog/spdlog.h>
#include <spdlog/fmt/bundled/color.h>

StripperCS2 g_StripperCS2;
IServerGameDLL* server = NULL;
IServerGameClients* gameclients = NULL;
IVEngineServer* engine = NULL;
IGameEventManager2* gameevents = NULL;
ICvar* icvar = NULL;

// Should only be called within the active game loop (i e map should be loaded and active)
// otherwise that'll be nullptr!
CGlobalVars* GetGameGlobals()
{
	INetworkGameServer* server = g_pNetworkServerService->GetIGameServer();

	if (!server)
		return nullptr;

	return g_pNetworkServerService->GetIGameServer()->GetGlobals();
}

CGameEntitySystem* GameEntitySystem()
{
	return nullptr;
}

std::map<std::pair<std::string, std::string>, std::vector<std::unique_ptr<BaseAction>>> g_mapOverrides;

PLUGIN_EXPOSE(StripperCS2, g_StripperCS2);
bool StripperCS2::Load(PluginId id, ISmmAPI* ismm, char* error, size_t maxlen, bool late)
{
	spdlog::set_pattern("%^[%T] [StripperCS2] [%l] %v%$");

	PLUGIN_SAVEVARS();

	GET_V_IFACE_CURRENT(GetEngineFactory, engine, IVEngineServer, INTERFACEVERSION_VENGINESERVER);
	GET_V_IFACE_CURRENT(GetEngineFactory, icvar, ICvar, CVAR_INTERFACE_VERSION);
	GET_V_IFACE_ANY(GetServerFactory, server, IServerGameDLL, INTERFACEVERSION_SERVERGAMEDLL);
	GET_V_IFACE_ANY(GetServerFactory, gameclients, IServerGameClients, INTERFACEVERSION_SERVERGAMECLIENTS);
	GET_V_IFACE_ANY(GetEngineFactory, g_pNetworkServerService, INetworkServerService, NETWORKSERVERSERVICE_INTERFACE_VERSION);
	GET_V_IFACE_ANY(GetEngineFactory, g_pWorldRendererMgr, IWorldRendererMgr, WORLD_RENDERER_MGR_INTERFACE_VERSION);

	// Required to get the IMetamodListener events
	g_SMAPI->AddListener(this, this);

	if (!Hook::SetupHook())
	{
		spdlog::critical("Failed to setup hook!");
		return false;
	}

	g_pCVar = icvar;
	ConVar_Register(FCVAR_RELEASE | FCVAR_CLIENT_CAN_EXECUTE | FCVAR_GAMEDLL);

	spdlog::info("Stripper loaded");

	return true;
}

bool StripperCS2::Unload(char* error, size_t maxlen)
{
	Hook::Cleanup();

	return true;
}

void StripperCS2::AllPluginsLoaded()
{
}

void StripperCS2::OnLevelInit(char const* pMapName,
	char const* pMapEntities,
	char const* pOldLevel,
	char const* pLandmarkName,
	bool loadGame,
	bool background)
{
	g_mapOverrides.clear();

	std::filesystem::path path(Plat_GetGameDirectory());
	auto globalFilePath = path / "csgo/addons/StripperCS2/global.jsonc";

	if (std::filesystem::exists(globalFilePath))
	{
		Providers::JsonProvider provider;

		try {
			g_mapOverrides[std::make_pair("GLOBALOVERRIDE", "")] = provider.Load(globalFilePath.string());
		}
		catch (const std::exception& e)
		{
			spdlog::error("Provider failed to parse {}: {}", globalFilePath.string(), e.what());
		}
	}

	path /= "csgo/addons/StripperCS2/maps";
	path /= pMapName;

	if (!std::filesystem::exists(path))
	{
		spdlog::warn("No map overrides found for {}", pMapName);
		return;
	}

	for (const auto& entry : std::filesystem::recursive_directory_iterator(path))
	{
		if (entry.is_regular_file())
		{
			std::filesystem::path filePath = entry.path();

			std::filesystem::path cleanPath = filePath.lexically_relative(path);

			ConMsg("Loading: %s\n", cleanPath.string().c_str());

			if (filePath.extension() == ".jsonc")
			{
				std::string worldName = cleanPath.has_parent_path() ? cleanPath.parent_path().string() : pMapName;
				std::string lumpName = cleanPath.stem().string();

				std::transform(worldName.begin(), worldName.end(), worldName.begin(), [](unsigned char c) { return std::tolower(c); });
				std::transform(lumpName.begin(), lumpName.end(), lumpName.begin(), [](unsigned char c) { return std::tolower(c); });

				ConMsg("%s %s\n", worldName.c_str(), lumpName.c_str());
				ConMsg("%s\n", cleanPath.string().c_str());

				Providers::JsonProvider provider;

				try {
					g_mapOverrides[std::make_pair(worldName, lumpName)] = provider.Load(filePath.string());
				}
				catch (const std::exception& e)
				{
					spdlog::error("Provider failed to parse {}: {}", filePath.string(), e.what());
				}
			}
		}
	}

	META_CONPRINTF("OnLevelInit(%s)\n", pMapName);
}

void StripperCS2::OnLevelShutdown()
{
	META_CONPRINTF("OnLevelShutdown()\n");
}

bool StripperCS2::Pause(char* error, size_t maxlen)
{
	return true;
}

bool StripperCS2::Unpause(char* error, size_t maxlen)
{
	return true;
}

const char* StripperCS2::GetLicense()
{
	return "GPLv3";
}

const char* StripperCS2::GetVersion()
{
	return "1.0.8";
}

const char* StripperCS2::GetDate()
{
	return __DATE__;
}

const char* StripperCS2::GetLogTag()
{
	return "STRIPPER";
}

const char* StripperCS2::GetAuthor()
{
	return "Poggu";
}

const char* StripperCS2::GetDescription()
{
	return "CS2 Map Lump Editor";
}

const char* StripperCS2::GetName()
{
	return "StripperCS2";
}

const char* StripperCS2::GetURL()
{
	return "https://poggu.me";
}