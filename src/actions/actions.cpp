#include "entity2/entitykeyvalues.h"
#include "actions/actions.h"
#include <vector>
#include <pcre/pcre2.h>

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

bool DoesEntityMatch(CEntityKeyValues* keyValues, std::vector<ActionEntry>& m_vecMatches)
{
	for (const auto& match : m_vecMatches)
	{
		if (auto io = std::get_if<IOConnection>(&match.m_Value))
		{
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