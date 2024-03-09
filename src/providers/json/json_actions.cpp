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

#include <json.hpp>
#include "actions/actions.h"
#include "providers/json_provider.h"

using json = nlohmann::json;

namespace Providers::JSON
{

void ParseEntry(nlohmann::detail::iteration_proxy<nlohmann::json::const_iterator>&& items, std::vector<ActionEntry>& vecEntries, bool isIOArray)
{
	for (auto& [key, value] : items)
	{
		if (key == "io")
		{
			auto ParseIOFields = [&vecEntries](const json& j) {
				ActionEntry entry;
				entry.m_bIsIO = true;

				IOConnection ioDesc;
				for (auto& [ioKey, ioValue] : j.items())
				{
					if (ioKey == "inputname")
						ioDesc.m_pszInputName = ioValue.get<std::string>();
					else if (ioKey == "outputname")
						ioDesc.m_pszOutputName = ioValue.get<std::string>();
					else if (ioKey == "targetname")
						ioDesc.m_pszTargetName = ioValue.get<std::string>();
					else if (ioKey == "delay")
						ioDesc.m_flDelay = ioValue.get<float>();
				}

				entry.m_IOConnection = std::move(ioDesc);
				vecEntries.push_back(std::move(entry));
			};

			if (isIOArray)
			{
				for (auto& io : value)
					ParseIOFields(io);
			}
			else
				ParseIOFields(value);
		}
		else
		{
			ActionEntry entry;
			entry.m_bIsIO = false;
			entry.m_strName = key;
			entry.m_strValue = value.get<std::string>();
			vecEntries.push_back(std::move(entry));
		}
	}
}

void ParseFilters(const json& j, std::vector<std::unique_ptr<BaseAction>>& actions)
{
	auto& filter = j["filter"];
	for (auto& filterAction : filter)
	{
		auto action = std::make_unique<FilterAction>();

		ParseEntry(filterAction.items(), action->m_vecMatches);

		actions.push_back(std::move(action));
	}
}

void ParseModify(const json& j, std::vector<std::unique_ptr<BaseAction>>& actions)
{
	auto& filter = j["modify"];
	for (auto& modifyAction : filter)
	{
		auto action = std::make_unique<ModifyAction>();

		for (auto& [key, value] : modifyAction.items())
		{
			if (key == "match")
				ParseEntry(value.items(), action->m_vecMatches);
			else if (key == "replace")
				ParseEntry(value.items(), action->m_vecReplacements, false);
			else if (key == "delete")
				ParseEntry(value.items(), action->m_vecDeletions);
			else if (key == "insert")
				ParseEntry(value.items(), action->m_vecInsertions);
		}


		actions.push_back(std::move(action));
	}
}

void ParseAdd(const json& j, std::vector<std::unique_ptr<BaseAction>>& actions)
{
	auto& filter = j["add"];
	for (auto& addAction : filter)
	{
		auto action = std::make_unique<AddAction>();

		ParseEntry(addAction.items(), action->m_vecInsertions);

		actions.push_back(std::move(action));
	}
}

} // namespace Providers::JSON