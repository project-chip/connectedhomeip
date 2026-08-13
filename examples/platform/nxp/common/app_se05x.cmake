#
# Copyright (c) 2026 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

# ****************************************************************
# SE05X Application Files
# ****************************************************************

if(CONFIG_CHIP_SE05X)
if(CONFIG_APP_FREERTOS_OS)
    list(FIND EXTRA_MCUX_MODULES "${CHIP_ROOT}/third_party/simw-top-mini/repo/matter" se_index)
    if(se_index EQUAL -1)
        message(FATAL_ERROR "MCUX_MODULES must include ${CHIP_ROOT}/third_party/simw-top-mini/repo/matter in the application when CONFIG_CHIP_SE05X is enabled")
    endif()

    target_include_directories(app PRIVATE
        ${EXAMPLE_PLATFORM_NXP_COMMON_DIR}/app_se05x/include
    )

    target_sources(app PRIVATE
        ${EXAMPLE_PLATFORM_NXP_COMMON_DIR}/app_se05x/source/AppSe05x.cpp
    )
else()
    list(FIND ZEPHYR_EXTRA_MODULES "${CHIP_ROOT}/third_party/simw-top-mini/repo/matter" se_index)
    if(se_index EQUAL -1)
        message(FATAL_ERROR "ZEPHYR MODULES must include ${CHIP_ROOT}/third_party/simw-top-mini/repo/matter in the application when CONFIG_CHIP_SE05X is enabled")
    endif()

    target_include_directories(app PRIVATE
        ${EXAMPLE_PLATFORM_NXP_COMMON_DIR}/app_se05x/include/
        ${CHIP_ROOT}/src/platform/nxp/crypto/se05x/
    )

if (CONFIG_CHIP_APP_OPERATIONAL_KEYSTORE)
    target_include_directories(app PRIVATE
        ${EXAMPLE_PLATFORM_NXP_COMMON_DIR}/operational_keystore/include
    )
endif()

# GN platform abstraction layer is platform-agnostic for Zephyr, se05x based nxp crypto lib file added here.
    target_sources(app PRIVATE
        ${EXAMPLE_PLATFORM_NXP_COMMON_DIR}/app_se05x/source/AppSe05x.cpp
        ${CHIP_ROOT}/src/platform/nxp/crypto/se05x/CHIPCryptoPALHsm_se05x_utils.cpp
        ${CHIP_ROOT}/src/platform/nxp/crypto/se05x/PersistentStorageOperationalKeystore_se05x.cpp
        ${CHIP_ROOT}/src/platform/nxp/crypto/se05x/CHIPCryptoPALHsm_se05x_hkdf.cpp
        ${CHIP_ROOT}/src/platform/nxp/crypto/se05x/CHIPCryptoPALHsm_se05x_hmac.cpp
        ${CHIP_ROOT}/src/platform/nxp/crypto/se05x/CHIPCryptoPALHsm_se05x_p256.cpp
        ${CHIP_ROOT}/src/platform/nxp/crypto/se05x/CHIPCryptoPALHsm_se05x_pbkdf.cpp
        ${CHIP_ROOT}/src/platform/nxp/crypto/se05x/CHIPCryptoPALHsm_se05x_rng.cpp
        ${CHIP_ROOT}/src/platform/nxp/crypto/se05x/CHIPCryptoPALHsm_se05x_spake2p.cpp
        ${CHIP_ROOT}/src/platform/nxp/crypto/se05x/CHIPCryptoPAL_HostFallBack.cpp
    )

if (CONFIG_CHIP_APP_OPERATIONAL_KEYSTORE)
    target_sources(app PRIVATE
        ${EXAMPLE_PLATFORM_NXP_COMMON_DIR}/operational_keystore/source/OperationalKeystoreSE05X.cpp
    )
endif()

if (CONFIG_CHIP_APP_FACTORY_DATA)
    if (CONFIG_CHIP_APP_FACTORY_DATA_IMPL_PLATFORM)
        target_sources(app PRIVATE
            ${EXAMPLE_NXP_PLATFORM_DIR}/factory_data/source/AppFactoryDataExample.cpp
        )
        if (CONFIG_CHIP_ENABLE_SECURE_WHOLE_FACTORY_DATA)
            target_compile_definitions(app PRIVATE
                ENABLE_SECURE_WHOLE_FACTORY_DATA
            )
        endif()
    elseif (CONFIG_CHIP_APP_FACTORY_DATA_IMPL_COMMON)
        target_sources(app PRIVATE
            ${EXAMPLE_PLATFORM_NXP_COMMON_DIR}/factory_data/source/AppFactoryDataDefaultImpl.cpp
        )
    endif()

    if (CONFIG_CHIP_SE05X_SPAKE_VERIFIER_USE_TP_VALUES OR CONFIG_CHIP_SE05X_DEVICE_ATTESTATION)
        target_sources(app PRIVATE
            ${EXAMPLE_PLATFORM_NXP_COMMON_DIR}/../se05x/mcu/common/factory_data_impl/Se05xDataProvider.cpp
        )
    endif()
endif()


endif()
endif()
