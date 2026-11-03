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
#include "ro_ns2jb_service_impl.hpp"
#include "ro_random.hpp"

namespace ams::ro::impl {

    #if defined(ATMOSPHERE_BOARD_NINTENDO_OUNCE)
    namespace {

        constexpr u64 MaxPermissionSize  = 64_MB;
        constexpr u64 MaxBulkReadSize    = 4_MB;
        constexpr u32 MaxRegionScanSteps = 256;

        void FlushCaches(uintptr_t addr, size_t size) {
            #if defined(ATMOSPHERE_ARCH_ARM64)
            /* Declare helper variables. */
            uintptr_t cache_type_register = 0;
            uintptr_t cache_line_size     = 0;
            const uintptr_t end_addr      = reinterpret_cast<uintptr_t>(addr) + size;

            /* Get the cache type register. */
            __asm__ __volatile__("mrs %[cache_type_register], ctr_el0" : [cache_type_register]"=r"(cache_type_register));

            /* Calculate cache line size. */
            cache_line_size = 4 << ((cache_type_register >> 16) & 0xF);

            /* Get the thread local region. */
            auto * const tlr = svc::GetThreadLocalRegion();

            /* Note to the kernel that we're performing cache maintenance, in case we get interrupted while touching cache lines. */
            tlr->cache_maintenance_flag = 1;
            ON_SCOPE_EXIT { tlr->cache_maintenance_flag = 0; };

            /* Iterate, flushing data cache lines. */
            for (uintptr_t cur = reinterpret_cast<uintptr_t>(addr) & ~(cache_line_size - 1); cur < end_addr; cur += cache_line_size) {
                __asm__ __volatile__ ("dc cvau, %[cur]" :: [cur]"r"(cur));
            }

            /* Insert a memory barrier, now that memory has been flushed. */
            __asm__ __volatile__("dsb ish" ::: "memory");

            /* Iterate, flushing instruction cache lines. */
            for (uintptr_t cur = reinterpret_cast<uintptr_t>(addr) & ~(cache_line_size - 1); cur < end_addr; cur += cache_line_size) {
                __asm__ __volatile__ ("ic ivau, %[cur]" :: [cur]"r"(cur));
            }

            /* Insert a memory barrier, now that memory has been flushed. */
            __asm__ __volatile__("dsb ish; isb" ::: "memory");
            #endif
        }

        Result QueryMemoryInfo(svc::MemoryInfo *out_info, svc::PageInfo *out_page_info, os::NativeHandle handle, u64 address) {
            if (handle == os::GetCurrentProcessHandle()) {
                R_RETURN(svc::QueryMemory(out_info, out_page_info, address));
            } else {
                R_RETURN(svc::QueryProcessMemory(out_info, out_page_info, handle, address));
            }
        }

        Result FindFreeAddress(u64 *out, os::NativeHandle handle, u64 size) {
            u64 begin = 0, extent = 0, alias = 0, alias_size = 0, heap = 0, heap_size = 0;
            R_TRY(svc::GetInfo(std::addressof(begin),      svc::InfoType_AslrRegionAddress,  handle, 0));
            R_TRY(svc::GetInfo(std::addressof(extent),     svc::InfoType_AslrRegionSize,     handle, 0));
            R_TRY(svc::GetInfo(std::addressof(alias),      svc::InfoType_AliasRegionAddress, handle, 0));
            R_TRY(svc::GetInfo(std::addressof(alias_size), svc::InfoType_AliasRegionSize,    handle, 0));
            R_TRY(svc::GetInfo(std::addressof(heap),       svc::InfoType_HeapRegionAddress,  handle, 0));
            R_TRY(svc::GetInfo(std::addressof(heap_size),  svc::InfoType_HeapRegionSize,     handle, 0));

            const u64 end = begin + extent;
            u64 at = begin != 0 ? begin : os::MemoryPageSize;

            *out = 0;
            for (u32 i = 0; i < MaxRegionScanSteps && at < end; ++i) {
                svc::MemoryInfo info = {};
                svc::PageInfo page_info = {};
                R_TRY(QueryMemoryInfo(std::addressof(info), std::addressof(page_info), handle, at));

                if (info.size == 0 || info.base_address + info.size <= at) {
                    R_SUCCEED();
                }

                const u64 next = info.base_address + info.size;
                if (at < alias + alias_size && at + size > alias) {
                    at = alias + alias_size;
                    continue;
                }

                if (at < heap + heap_size && at + size > heap) {
                    at = heap + heap_size;
                    continue;
                }

                if (info.state == svc::MemoryState_Free && at + size <= next && at + size <= end) {
                    *out = at;
                    R_SUCCEED();
                }

                at = next;
            }

            R_SUCCEED();
        }

        Result ValidateProcessAndQuery(ProcessIdentityHeader *out_identity, u64 *out_query_result, svc::MemoryInfo *out_info, os::NativeHandle handle, os::ProcessId process_id, u64 address, bool require_process_id) {
            /* Query process identity and program id. */
            u64 pid = 0;
            u64 program_id = 0;
            const Result pid_res  = svc::GetProcessId(std::addressof(pid), handle);
            const Result prog_res = svc::GetInfo(std::addressof(program_id), svc::InfoType_ProgramId, handle, 0);
            *out_identity = ProcessIdentityHeader{
                .pid               = pid,
                .program_id        = program_id,
                .pid_result        = pid_res.GetValue(),
                .program_id_result = prog_res.GetValue(),
            };
            R_TRY(pid_res);
            R_TRY(prog_res);
            R_UNLESS(process_id.value == 0 || process_id.value == pid, ns2jb::ResultInvalidMemoryState());

            /* Query the target memory region. */
            svc::PageInfo page_info = {};
            const Result query_res = svc::QueryProcessMemory(out_info, std::addressof(page_info), handle, address);
            if (out_query_result != nullptr) {
                *out_query_result = query_res.GetValue();
            }

            if (require_process_id) {
                R_TRY(query_res);
                R_UNLESS(process_id.value != 0, ns2jb::ResultInvalidMemoryState());
            }

            R_RETURN(query_res);
        }

    }

    Result ProbeExecution(ProbeReply *out, u64 nonce) {
        /* TODO: remove, very unnecessary command */
        constexpr u64 ProbeMagic         = 0x4558454F5232534EULL;
        constexpr u64 ProbeResponseKey   = 0x9D7C3A51E6B2048FULL;

        u64 pid = 0;
        u64 tid = 0;
        R_TRY(svc::GetProcessId(std::addressof(pid), os::GetCurrentProcessHandle()));
        R_TRY(svc::GetThreadId(std::addressof(tid), svc::PseudoHandle::CurrentThread));

        *out = ProbeReply{
            .magic        = ProbeMagic,
            .response     = nonce ^ ProbeResponseKey,
            .pid          = pid,
            .tid          = tid,
            .tick         = static_cast<u64>(os::GetSystemTick().GetInt64Value()),
            .registration = 0,
        };

        R_SUCCEED();
    }

    Result QueryProcessInfo(ProcessInfoReply *out, os::NativeHandle process_h, os::ProcessId process_id, u64 address) {
        std::memset(out, 0, sizeof(*out));

        /* Validate process and query region. */
        svc::MemoryInfo info = {};
        R_TRY(ValidateProcessAndQuery(std::addressof(out->identity), std::addressof(out->query_result), std::addressof(info), process_h, process_id, address, false));

        /* Populate address space and free page info. */
        out->memory_info = info;
        R_DISCARD(svc::GetInfo(std::addressof(out->aslr_region_address), svc::InfoType_AslrRegionAddress, process_h, 0));
        R_DISCARD(svc::GetInfo(std::addressof(out->aslr_region_size),    svc::InfoType_AslrRegionSize,    process_h, 0));

        R_TRY(FindFreeAddress(std::addressof(out->target_free_address), process_h, os::MemoryPageSize));
        R_TRY(FindFreeAddress(std::addressof(out->local_free_address),  os::GetCurrentProcessHandle(), os::MemoryPageSize));

        R_SUCCEED();
    }

    Result ReadProcessMemory(ProcessReadReply *out, os::NativeHandle process_h, os::ProcessId process_id, u64 source, u64 length) {
        std::memset(out, 0, sizeof(*out));

        /* Validate process and target region. */
        svc::MemoryInfo info = {};
        R_TRY(ValidateProcessAndQuery(std::addressof(out->identity), nullptr, std::addressof(info), process_h, process_id, source, true));

        const u64 page        = util::AlignDown(source, os::MemoryPageSize);
        const u64 page_offset = source & (os::MemoryPageSize - 1);
        R_UNLESS(length != 0,                                ns2jb::ResultInvalidArgument());
        R_UNLESS(length <= sizeof(out->data),                ns2jb::ResultInvalidArgument());
        R_UNLESS(page_offset + length <= os::MemoryPageSize, ns2jb::ResultInvalidArgument());

        /* Map the target page into our address space. */
        void *mapped_memory = nullptr;
        R_TRY(os::MapProcessMemory(std::addressof(mapped_memory), process_h, page, os::MemoryPageSize, GenerateSecureRandom));
        ON_SCOPE_EXIT { os::UnmapProcessMemory(mapped_memory, process_h, page, os::MemoryPageSize); };

        /* Copy the requested bytes. */
        std::memcpy(out->data, static_cast<const u8 *>(mapped_memory) + page_offset, length);

        out->local_address = reinterpret_cast<u64>(mapped_memory);
        out->bytes_read    = length;

        R_SUCCEED();
    }

    Result MapProcessCodeMemory(ProcessCodeMapReply *out, os::NativeHandle process_h, os::ProcessId process_id, u64 source, u64 destination, u64 size) {
        std::memset(out, 0, sizeof(*out));

        /* Validate process and source heap page. */
        svc::MemoryInfo info = {};
        R_TRY(ValidateProcessAndQuery(std::addressof(out->identity), nullptr, std::addressof(info), process_h, process_id, source, true));

        R_UNLESS(util::IsAligned(source | destination, os::MemoryPageSize), ns2jb::ResultInvalidAlignment());
        R_UNLESS(size == os::MemoryPageSize,                                ns2jb::ResultInvalidArgument());
        R_UNLESS(info.state == svc::MemoryState_Normal,                     ns2jb::ResultInvalidArgument());
        R_UNLESS(info.permission == svc::MemoryPermission_ReadWrite,        ns2jb::ResultInvalidArgument());
        R_UNLESS(info.attribute == 0,                                       ns2jb::ResultInvalidArgument());
        R_UNLESS(source + size <= info.base_address + info.size,            ns2jb::ResultInvalidArgument());

        /* Ensure destination range is unmapped. */
        svc::MemoryInfo target = {};
        svc::PageInfo page_info = {};
        R_TRY(svc::QueryProcessMemory(std::addressof(target), std::addressof(page_info), process_h, destination));
        R_UNLESS(target.state == svc::MemoryState_Free,                   ns2jb::ResultInvalidMemoryState());
        R_UNLESS(destination + size <= target.base_address + target.size, ns2jb::ResultInvalidMemoryState());

        /* Map code memory and mark it executable, rolling back on failure. */
        R_TRY(svc::MapProcessCodeMemory(process_h, destination, source, size));
        ON_RESULT_FAILURE { R_DISCARD(svc::UnmapProcessCodeMemory(process_h, destination, source, size)); };
        R_TRY(svc::SetProcessMemoryPermission(process_h, destination, size, svc::MemoryPermission_ReadExecute));

        R_SUCCEED();
    }

    Result UnmapProcessCodeMemory(ProcessCodeMapReply *out, os::NativeHandle process_h, os::ProcessId process_id, u64 source, u64 destination, u64 size) {
        std::memset(out, 0, sizeof(*out));

        /* Validate process and alignment. */
        svc::MemoryInfo info = {};
        R_TRY(ValidateProcessAndQuery(std::addressof(out->identity), nullptr, std::addressof(info), process_h, process_id, source, true));

        R_UNLESS(util::IsAligned(source | destination, os::MemoryPageSize), ns2jb::ResultInvalidAlignment());
        R_UNLESS(size == os::MemoryPageSize,                                ns2jb::ResultInvalidArgument());

        R_RETURN(svc::UnmapProcessCodeMemory(process_h, destination, source, size));
    }

    Result PatchProcessCodeMemory(ProcessPatchReply *out, os::NativeHandle process_h, os::ProcessId process_id, u64 address, u64 size, const ProcessPatchInput &patch) {
        std::memset(out, 0, sizeof(*out));

        /* TODO: cleanup codex's overly defensive garbage? */

        /* Validate process and executable code region. */
        svc::MemoryInfo info = {};
        R_TRY(ValidateProcessAndQuery(std::addressof(out->identity), std::addressof(out->query_result), std::addressof(info), process_h, process_id, address, true));

        const uintptr_t page        = util::AlignDown(address, os::MemoryPageSize);
        const uintptr_t page_offset = address & (os::MemoryPageSize - 1);

        R_UNLESS(size == sizeof(patch.expected),                         ns2jb::ResultInvalidMemoryState());
        R_UNLESS(util::IsAligned(address, sizeof(u32)),                  ns2jb::ResultInvalidMemoryState());
        R_UNLESS(page_offset + size <= os::MemoryPageSize,               ns2jb::ResultInvalidMemoryState());
        R_UNLESS((info.permission & svc::MemoryPermission_Execute) != 0, ns2jb::ResultInvalidMemoryState());
        R_UNLESS(info.attribute == 0,                                    ns2jb::ResultInvalidMemoryState());
        R_UNLESS(address + size <= info.base_address + info.size,        ns2jb::ResultInvalidMemoryState());

        /* Map the target code page into our address space. */
        void *mapped_memory = nullptr;
        R_TRY(os::MapProcessMemory(std::addressof(mapped_memory), process_h, page, os::MemoryPageSize, GenerateSecureRandom));
        ON_SCOPE_EXIT { os::UnmapProcessMemory(mapped_memory, process_h, page, os::MemoryPageSize); };

        /* Validate that we received a writable alias. */
        const uintptr_t local = reinterpret_cast<uintptr_t>(mapped_memory);
        svc::MemoryInfo alias = {};
        svc::PageInfo page_info = {};
        R_TRY(svc::QueryMemory(std::addressof(alias), std::addressof(page_info), local));
        R_UNLESS(alias.permission == svc::MemoryPermission_ReadWrite, ns2jb::ResultInvalidMemoryState());
        R_UNLESS(alias.attribute == 0,                                ns2jb::ResultInvalidMemoryState());

        /* Verify the expected bytes. */
        u8 *mapped = static_cast<u8 *>(mapped_memory) + page_offset;
        R_UNLESS(std::memcmp(mapped, patch.expected, size) == 0, ns2jb::ResultPatchExpectedMismatch());

        /* Write the replacement and publish the new instructions. */
        std::memcpy(mapped, patch.replacement, size);
        FlushCaches(reinterpret_cast<uintptr_t>(mapped), size);

        /* TODO: codex's overly defensive garbage? */
        R_UNLESS(std::memcmp(mapped, patch.replacement, size) == 0, ns2jb::ResultPatchReadbackMismatch());

        out->local_address    = local;
        out->alias_permission = static_cast<u32>(alias.permission);
        out->alias_attribute  = static_cast<u32>(alias.attribute);
        out->expected_matched = 1;
        out->readback_matched = 1;

        R_SUCCEED();
    }

    Result SetProcessMemoryPermission(ProcessPermissionReply *out, os::NativeHandle process_h, os::ProcessId process_id, u64 address, u64 size, u64 permission) {
        std::memset(out, 0, sizeof(*out));

        /* Validate FS process and code region. */
        svc::MemoryInfo info = {};
        R_TRY(ValidateProcessAndQuery(std::addressof(out->identity), nullptr, std::addressof(info), process_h, process_id, address, true));

        /* TODO: codex's overly defensive garbage? */
        #if 0
        R_UNLESS(out->identity.program_id == ncm::SystemProgramId::Fs.value,                                                ns2jb::ResultInvalidMemoryState());

        R_UNLESS(util::IsAligned(address | size, os::MemoryPageSize),                                                       ns2jb::ResultInvalidMemoryState());
        R_UNLESS(size != 0 && size <= MaxPermissionSize,                                                                    ns2jb::ResultInvalidMemoryState());
        R_UNLESS(info.state == svc::MemoryState_Code,                                                                       ns2jb::ResultInvalidMemoryState());
        R_UNLESS(info.attribute == 0,                                                                                       ns2jb::ResultInvalidMemoryState());
        R_UNLESS(info.permission == svc::MemoryPermission_Execute || info.permission == svc::MemoryPermission_ReadExecute,  ns2jb::ResultInvalidMemoryState());
        R_UNLESS(permission == svc::MemoryPermission_Execute || permission == svc::MemoryPermission_ReadExecute,            ns2jb::ResultInvalidMemoryState());
        R_UNLESS(address + size >= address && address + size <= info.base_address + info.size,                              ns2jb::ResultInvalidMemoryState());
        #endif

        /* Update permission and re-query the region. */
        R_TRY(os::SetProcessMemoryPermission(process_h, address, size, static_cast<os::MemoryPermission>(permission)));

        svc::MemoryInfo after = {};
        svc::PageInfo page_info = {};
        R_TRY(svc::QueryProcessMemory(std::addressof(after), std::addressof(page_info), process_h, address));

        out->previous_permission = static_cast<u32>(info.permission);
        out->after_memory_info   = after;

        R_SUCCEED();
    }

    Result BulkReadProcessMemory(ProcessBulkReadReply *out, void *receive_buf, size_t receive_size, os::NativeHandle process_h, os::ProcessId process_id, u64 source, u64 length) {
        std::memset(out, 0, sizeof(*out));

        /* Validate process and code/data region. */
        svc::MemoryInfo info = {};
        R_TRY(ValidateProcessAndQuery(std::addressof(out->identity), nullptr, std::addressof(info), process_h, process_id, source, true));

        const u64 page        = util::AlignDown(source, os::MemoryPageSize);
        const u64 page_offset = source & (os::MemoryPageSize - 1);

        R_UNLESS(receive_buf != nullptr,                                                           ns2jb::ResultInvalidMemoryState());
        R_UNLESS(length != 0 && length <= MaxBulkReadSize && length <= receive_size,               ns2jb::ResultInvalidMemoryState());
        R_UNLESS(source + length >= source && source + length <= info.base_address + info.size,    ns2jb::ResultInvalidMemoryState());
        R_UNLESS(info.state == svc::MemoryState_Code || info.state == svc::MemoryState_CodeData,   ns2jb::ResultInvalidMemoryState());
        R_UNLESS(info.attribute == 0,                                                              ns2jb::ResultInvalidMemoryState());

        /* Map the requested page range into our address space. */
        const u64 map_size = util::AlignUp(page_offset + length, os::MemoryPageSize);
        void *mapped_memory = nullptr;
        R_TRY(os::MapProcessMemory(std::addressof(mapped_memory), process_h, page, map_size, GenerateSecureRandom));
        ON_SCOPE_EXIT { os::UnmapProcessMemory(mapped_memory, process_h, page, map_size); };

        /* Copy the requested bytes to the caller's receive buffer. */
        std::memcpy(receive_buf, static_cast<const u8 *>(mapped_memory) + page_offset, length);

        out->local_address = reinterpret_cast<u64>(mapped_memory);
        out->bytes_read    = length;

        R_SUCCEED();
    }
    #endif

}
