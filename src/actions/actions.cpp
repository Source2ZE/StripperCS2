#include "entity2/entitykeyvalues.h"
#include "actions/actions.h"
#include <vector>
#include <pcre/pcre2.h>

bool DoesEntityMatch(CEntityKeyValues* keyValues, std::vector<ActionEntry>& m_vecMatches)
{
	for (const auto& match : m_vecMatches)
	{
		if (match.m_bIsIO)
		{
		}
		else
		{
			if (!keyValues->HasValue(match.m_strName.c_str()))
				return false;

			auto propValue = keyValues->GetString(match.m_strName.c_str());
			if(match.m_bIsRegex)
			{
				// regex compare
				pcre2_match_data* match_data = pcre2_match_data_create_from_pattern(match.m_pRegex, NULL);
				int rc = pcre2_match(match.m_pRegex, (PCRE2_SPTR)propValue, PCRE2_ZERO_TERMINATED, 0, 0, match_data, NULL);

				pcre2_match_data_free(match_data);

				return rc >= 0;
			}
			else
			{
				// string compare
				if (V_strcmp(propValue, match.m_strValue.c_str()) != 0)
					return false;
			}
		}
	}

	return true;
}