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

#include <stratosphere/sm/sm_types.hpp>

namespace ams::sm {

    /* Initialization. */
    Result Initialize();
    Result Finalize();

    /* Ordinary SM API. */
    Result GetServiceHandle(os::NativeHandle *out, ServiceName name);
    Result RegisterService(os::NativeHandle *out, ServiceName name, size_t max_sessions, bool is_light);
    #if defined(ATMOSPHERE_BOARD_NINTENDO_OUNCE)
    Result RegisterServiceInSpace(os::NativeHandle *out, u32 space, ServiceName name, size_t max_sessions, bool is_light);
    #endif
    Result UnregisterService(ServiceName name);

    /* Atmosphere extensions. */
    Result HasService(bool *out, ServiceName name);
    Result WaitService(ServiceName name);

}
