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
#include <stratosphere/ro/ro_types.hpp>
#include <stratosphere/ldr/ldr_types.hpp>
#include <stratosphere/sf.hpp>

#if defined(ATMOSPHERE_BOARD_NINTENDO_OUNCE)
#define AMS_RO_I_DEBUG_MONITOR_INTERFACE_OUNCE_INTERFACE_INFO(C, H)                                                                                                                                                                                                                                                               \
    AMS_SF_METHOD_INFO(C, H, 0x524F0001, Result, ProbeExecution,             (sf::Out<ams::ro::impl::ProbeReply> out, u64 nonce),                                                                                                                (out, nonce))                                                    \
    AMS_SF_METHOD_INFO(C, H, 0x524F0010, Result, QueryProcessInfo,           (sf::Out<ams::ro::impl::ProcessInfoReply> out, os::ProcessId process_id, u64 address, sf::CopyHandle &&process_h),                                                  (out, process_id, address, std::move(process_h)))                \
    AMS_SF_METHOD_INFO(C, H, 0x524F0011, Result, ReadProcessMemory,          (sf::Out<ams::ro::impl::ProcessReadReply> out, os::ProcessId process_id, u64 source, u64 length, sf::CopyHandle &&process_h),                                       (out, process_id, source, length, std::move(process_h)))         \
    AMS_SF_METHOD_INFO(C, H, 0x524F0012, Result, MapProcessCodeMemory,       (sf::Out<ams::ro::impl::ProcessCodeMapReply> out, os::ProcessId process_id, u64 source, u64 destination, u64 size, sf::CopyHandle &&process_h),                     (out, process_id, source, destination, size, std::move(process_h))) \
    AMS_SF_METHOD_INFO(C, H, 0x524F0013, Result, UnmapProcessCodeMemory,     (sf::Out<ams::ro::impl::ProcessCodeMapReply> out, os::ProcessId process_id, u64 source, u64 destination, u64 size, sf::CopyHandle &&process_h),                     (out, process_id, source, destination, size, std::move(process_h))) \
    AMS_SF_METHOD_INFO(C, H, 0x524F0014, Result, PatchProcessCodeMemory,     (sf::Out<ams::ro::impl::ProcessPatchReply> out, os::ProcessId process_id, u64 address, u64 size, ams::ro::impl::ProcessPatchInput patch, sf::CopyHandle &&process_h), (out, process_id, address, size, patch, std::move(process_h)))   \
    AMS_SF_METHOD_INFO(C, H, 0x524F0015, Result, SetProcessMemoryPermission, (sf::Out<ams::ro::impl::ProcessPermissionReply> out, os::ProcessId process_id, u64 address, u64 size, u64 permission, sf::CopyHandle &&process_h),                  (out, process_id, address, size, permission, std::move(process_h))) \
    AMS_SF_METHOD_INFO(C, H, 0x524F0016, Result, BulkReadProcessMemory,      (sf::Out<ams::ro::impl::ProcessBulkReadReply> out, const sf::OutMapAliasBuffer &receive_buf, os::ProcessId process_id, u64 source, u64 length, sf::CopyHandle &&process_h), (out, receive_buf, process_id, source, length, std::move(process_h)))
#else
#define AMS_RO_I_DEBUG_MONITOR_INTERFACE_OUNCE_INTERFACE_INFO(C, H)
#endif

#define AMS_RO_I_DEBUG_MONITOR_INTERFACE_INTERFACE_INFO(C, H)                                                                                                                                          \
    AMS_SF_METHOD_INFO(C, H, 0, Result, GetProcessModuleInfo, (sf::Out<u32> out_count, const sf::OutArray<ldr::ModuleInfo> &out_infos, os::ProcessId process_id), (out_count, out_infos, process_id)) \
    AMS_RO_I_DEBUG_MONITOR_INTERFACE_OUNCE_INTERFACE_INFO(C, H)

AMS_SF_DEFINE_INTERFACE(ams::ro::impl, IDebugMonitorInterface, AMS_RO_I_DEBUG_MONITOR_INTERFACE_INTERFACE_INFO, 0xBBA11B0A)
