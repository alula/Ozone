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
#include <stratosphere.hpp>

namespace ams::ro {

    class DebugMonitorService {
        public:
            Result GetProcessModuleInfo(sf::Out<u32> out_count, const sf::OutArray<ldr::ModuleInfo> &out_infos, os::ProcessId process_id);

            #if defined(ATMOSPHERE_BOARD_NINTENDO_OUNCE)
            Result ProbeExecution(sf::Out<impl::ProbeReply> out, u64 nonce);
            Result QueryProcessInfo(sf::Out<impl::ProcessInfoReply> out, os::ProcessId process_id, u64 address, sf::CopyHandle &&process_h);
            Result ReadProcessMemory(sf::Out<impl::ProcessReadReply> out, os::ProcessId process_id, u64 source, u64 length, sf::CopyHandle &&process_h);
            Result MapProcessCodeMemory(sf::Out<impl::ProcessCodeMapReply> out, os::ProcessId process_id, u64 source, u64 destination, u64 size, sf::CopyHandle &&process_h);
            Result UnmapProcessCodeMemory(sf::Out<impl::ProcessCodeMapReply> out, os::ProcessId process_id, u64 source, u64 destination, u64 size, sf::CopyHandle &&process_h);
            Result PatchProcessCodeMemory(sf::Out<impl::ProcessPatchReply> out, os::ProcessId process_id, u64 address, u64 size, impl::ProcessPatchInput patch, sf::CopyHandle &&process_h);
            Result SetProcessMemoryPermission(sf::Out<impl::ProcessPermissionReply> out, os::ProcessId process_id, u64 address, u64 size, u64 permission, sf::CopyHandle &&process_h);
            Result BulkReadProcessMemory(sf::Out<impl::ProcessBulkReadReply> out, const sf::OutMapAliasBuffer &receive_buf, os::ProcessId process_id, u64 source, u64 length, sf::CopyHandle &&process_h);
            #endif
    };
    static_assert(ro::impl::IsIDebugMonitorInterface<DebugMonitorService>);

}
