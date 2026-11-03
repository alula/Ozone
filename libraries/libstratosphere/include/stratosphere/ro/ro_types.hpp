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
#include <stratosphere/ncm/ncm_ids.hpp>
#include <stratosphere/rocrt/rocrt.hpp>

namespace ams::ro {

    enum NrrKind : u8 {
        NrrKind_User      = 0,
        NrrKind_JitPlugin = 1,

        NrrKind_Count,
    };

    static constexpr size_t ModuleIdSize = 0x20;
    struct ModuleId {
        u8 data[ModuleIdSize];
    };
    static_assert(sizeof(ModuleId) == ModuleIdSize);

    struct NrrCertification {
        static constexpr size_t RsaKeySize = 0x100;
        static constexpr size_t SignedSize = 0x120;

        u64 program_id_mask;
        u64 program_id_pattern;
        u8  reserved_10[0x10];
        u8  public_key[RsaKeySize];
        u8  sign[RsaKeySize];
    };
    static_assert(sizeof(NrrCertification) == NrrCertification::RsaKeySize + NrrCertification::SignedSize);

    class NrrHeader {
        public:
            static constexpr u32 Signature = util::FourCC<'N','R','R','0'>::Code;
        private:
            u32 m_signature;
            u32 m_key_generation;
            u8  m_reserved_08[0x08];
            NrrCertification m_certification;
            u8  m_sign[0x100];
            ncm::ProgramId m_program_id;
            u32 m_size;
            u8  m_nrr_kind; /* 7.0.0+ */
            u8  m_reserved_33D[3];
            u32 m_hash_list_offset_address;
            u32 m_num_hash;
            u8  m_reserved_348[8];
        public:
            bool IsMagicValid() const {
                return m_signature == Signature;
            }

            bool IsProgramIdValid() const {
                return (m_program_id.value & m_certification.program_id_mask) == m_certification.program_id_pattern;
            }

            NrrKind GetNrrKind() const {
                const NrrKind kind = static_cast<NrrKind>(m_nrr_kind);
                AMS_ABORT_UNLESS(kind < NrrKind_Count);
                return kind;
            }

            ncm::ProgramId GetProgramId() const {
                return m_program_id;
            }

            u32 GetSize() const {
                return m_size;
            }

            u32 GetNumHashes() const {
                return m_num_hash;
            }

            size_t GetHashesOffset() const {
                return m_hash_list_offset_address;
            }

            uintptr_t GetHashes() const {
                return reinterpret_cast<uintptr_t>(this) + this->GetHashesOffset();
            }

            u32 GetKeyGeneration() const {
                return m_key_generation;
            }

            const u8 *GetCertificationSignature() const {
                return m_certification.sign;
            }

            const u8 *GetCertificationSignedArea() const {
                return reinterpret_cast<const u8 *>(std::addressof(m_certification));
            }

            const u8 *GetCertificationModulus() const {
                return m_certification.public_key;
            }

            const u8 *GetSignature() const {
                return m_sign;
            }

            const u8 *GetSignedArea() const {
                return reinterpret_cast<const u8 *>(std::addressof(m_program_id));
            }

            size_t GetSignedAreaSize() const {
                return m_size - GetSignedAreaOffset();
            }

            static constexpr size_t GetSignedAreaOffset();
    };
    static_assert(sizeof(NrrHeader) == 0x350, "NrrHeader definition!");

    constexpr size_t NrrHeader::GetSignedAreaOffset() {
        return AMS_OFFSETOF(NrrHeader, m_program_id);
    }
    
    static constexpr size_t RocrtHeaderSize = 0x10;
    struct RocrtHeader {
        rocrt::ModuleHeaderLocation module_header_location;
        u32 reserved;
    };
    static_assert(sizeof(RocrtHeader) == RocrtHeaderSize);

    class NroHeader {
        public:
            static constexpr u32 Signature = util::FourCC<'N','R','O','0'>::Code;
            static constexpr u32 FlagAlignedHeader = 1;
            static constexpr u32 FlagCompress = 2;
        private:
            RocrtHeader m_rocrt;
            u32 m_signature;
            u32 m_version;
            u32 m_size;
            u32 m_flags;
            u32 m_text_memory_offset;
            u32 m_text_size;
            u32 m_ro_memory_offset;
            u32 m_ro_size;
            u32 m_data_memory_offset;
            u32 m_data_size;
            u32 m_bss_size;
            u32 m_additional_header_offset; /* 23.0.0+ */
            ModuleId m_module_id;
            u32 m_dso_handle_offset;
            u32 m_reserved_64;
            u32 m_embedded_offset;
            u32 m_embedded_size;
            u32 m_dyn_str_offset;
            u32 m_dyn_str_size;
            u32 m_dyn_sym_offset;
            u32 m_dyn_sym_size;
        public:
            bool IsMagicValid() const {
                return m_signature == Signature;
            }

            u32 GetVersion() const {
                return m_version;
            }

            u32 GetSize() const {
                return m_size;
            }

            u32 GetFlags() const {
                return m_flags;
            }

            bool IsAlignedHeader() const {
                return m_flags & FlagAlignedHeader;
            }
            
            bool IsCompress() const {
                return m_flags & FlagCompress;
            }

            u32 GetTextOffset() const {
                return m_text_memory_offset;
            }

            u32 GetTextSize() const {
                return m_text_size;
            }

            u32 GetRoOffset() const {
                return m_ro_memory_offset;
            }

            u32 GetRoSize() const {
                return m_ro_size;
            }

            u32 GetRwOffset() const {
                return m_data_memory_offset;
            }

            u32 GetRwSize() const {
                return m_data_size;
            }

            u32 GetBssSize() const {
                return m_bss_size;
            }
            
            u32 GetAdditionalHeaderOffset() const {
                return m_additional_header_offset;
            }

            const ModuleId *GetModuleId() const {
                return std::addressof(m_module_id);
            }
    };
    static_assert(sizeof(NroHeader) == 0x80, "NroHeader definition!");
    
    class AdditionalNroHeader {
        public:
            static constexpr u32 Signature = util::FourCC<'N','R','A','0'>::Code;
        private:
            u32 m_signature;
            u8  m_reserved_04[0xC];
            u32 m_compressed_size;
            u32 m_reserved_14;
            u32 m_hash_size;
            u32 m_hash_offset;
            u8  m_reserved_20[0x40];
            u8  m_hash[0x20];
        public:
            bool IsMagicValid() const {
                return m_signature == Signature;
            }

            u32 GetCompressedSize() const {
                return m_compressed_size;
            }

            u32 GetHashSize() const {
                return m_hash_size;
            }
            
            u32 GetHashOffset() const {
                return m_hash_offset;
            }

            const u8 *GetHash() const {
                return m_hash;
            }
    };
    static_assert(sizeof(AdditionalNroHeader) == 0x80, "AdditionalNroHeader definition!");

    #if defined(ATMOSPHERE_BOARD_NINTENDO_OUNCE)
    namespace impl {

        struct ProbeReply {
            u64 magic;
            u64 response;
            u64 pid;
            u64 tid;
            u64 tick;
            u64 registration;
        };
        static_assert(sizeof(ProbeReply) == 0x30);

        struct ProcessIdentityHeader {
            u64 pid;
            u64 program_id;
            u32 pid_result;
            u32 program_id_result;
        };
        static_assert(sizeof(ProcessIdentityHeader) == 0x18);

        struct ProcessInfoReply {
            ProcessIdentityHeader identity;
            u64 query_result;
            svc::MemoryInfo memory_info;
            u64 aslr_region_address;
            u64 aslr_region_size;
            u64 target_free_address;
            u64 local_free_address;
        };
        static_assert(sizeof(ProcessInfoReply) == 0x68);

        struct ProcessReadReply {
            ProcessIdentityHeader identity;
            u64 local_address;
            u64 map_result;
            u64 unmap_result;
            u64 bytes_read;
            u8 data[0x80];
        };
        static_assert(sizeof(ProcessReadReply) == 0xB8);

        struct ProcessCodeMapReply {
            ProcessIdentityHeader identity;
            u64 map_result;
            u64 permission_result;
            u64 rollback_result;
            u64 reserved[2];
        };
        static_assert(sizeof(ProcessCodeMapReply) == 0x40);

        struct ProcessPatchInput {
            u8 expected[0x10];
            u8 replacement[0x10];
        };
        static_assert(sizeof(ProcessPatchInput) == 0x20);

        struct ProcessPatchReply {
            ProcessIdentityHeader identity;
            u64 query_result;
            u64 map_result;
            u32 alias_permission;
            u32 alias_attribute;
            u64 expected_matched;
            u64 readback_matched;
            u64 unmap_result;
            u64 local_address;
        };
        static_assert(sizeof(ProcessPatchReply) == 0x50);

        struct ProcessPermissionReply {
            ProcessIdentityHeader identity;
            u64 previous_permission;
            u64 permission_result;
            u64 query_after_result;
            svc::MemoryInfo after_memory_info;
        };
        static_assert(sizeof(ProcessPermissionReply) == 0x58);

        struct ProcessBulkReadReply {
            ProcessIdentityHeader identity;
            u64 local_address;
            u64 map_result;
            u64 unmap_result;
            u64 bytes_read;
            u64 reserved;
        };
        static_assert(sizeof(ProcessBulkReadReply) == 0x40);

    }
    #endif

}
