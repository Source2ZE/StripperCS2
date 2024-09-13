/**
 * =============================================================================
 * StripperCS2
 * Copyright (C) 2023 Source2ZE
 * =============================================================================
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License, version 3.0, as published by the
 * Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "entity2/entitykeyvalues.h"
#include "actions/actions.h"
#include <vector>
#include <pcre/pcre2.h>
#include <spdlog/spdlog.h>

bool DoesValueMatch(const char* value, const ActionVariant_t& variant)
{
	if (auto str = std::get_if<std::string>(&variant))
	{
		// string compare
		return V_strcmp(value, str->c_str()) == 0;
	}
	else if (auto pRegex = std::get_if<pcre2_code*>(&variant))
	{
		// regex compare
		pcre2_match_data* match_data = pcre2_match_data_create_from_pattern(*pRegex, NULL);
		int rc = pcre2_match(*pRegex, (PCRE2_SPTR)value, PCRE2_ZERO_TERMINATED, 0, 0, match_data, NULL);

		pcre2_match_data_free(match_data);

		return rc >= 0;
	}

	return false;
}

bool DoesValueMatch(const char* value, const IOConnectionVariant_t& variant)
{
	// if the io field is missing, treat it as match
	if (std::holds_alternative<std::monostate>(variant))
		return true;
	else if (auto str = std::get_if<std::string>(&variant))
	{
		// string compare
		return V_strcmp(value, str->c_str()) == 0;
	}
	else if (auto pRegex = std::get_if<pcre2_code*>(&variant))
	{
		// regex compare
		pcre2_match_data* match_data = pcre2_match_data_create_from_pattern(*pRegex, NULL);
		int rc = pcre2_match(*pRegex, (PCRE2_SPTR)value, PCRE2_ZERO_TERMINATED, 0, 0, match_data, NULL);

		pcre2_match_data_free(match_data);

		return rc >= 0;
	}

	return false;
}

bool DoesConnectionMatch(const EntityIOConnectionDescFat_t* connectionDesc, const IOConnection* matchConnection)
{
	if (!DoesValueMatch(connectionDesc->m_pszInputName, matchConnection->m_pszInputName))
		return false;

	if (!DoesValueMatch(connectionDesc->m_pszOutputName, matchConnection->m_pszOutputName))
		return false;

	if (!DoesValueMatch(connectionDesc->m_pszTargetName, matchConnection->m_pszTargetName))
		return false;

	if (!DoesValueMatch(connectionDesc->m_pszOverrideParam, matchConnection->m_pszOverrideParam))
		return false;

	if (matchConnection->m_flDelay.has_value() && connectionDesc->m_flDelay != matchConnection->m_flDelay.value())
		return false;

	if (matchConnection->m_nTimesToFire.has_value() && connectionDesc->m_nTimesToFire != matchConnection->m_nTimesToFire.value())
		return false;

	return true;
}

bool DoesEntityMatch(CEntityKeyValues* keyValues, std::vector<ActionEntry>& m_vecMatches, std::vector<int>* m_vecMatchedIO = nullptr)
{
	for (const auto& match : m_vecMatches)
	{
		if (auto io = std::get_if<IOConnection>(&match.m_Value))
		{
			int num = keyValues->GetNumConnectionDescs();

			if (!num)
				return false;

			bool found = false;
			for (int i = 0; i < num; i++)
			{
				auto connectionDesc = keyValues->GetConnectionDesc(i);

				if (connectionDesc && DoesConnectionMatch(connectionDesc, io))
				{
					if(m_vecMatchedIO)
						m_vecMatchedIO->push_back(i);
					found = true;
				}
			}

			if (!found)
				return false;
		}
		else
		{
			if (!keyValues->HasValue(match.m_strName.c_str()))
				return false;

			auto propValue = keyValues->GetString(match.m_strName.c_str());
			if (!DoesValueMatch(propValue, match.m_Value))
				return false;
		}
	}

	return true;
}

void AddEntityInsert(CEntityKeyValues* keyValues, const ActionEntry& entry)
{
	if (auto io = std::get_if<IOConnection>(&entry.m_Value))
	{
		auto outputName = VariantOrDefault<std::string>(io->m_pszOutputName, "");
		auto targetName = VariantOrDefault<std::string>(io->m_pszTargetName, "");
		auto inputName = VariantOrDefault<std::string>(io->m_pszInputName, "");
		auto overrideParam = VariantOrDefault<std::string>(io->m_pszOverrideParam, "");
		auto delay = io->m_flDelay.value_or(0);
		auto timesToFire = io->m_nTimesToFire.value_or(-1);
		auto targetType = io->m_eTargetType.value_or(ENTITY_IO_TARGET_ENTITYNAME_OR_CLASSNAME); // Default value used by Hammer IO

		spdlog::info("Created IO {} {} {} {} {} {} {}", outputName.c_str(), targetType, targetName.c_str(), inputName.c_str(), overrideParam.c_str(), delay, timesToFire);
		keyValues->AddConnectionDesc(outputName.c_str(), targetType, targetName.c_str(), inputName.c_str(), overrideParam.c_str(), delay, timesToFire);
	}
	else if (auto str = std::get_if<std::string>(&entry.m_Value))
	{
		keyValues->SetString(entry.m_strName.c_str(), str->c_str());
	}
}

void ApplyMapOverride(std::vector<std::unique_ptr<BaseAction>>& actions, CUtlVector<CEntityKeyValues*>* vecEntityKeyValues, LumpData* lumpData)
{
	for (const auto& action : actions)
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
				std::vector<int> vecMatchedIO;
				auto keyValues = (*vecEntityKeyValues)[j];
				if (!DoesEntityMatch(keyValues, modifyAction->m_vecMatches, &vecMatchedIO))
					continue;

				for (auto& replace : modifyAction->m_vecReplacements)
				{
					if (auto io = std::get_if<IOConnection>(&replace.m_Value))
					{
						int num = keyValues->GetNumConnectionDescs();
						int removed = 0;
						for (int i = 0; i < num; i++)
						{
							if (auto connectionDesc = keyValues->GetConnectionDesc(i - removed))
							{
								// checks if this IO entry is matched by our previous modify match
								if (std::find(vecMatchedIO.begin(), vecMatchedIO.end(), i) == vecMatchedIO.end())
									continue;

								auto outputName = VariantOrDefault<std::string>(io->m_pszOutputName, connectionDesc->m_pszOutputName);
								auto targetName = VariantOrDefault<std::string>(io->m_pszTargetName, connectionDesc->m_pszTargetName);
								auto inputName = VariantOrDefault<std::string>(io->m_pszInputName, connectionDesc->m_pszInputName);
								auto overrideParam = VariantOrDefault<std::string>(io->m_pszOverrideParam, connectionDesc->m_pszOverrideParam);
								auto delay = io->m_flDelay.value_or(connectionDesc->m_flDelay);
								auto timesToFire = io->m_nTimesToFire.value_or(connectionDesc->m_nTimesToFire);
								auto targetType = io->m_eTargetType.value_or(connectionDesc->m_eTargetType);

								keyValues->RemoveConnectionDesc(i - removed);

								keyValues->AddConnectionDesc(outputName.c_str(), targetType, targetName.c_str(), inputName.c_str(), overrideParam.c_str(), delay, timesToFire);
								removed++;
							}
						}
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
						for (int i = 0; i < keyValues->GetNumConnectionDescs(); i++)
						{
							if (auto connectionDesc = keyValues->GetConnectionDesc(i))
							{
								spdlog::warn("Connection: {}", connectionDesc->m_pszOutputName);
								if (!DoesConnectionMatch(connectionDesc, io))
									continue;

								keyValues->RemoveConnectionDesc(i);
								i--;
							}
						}
					}
					else if (!std::holds_alternative<std::monostate>(_delete.m_Value))
						if (keyValues->HasValue(_delete.m_strName.c_str()) && DoesValueMatch(keyValues->GetString(_delete.m_strName.c_str()), _delete.m_Value))
							keyValues->RemoveKeyValue(_delete.m_strName.c_str());
				}

				for (const auto& insert : modifyAction->m_vecInsertions)
					AddEntityInsert(keyValues, insert);
			}
		}
	}
}
