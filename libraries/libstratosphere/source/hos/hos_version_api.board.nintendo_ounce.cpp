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
#include <stratosphere.hpp>

namespace ams::hos {

    namespace {

        constinit os::SdkMutex g_hos_init_lock;
        constinit hos::Version g_hos_version;
        constinit bool g_set_hos_version;

    }

    void InitializeVersionInternal(bool allow_approximate) {
        AMS_UNUSED(allow_approximate);
        hos::Version current = hos::Version_Current;

        /* TODO: how to implement this on Ounce? */
        g_hos_version     = hos::Version_Current;
        g_set_hos_version = true;

        #if defined(ATMOSPHERE_OS_HORIZON)
        /* Set the version for libnx. */
        {
            const u32 major = (static_cast<u32>(current) >> 24) & 0xFF;
            const u32 minor = (static_cast<u32>(current) >> 16) & 0xFF;
            const u32 micro = (static_cast<u32>(current) >>  8) & 0xFF;
            hosversionSet((BIT(31)) | (MAKEHOSVERSION(major, minor, micro)));
        }
        #endif
    }

    ::ams::hos::Version GetVersion() {
        return g_hos_version;
    }

}
