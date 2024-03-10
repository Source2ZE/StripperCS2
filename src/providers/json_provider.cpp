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

#include "json_provider.h"
#include "json.hpp"
#include "actions/actions.h"
#include <fstream>

using json = nlohmann::json;

namespace Providers
{

std::vector<std::unique_ptr<BaseAction>> JsonProvider::Load(const std::string& path)
{
	nlohmann::json j;
	std::ifstream i(path);

	if (!i.is_open())
		throw std::runtime_error("Failed to open file: " + path);

	i >> j;
	i.close();

	std::vector<std::unique_ptr<BaseAction>> actions;

	if (j.find("filter") != j.end())
		JSON::ParseFilters(j, actions);

	if (j.find("add") != j.end())
		JSON::ParseAdd(j, actions);

	if (j.find("modify") != j.end())
		JSON::ParseModify(j, actions);

	return actions;

}

} // namespace Providers