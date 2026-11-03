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

namespace ams::ro::impl {

    #if defined(ATMOSPHERE_BOARD_NINTENDO_OUNCE)
    Result ProbeExecution(ProbeReply *out, u64 nonce);
    Result QueryProcessInfo(ProcessInfoReply *out, os::NativeHandle process_h, os::ProcessId process_id, u64 address);
    Result ReadProcessMemory(ProcessReadReply *out, os::NativeHandle process_h, os::ProcessId process_id, u64 source, u64 length);
    Result MapProcessCodeMemory(ProcessCodeMapReply *out, os::NativeHandle process_h, os::ProcessId process_id, u64 source, u64 destination, u64 size);
    Result UnmapProcessCodeMemory(ProcessCodeMapReply *out, os::NativeHandle process_h, os::ProcessId process_id, u64 source, u64 destination, u64 size);
    Result PatchProcessCodeMemory(ProcessPatchReply *out, os::NativeHandle process_h, os::ProcessId process_id, u64 address, u64 size, const ProcessPatchInput &patch);
    Result SetProcessMemoryPermission(ProcessPermissionReply *out, os::NativeHandle process_h, os::ProcessId process_id, u64 address, u64 size, u64 permission);
    Result BulkReadProcessMemory(ProcessBulkReadReply *out, void *receive_buf, size_t receive_size, os::NativeHandle process_h, os::ProcessId process_id, u64 source, u64 length);
    #endif

}
