/*
 * Copyright (c) Atmosphère-NX
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once
#include <vapours.hpp>

namespace ams::ncm {

    enum class PlatformId : u8 {
        None    = 0x00,
        Nx      = 0x01,
        Ounce   = 0x04,
        Invalid = 0xFF,
    };


    constexpr inline u64 PlatformIdShift = 56;
    constexpr inline u64 PlatformIdMask = 0xFF00000000000000ULL;
    constexpr inline u64 ContentIdMask  = 0x00FFFFFFFFFFFFFFULL;

    namespace impl {

        #if defined(ATMOSPHERE_BOARD_NINTENDO_NX)
        inline constexpr PlatformId CurrentPlatformId = PlatformId::Nx;
        #elif defined(ATMOSPHERE_BOARD_NINTENDO_OUNCE)
        inline constexpr PlatformId CurrentPlatformId = PlatformId::Ounce;
        #else
        inline constexpr PlatformId CurrentPlatformId = PlatformId::Invalid;
        #endif

    }

}
