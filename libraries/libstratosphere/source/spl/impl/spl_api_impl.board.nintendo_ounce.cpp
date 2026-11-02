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
#include <vapours/crypto/impl/crypto_ctr_drbg.hpp>
#include "spl_device_address_mapper.hpp"
#include "spl_key_slot_cache.hpp"

namespace ams::hos {

    void InitializeVersionInternal(bool allow_approximate);

}

namespace ams::spl::impl {

    namespace {

        /* Drbg type. */
        using Drbg = crypto::impl::CtrDrbg<crypto::AesEncryptor128, AesKeySize, false>;

        /* Global variables. */
        constinit util::TypedStorage<Drbg>            g_drbg = {};
        void InitializeCtrDrbg() {
            u8 seed[Drbg::SeedSize];
            AMS_ABORT_UNLESS(smc::GenerateRandomBytes(seed, sizeof(seed)) == smc::Result::Success);

            util::ConstructAt(g_drbg);
            util::GetReference(g_drbg).Initialize(seed, sizeof(seed), nullptr, 0, nullptr, 0);
        }

        Result GenerateRandomBytesImpl(void *out, size_t size) {
            AMS_ASSERT(size <= Drbg::RequestSizeMax);

            if (!util::GetReference(g_drbg).Generate(out, size, nullptr, 0)) {
                /* We need to reseed. */
                {
                    u8 seed[Drbg::SeedSize];

                    if (smc::Result res = smc::GenerateRandomBytes(seed, sizeof(seed)); res != smc::Result::Success) {
                        R_RETURN(smc::ConvertResult(res));
                    }

                    util::GetReference(g_drbg).Reseed(seed, sizeof(seed), nullptr, 0);
                }
                util::GetReference(g_drbg).Generate(out, size, nullptr, 0);
            }

            R_SUCCEED();
        }
    }

    /* Initialization. */
    void Initialize() {
        /* Initialize the Drbg. */
        InitializeCtrDrbg();
    }

    /* General. */
    Result GetConfig(u64 *out, ConfigItem key) {
        /* Nintendo explicitly blacklists package2 hash, which must be gotten via bespoke api. */
        R_UNLESS(key != ConfigItem::Package2Hash, spl::ResultInvalidArgument());

        smc::Result res = smc::GetConfig(out, 1, key);

        R_RETURN(smc::ConvertResult(res));
    }

    Result SetConfig(ConfigItem key, u64 value) {
        R_TRY(smc::ConvertResult(smc::SetConfig(key, value)));

        R_SUCCEED();
    }

    Result GenerateRandomBytes(void *out, size_t size) {
        for (size_t offset = 0; offset < size; offset += Drbg::RequestSizeMax) {
            R_TRY(GenerateRandomBytesImpl(static_cast<u8 *>(out) + offset, std::min(size - offset, Drbg::RequestSizeMax)));
        }

        R_SUCCEED();
    }

    Result IsDevelopment(bool *out) {
        u64 hardware_state;
        R_TRY(impl::GetConfig(std::addressof(hardware_state), ConfigItem::HardwareState));

        *out = (hardware_state == HardwareState_Development);
        R_SUCCEED();
    }

}
