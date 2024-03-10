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

enum ActionType_t
{
	Filter,
	Modify,
	Add
};

struct IOConnection
{
	std::string m_pszOutputName;
	EntityIOTargetType_t m_eTargetType;
	std::string m_pszTargetName;
	std::string m_pszInputName;
	std::string m_pszOverrideParam;
	float m_flDelay;
	int32 m_nTimesToFire;
};


struct ActionEntry
{
	ActionEntry() = default;
	~ActionEntry()
	{
		if (m_bIsRegex && m_pRegex)
			pcre2_code_free(m_pRegex);
	}

	ActionEntry(ActionEntry&& other) noexcept
		: m_nFlags(other.m_nFlags), m_strName(std::move(other.m_strName)), m_strValue(std::move(other.m_strValue)), m_IOConnection(std::move(other.m_IOConnection)), m_pRegex(other.m_pRegex)
	{
		other.m_pRegex = nullptr;
	}

	ActionEntry& operator=(ActionEntry&& other) noexcept
	{
		if (this != &other)
		{
			m_nFlags = other.m_nFlags;
			m_strName = std::move(other.m_strName);
			m_strValue = std::move(other.m_strValue);
			m_IOConnection = std::move(other.m_IOConnection);
			m_pRegex = other.m_pRegex;
			other.m_pRegex = nullptr;
		}
		return *this;
	}

	// fuck copying
	ActionEntry(const ActionEntry&) = delete;
	ActionEntry& operator=(const ActionEntry&) = delete;

	union
	{
		struct
		{
			bool m_bIsIO : 1;
			bool m_bIsRegex : 1;
		};
		uint8 m_nFlags;
	};
	std::string m_strName;
	std::string m_strValue;
	IOConnection m_IOConnection;
	pcre2_code* m_pRegex = nullptr;
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

bool DoesEntityMatch(CEntityKeyValues* keyValues, std::vector<ActionEntry>& m_vecMatches);