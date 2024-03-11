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

#pragma once
#include <string>
#include <vector>
#include "entitykeyvalues.h"
#include "pcre/pcre2.h"
#include <variant>
#include <memory>
#include <optional>

struct IOConnection;

typedef std::variant<std::monostate, std::string, IOConnection, pcre2_code*> ActionVariant_t;
typedef std::variant<std::monostate, std::string, pcre2_code*> IOConnectionVariant_t;

enum ActionType_t
{
	Filter,
	Modify,
	Add
};

struct IOConnection
{
	IOConnection() = default;
	~IOConnection()
	{
		if (auto pRegex = std::get_if<pcre2_code*>(&m_pszOutputName))
			pcre2_code_free(*pRegex);

		if (auto pRegex = std::get_if<pcre2_code*>(&m_pszTargetName))
			pcre2_code_free(*pRegex);

		if (auto pRegex = std::get_if<pcre2_code*>(&m_pszInputName))
			pcre2_code_free(*pRegex);

		if (auto pRegex = std::get_if<pcre2_code*>(&m_pszOverrideParam))
			pcre2_code_free(*pRegex);
	}

	IOConnection(IOConnection&& other) noexcept : m_pszOutputName(std::move(other.m_pszOutputName)), m_eTargetType(std::move(other.m_eTargetType)),
		m_pszTargetName(std::move(other.m_pszTargetName)), m_pszInputName(std::move(other.m_pszInputName)),
		m_pszOverrideParam(std::move(other.m_pszOverrideParam)), m_flDelay(std::move(other.m_flDelay)), m_nTimesToFire(std::move(other.m_nTimesToFire))
	{
		other.m_pszOutputName = std::monostate{};
		other.m_pszTargetName = std::monostate{};
		other.m_pszInputName = std::monostate{};
		other.m_pszOverrideParam = std::monostate{};
	}

	IOConnection& operator=(IOConnection&& other) noexcept
	{
		if (this != &other)
		{
			m_pszOutputName = std::move(other.m_pszOutputName);
			m_eTargetType = std::move(other.m_eTargetType);
			m_flDelay = std::move(other.m_flDelay);
			m_nTimesToFire = std::move(other.m_nTimesToFire);
			m_pszTargetName = std::move(other.m_pszOutputName);
			m_pszInputName = std::move(other.m_pszOutputName);
			m_pszOverrideParam = std::move(other.m_pszOutputName);
			other.m_pszOutputName = std::monostate{};
			other.m_pszTargetName = std::monostate{};
			other.m_pszInputName = std::monostate{};
			other.m_pszOverrideParam = std::monostate{};
		}
		return *this;
	}

	// fuck copying
	IOConnection(const IOConnection&) = delete;
	IOConnection& operator=(const IOConnection&) = delete;

	IOConnectionVariant_t m_pszOutputName;
	std::optional<EntityIOTargetType_t> m_eTargetType;
	IOConnectionVariant_t m_pszTargetName;
	IOConnectionVariant_t m_pszInputName;
	IOConnectionVariant_t m_pszOverrideParam;
	std::optional<float> m_flDelay;
	std::optional<int32> m_nTimesToFire;
};


struct ActionEntry
{
	ActionEntry() = default;
	~ActionEntry()
	{
		if (auto pRegex = std::get_if<pcre2_code*>(&m_Value))
			pcre2_code_free(*pRegex);
	}

	ActionEntry(ActionEntry&& other) noexcept : m_strName(std::move(other.m_strName)), m_Value(std::move(other.m_Value))
	{
		other.m_Value = std::monostate{};
	}

	ActionEntry& operator=(ActionEntry&& other) noexcept
	{
		if (this != &other)
		{
			m_strName = std::move(other.m_strName);
			m_Value = std::move(other.m_Value);
			other.m_Value = std::monostate{};
		}
		return *this;
	}

	// fuck copying
	ActionEntry(const ActionEntry&) = delete;
	ActionEntry& operator=(const ActionEntry&) = delete;

	std::string m_strName;
	ActionVariant_t m_Value;
};

class BaseAction
{
public:
	virtual ~BaseAction() = default;
	virtual ActionType_t GetType() const = 0;
};

class FilterAction : public BaseAction
{
public:
	FilterAction() = default;
	ActionType_t GetType() const override { return ActionType_t::Filter; }
public:
	std::vector<ActionEntry> m_vecMatches;
};

class ModifyAction : public BaseAction
{
public:
	ModifyAction() = default;
	ActionType_t GetType() const override { return ActionType_t::Modify; }
public:
	std::vector<ActionEntry> m_vecMatches;
	std::vector<ActionEntry> m_vecReplacements;
	std::vector<ActionEntry> m_vecDeletions;
	std::vector<ActionEntry> m_vecInsertions;
};

class AddAction : public BaseAction
{
public:
	AddAction() = default;
	ActionType_t GetType() const override { return ActionType_t::Add; }
public:
	std::vector<ActionEntry> m_vecInsertions;
};

bool DoesValueMatch(const char* value, const ActionVariant_t& variant);
bool DoesEntityMatch(CEntityKeyValues* keyValues, std::vector<ActionEntry>& m_vecMatches);
void AddEntityInsert(CEntityKeyValues* keyValues, const ActionEntry& entry);
bool DoesConnectionMatch(const EntityIOConnectionDescFat_t* connectionDesc, const IOConnection* matchConnection);

template <typename T, typename V>
T VariantOrDefault(V variant, T defaultValue)
{
	if (auto val = std::get_if<T>(&variant))
		return *val;

	return defaultValue;
}

struct LumpData
{
	CUtlString m_name;
	char pad[0x20];
	CKeyValues3Context* m_allocatorContext;
};

void ApplyMapOverride(std::vector<std::unique_ptr<BaseAction>>& actions, CUtlVector<CEntityKeyValues*>* vecEntityKeyValues, LumpData* lumpData);