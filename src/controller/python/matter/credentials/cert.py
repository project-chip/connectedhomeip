#
#    Copyright (c) 2023 Project CHIP Authors
#    All rights reserved.
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#

import ctypes

from ..native import GetLibraryHandle, NativeLibraryHandleMethodArguments, PyChipError


def _handle():
    handle = GetLibraryHandle()
    if handle.pychip_ConvertX509CertToChipCert.argtypes is None:
        setter = NativeLibraryHandleMethodArguments(handle)
        setter.Set("pychip_ConvertX509CertToChipCert", PyChipError, [ctypes.POINTER(
            ctypes.c_uint8), ctypes.c_size_t, ctypes.POINTER(ctypes.c_uint8), ctypes.POINTER(ctypes.c_size_t)])
        setter.Set("pychip_ConvertChipCertToX509Cert", PyChipError, [ctypes.POINTER(
            ctypes.c_uint8), ctypes.c_size_t, ctypes.POINTER(ctypes.c_uint8), ctypes.POINTER(ctypes.c_size_t)])
    if handle.pychip_GetMaxCertificateChainDocumentSize.argtypes is None:
        NativeLibraryHandleMethodArguments(handle).Set("pychip_GetMaxCertificateChainDocumentSize", ctypes.c_uint32, [])
    if handle.pychip_GetCertificateChainDocumentSizeForProfiles.argtypes is None:
        NativeLibraryHandleMethodArguments(handle).Set(
            "pychip_GetCertificateChainDocumentSizeForProfiles", ctypes.c_uint32, [ctypes.c_uint8, ctypes.c_uint8])
    return handle


def convert_x509_cert_to_chip_cert(x509Cert: bytes) -> bytes:
    """Converts a x509 certificate to CHIP Certificate."""
    output_buffer = (ctypes.c_uint8 * 1024)()
    output_size = ctypes.c_size_t(1024)
    ptr_type = ctypes.POINTER(ctypes.c_uint8)

    _handle().pychip_ConvertX509CertToChipCert(ctypes.cast(x509Cert, ptr_type), len(x509Cert),
                                               ctypes.cast(output_buffer, ptr_type), ctypes.byref(output_size)).raise_on_error()

    return bytes(output_buffer)[:output_size.value]


def convert_chip_cert_to_x509_cert(chipCert: bytes) -> bytes:
    """Converts a x509 certificate to CHIP Certificate."""
    output_buffer = (ctypes.c_byte * 1024)()
    output_size = ctypes.c_size_t(1024)
    ptr_type = ctypes.POINTER(ctypes.c_uint8)

    _handle().pychip_ConvertChipCertToX509Cert(ctypes.cast(chipCert, ptr_type), len(chipCert),
                                               ctypes.cast(output_buffer, ptr_type), ctypes.byref(output_size)).raise_on_error()

    return bytes(output_buffer)[:output_size.value]


def get_max_certificate_chain_document_size(subject_profile: int | None = None, issuer_profile: int | None = None) -> int:
    """Return a native SDK bound for both certificate algorithms, or the global bound if either is unknown."""
    # Preserve the parameterless API and native symbol for existing callers.
    if subject_profile is None or issuer_profile is None:
        return _handle().pychip_GetMaxCertificateChainDocumentSize()
    return _handle().pychip_GetCertificateChainDocumentSizeForProfiles(subject_profile, issuer_profile)
