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

    /* Access utilities. */
    void SetDevelopmentHardware(bool is_development_hardware);
    void SetDevelopmentFunctionEnabled(bool is_development_function_enabled);

    class RoService {
        private:
            size_t m_context_id;
            NrrKind m_nrr_kind;
        protected:
            explicit RoService(NrrKind k);
        public:
            virtual ~RoService();
        public:
            /* Actual commands. */
            Result MapManualLoadModuleMemory(sf::Out<u64> out_load_address, const sf::ClientProcessId &client_pid, u64 nro_address, u64 nro_size, u64 bss_address, u64 bss_size);
            Result UnmapManualLoadModuleMemory(const sf::ClientProcessId &client_pid, u64 nro_address);
            Result RegisterModuleInfo(const sf::ClientProcessId &client_pid, u64 nrr_address, u64 nrr_size);
            Result UnregisterModuleInfo(const sf::ClientProcessId &client_pid, u64 nrr_address);
            Result RegisterProcessHandle(const sf::ClientProcessId &client_pid, sf::CopyHandle &&process_h);
            Result RegisterModuleInfoWithUserProcessHandle(const sf::ClientProcessId &client_pid, u64 nrr_address, u64 nrr_size, sf::CopyHandle &&process_h);

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
    static_assert(ro::impl::IsIRoInterface<RoService>);

}
