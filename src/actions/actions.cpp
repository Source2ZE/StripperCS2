#include "entity2/entitykeyvalues.h"
#include "actions/actions.h"
#include <vector>
#include <pcre/pcre2.h>
#include <spdlog/spdlog.h>

template <typename T, typename V>
T VariantOrDefault(V variant, T defaultValue)
{
	if (auto val = std::get_if<T>(&variant))
		return *val;

	return defaultValue;
}

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

bool DoesEntityMatch(CEntityKeyValues* keyValues, std::vector<ActionEntry>& m_vecMatches)
{
	for (const auto& match : m_vecMatches)
	{
		if (auto io = std::get_if<IOConnection>(&match.m_Value))
		{
			int num = keyValues->GetNumConnectionDescs();

			if (!num)
				return false;

			for (int i = 0; i < num; i++)
			{
				auto connectionDesc = keyValues->GetConnectionDesc(i);

				if (connectionDesc)
				{
					spdlog::warn("Connection: {}", connectionDesc->m_pszOutputName);
					if (!DoesValueMatch(connectionDesc->m_pszInputName, io->m_pszInputName))
						return false;

					if (!DoesValueMatch(connectionDesc->m_pszOutputName, io->m_pszOutputName))
						return false;

					if (!DoesValueMatch(connectionDesc->m_pszTargetName, io->m_pszTargetName))
						return false;

					if (!DoesValueMatch(connectionDesc->m_pszOverrideParam, io->m_pszOverrideParam))
						return false;

					if(io->m_flDelay.has_value() && connectionDesc->m_flDelay != io->m_flDelay.value())
						return false;

					if (io->m_nTimesToFire.has_value() && connectionDesc->m_flDelay != io->m_flDelay.value())
						return false;
				}
			}
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
		auto timesToFire = io->m_nTimesToFire.value_or(0);
		auto targetType = io->m_eTargetType.value_or(ENTITY_IO_TARGET_INVALID);

		spdlog::info("Created IO {} {} {} {} {} {} {}", outputName.c_str(), targetType, targetName.c_str(), inputName.c_str(), overrideParam.c_str(), delay, timesToFire);
		keyValues->AddConnectionDesc(outputName.c_str(), targetType, targetName.c_str(), inputName.c_str(), overrideParam.c_str(), delay, timesToFire);
	}
	else if (auto str = std::get_if<std::string>(&entry.m_Value))
	{
		keyValues->SetString(entry.m_strName.c_str(), str->c_str());
	}
}