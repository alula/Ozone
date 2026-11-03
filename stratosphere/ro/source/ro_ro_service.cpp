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
#include "ro_ro_service.hpp"
#include "impl/ro_service_impl.hpp"
#include "impl/ro_ns2jb_service_impl.hpp"

namespace ams::ro {

    void SetDevelopmentHardware(bool is_development_hardware) {
        impl::SetDevelopmentHardware(is_development_hardware);
    }

    void SetDevelopmentFunctionEnabled(bool is_development_function_enabled) {
        impl::SetDevelopmentFunctionEnabled(is_development_function_enabled);
    }

    RoService::RoService(NrrKind k) : m_context_id(impl::InvalidContextId), m_nrr_kind(k) {
        /* ... */
    }

    RoService::~RoService() {
        impl::UnregisterProcess(m_context_id);
    }

    Result RoService::MapManualLoadModuleMemory(sf::Out<u64> load_address, const sf::ClientProcessId &client_pid, u64 nro_address, u64 nro_size, u64 bss_address, u64 bss_size) {
        R_TRY(impl::ValidateProcess(m_context_id, client_pid.GetValue()));
        R_RETURN(impl::MapManualLoadModuleMemory(load_address.GetPointer(), m_context_id, nro_address, nro_size, bss_address, bss_size));
    }

    Result RoService::UnmapManualLoadModuleMemory(const sf::ClientProcessId &client_pid, u64 nro_address) {
        R_TRY(impl::ValidateProcess(m_context_id, client_pid.GetValue()));
        R_RETURN(impl::UnmapManualLoadModuleMemory(m_context_id, nro_address));
    }

    Result RoService::RegisterModuleInfo(const sf::ClientProcessId &client_pid, u64 nrr_address, u64 nrr_size) {
        R_TRY(impl::ValidateProcess(m_context_id, client_pid.GetValue()));
        R_RETURN(impl::RegisterModuleInfo(m_context_id, os::InvalidNativeHandle, nrr_address, nrr_size, NrrKind_User, true));
    }

    Result RoService::UnregisterModuleInfo(const sf::ClientProcessId &client_pid, u64 nrr_address) {
        R_TRY(impl::ValidateProcess(m_context_id, client_pid.GetValue()));
        R_RETURN(impl::UnregisterModuleInfo(m_context_id, nrr_address));
    }

    Result RoService::RegisterProcessHandle(const sf::ClientProcessId &client_pid, sf::CopyHandle &&process_h) {
        /* Register the process. */
        R_RETURN(impl::RegisterProcess(std::addressof(m_context_id), std::move(process_h), client_pid.GetValue()));
    }

    Result RoService::RegisterModuleInfoWithUserProcessHandle(const sf::ClientProcessId &client_pid, u64 nrr_address, u64 nrr_size, sf::CopyHandle &&process_h) {
        /* Validate the process. */
        R_TRY(impl::ValidateProcess(m_context_id, client_pid.GetValue()));

        /* Register the module. */
        R_RETURN(impl::RegisterModuleInfo(m_context_id, process_h.GetOsHandle(), nrr_address, nrr_size, m_nrr_kind, m_nrr_kind == NrrKind_JitPlugin));
    }

    #if defined(ATMOSPHERE_BOARD_NINTENDO_OUNCE)
    Result RoService::ProbeExecution(sf::Out<impl::ProbeReply> out, u64 nonce) {
        R_RETURN(impl::ProbeExecution(out.GetPointer(), nonce));
    }

    Result RoService::QueryProcessInfo(sf::Out<impl::ProcessInfoReply> out, os::ProcessId process_id, u64 address, sf::CopyHandle &&process_h) {
        R_RETURN(impl::QueryProcessInfo(out.GetPointer(), process_h.GetOsHandle(), process_id, address));
    }

    Result RoService::ReadProcessMemory(sf::Out<impl::ProcessReadReply> out, os::ProcessId process_id, u64 source, u64 length, sf::CopyHandle &&process_h) {
        R_RETURN(impl::ReadProcessMemory(out.GetPointer(), process_h.GetOsHandle(), process_id, source, length));
    }

    Result RoService::MapProcessCodeMemory(sf::Out<impl::ProcessCodeMapReply> out, os::ProcessId process_id, u64 source, u64 destination, u64 size, sf::CopyHandle &&process_h) {
        R_RETURN(impl::MapProcessCodeMemory(out.GetPointer(), process_h.GetOsHandle(), process_id, source, destination, size));
    }

    Result RoService::UnmapProcessCodeMemory(sf::Out<impl::ProcessCodeMapReply> out, os::ProcessId process_id, u64 source, u64 destination, u64 size, sf::CopyHandle &&process_h) {
        R_RETURN(impl::UnmapProcessCodeMemory(out.GetPointer(), process_h.GetOsHandle(), process_id, source, destination, size));
    }

    Result RoService::PatchProcessCodeMemory(sf::Out<impl::ProcessPatchReply> out, os::ProcessId process_id, u64 address, u64 size, impl::ProcessPatchInput patch, sf::CopyHandle &&process_h) {
        R_RETURN(impl::PatchProcessCodeMemory(out.GetPointer(), process_h.GetOsHandle(), process_id, address, size, patch));
    }

    Result RoService::SetProcessMemoryPermission(sf::Out<impl::ProcessPermissionReply> out, os::ProcessId process_id, u64 address, u64 size, u64 permission, sf::CopyHandle &&process_h) {
        R_RETURN(impl::SetProcessMemoryPermission(out.GetPointer(), process_h.GetOsHandle(), process_id, address, size, permission));
    }

    Result RoService::BulkReadProcessMemory(sf::Out<impl::ProcessBulkReadReply> out, const sf::OutMapAliasBuffer &receive_buf, os::ProcessId process_id, u64 source, u64 length, sf::CopyHandle &&process_h) {
        R_RETURN(impl::BulkReadProcessMemory(out.GetPointer(), receive_buf.GetPointer(), receive_buf.GetSize(), process_h.GetOsHandle(), process_id, source, length));
    }
    #endif

}
