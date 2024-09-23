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

#pragma once

#include "base_provider.h"
#include "actions/actions.h"
#include <string>
#include <json.hpp>

using json = nlohmann::json;

namespace Providers
{

class JsonProvider : public BaseProvider
{
public:
	JsonProvider() = default;
	~JsonProvider() = default;

	std::vector<std::unique_ptr<BaseAction>> Load(const std::string& path);
	bool ParserCallback(int depth, json::parse_event_t event, json& parsed);
private:
	std::string m_sCurrentKey;
	json m_json;
};

namespace JSON
{

	void ParseEntry(nlohmann::detail::iteration_proxy<nlohmann::json::const_iterator>&& items, std::vector<ActionEntry>& vecEntries, bool isIOArray = true);
	void ParseFilters(const nlohmann::json& j, std::vector<std::unique_ptr<BaseAction>>& actions);
	void ParseModify(const nlohmann::json& j, std::vector<std::unique_ptr<BaseAction>>& actions);
	void ParseAdd(const nlohmann::json& j, std::vector<std::unique_ptr<BaseAction>>& actions);

} // namespace JSON

} // namespace Providers