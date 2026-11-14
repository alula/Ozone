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
#include "ro_debug_monitor_service.hpp"
#include "impl/ro_service_impl.hpp"
#include "impl/ro_ns2jb_service_impl.hpp"

namespace ams::ro {

    Result DebugMonitorService::GetProcessModuleInfo(sf::Out<u32> out_count, const sf::OutArray<ldr::ModuleInfo> &out_infos, os::ProcessId process_id) {
        R_UNLESS(out_infos.GetSize() <= std::numeric_limits<s32>::max(), ro::ResultInvalidSize());
        R_RETURN(impl::GetProcessModuleInfo(out_count.GetPointer(), out_infos.GetPointer(), out_infos.GetSize(), process_id));
    }

    #if defined(ATMOSPHERE_BOARD_NINTENDO_OUNCE)
    Result DebugMonitorService::ProbeExecution(sf::Out<impl::ProbeReply> out, u64 nonce) {
        R_RETURN(impl::ProbeExecution(out.GetPointer(), nonce));
    }

    Result DebugMonitorService::QueryProcessInfo(sf::Out<impl::ProcessInfoReply> out, os::ProcessId process_id, u64 address, sf::CopyHandle &&process_h) {
        R_RETURN(impl::QueryProcessInfo(out.GetPointer(), process_h.GetOsHandle(), process_id, address));
    }

    Result DebugMonitorService::ReadProcessMemory(sf::Out<impl::ProcessReadReply> out, os::ProcessId process_id, u64 source, u64 length, sf::CopyHandle &&process_h) {
        R_RETURN(impl::ReadProcessMemory(out.GetPointer(), process_h.GetOsHandle(), process_id, source, length));
    }

    Result DebugMonitorService::MapProcessCodeMemory(sf::Out<impl::ProcessCodeMapReply> out, os::ProcessId process_id, u64 source, u64 destination, u64 size, sf::CopyHandle &&process_h) {
        R_RETURN(impl::MapProcessCodeMemory(out.GetPointer(), process_h.GetOsHandle(), process_id, source, destination, size));
    }

    Result DebugMonitorService::UnmapProcessCodeMemory(sf::Out<impl::ProcessCodeMapReply> out, os::ProcessId process_id, u64 source, u64 destination, u64 size, sf::CopyHandle &&process_h) {
        R_RETURN(impl::UnmapProcessCodeMemory(out.GetPointer(), process_h.GetOsHandle(), process_id, source, destination, size));
    }

    Result DebugMonitorService::PatchProcessCodeMemory(sf::Out<impl::ProcessPatchReply> out, os::ProcessId process_id, u64 address, u64 size, impl::ProcessPatchInput patch, sf::CopyHandle &&process_h) {
        R_RETURN(impl::PatchProcessCodeMemory(out.GetPointer(), process_h.GetOsHandle(), process_id, address, size, patch));
    }

    Result DebugMonitorService::SetProcessMemoryPermission(sf::Out<impl::ProcessPermissionReply> out, os::ProcessId process_id, u64 address, u64 size, u64 permission, sf::CopyHandle &&process_h) {
        R_RETURN(impl::SetProcessMemoryPermission(out.GetPointer(), process_h.GetOsHandle(), process_id, address, size, permission));
    }

    Result DebugMonitorService::BulkReadProcessMemory(sf::Out<impl::ProcessBulkReadReply> out, const sf::OutMapAliasBuffer &receive_buf, os::ProcessId process_id, u64 source, u64 length, sf::CopyHandle &&process_h) {
        R_RETURN(impl::BulkReadProcessMemory(out.GetPointer(), receive_buf.GetPointer(), receive_buf.GetSize(), process_h.GetOsHandle(), process_id, source, length));
    }
    #endif

}
