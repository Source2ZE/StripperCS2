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

#include <funchook.h>

namespace Hook
{

typedef int (*WorldInit_t)(void* pWorld, void* pSceneWorld, int a3, int a4);
inline WorldInit_t g_pWorldInit = nullptr;
inline funchook_t* g_pHook = nullptr;

void Detour_WorldInit(void* pWorld, void* pSceneWorld, int a3, int a4);
bool SetupHook();
void Cleanup();

} // namespace Hook