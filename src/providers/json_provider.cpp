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

#include "json_provider.h"
#include "json.hpp"
#include "actions/actions.h"
#include <fstream>

namespace Providers
{

std::vector<std::unique_ptr<BaseAction>> JsonProvider::Load(const std::string& path)
{
	std::ifstream i(path);

	if (!i.is_open())
		throw std::runtime_error("Failed to open file: " + path);

	json::parse(i, std::bind(&JsonProvider::ParserCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), true, true);
	i.close();

	std::vector<std::unique_ptr<BaseAction>> actions;

	if (m_json.find("filter") != m_json.end())
		JSON::ParseFilters(m_json, actions);

	if (m_json.find("add") != m_json.end())
		JSON::ParseAdd(m_json, actions);

	if (m_json.find("modify") != m_json.end())
		JSON::ParseModify(m_json, actions);

	return actions;

}

// Our custom JSON parser, which retains the full config even when duplicate top-level keys are used (default parser does not do this)
bool JsonProvider::ParserCallback(int depth, json::parse_event_t event, json& parsed)
{
	if (depth != 1)
		return true;

	if (event == json::parse_event_t::key)
	{
		m_sCurrentKey = parsed.get<std::string>();
	}
	else if (event == json::parse_event_t::array_end)
	{
		for (json item : parsed)
			m_json[m_sCurrentKey].push_back(item);
	}

	return true;
}

} // namespace Providers