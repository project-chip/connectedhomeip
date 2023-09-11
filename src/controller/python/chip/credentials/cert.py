#
# SPDX-FileCopyrightText: 2023 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
#

import ctypes

import chip.native


def _handle():
    handle = chip.native.GetLibraryHandle()
    if handle.pychip_ConvertX509CertToChipCert.argtypes is None:
        setter = chip.native.NativeLibraryHandleMethodArguments(handle)
        setter.Set("pychip_ConvertX509CertToChipCert", chip.native.PyChipError, [ctypes.POINTER(
            ctypes.c_uint8), ctypes.c_size_t, ctypes.POINTER(ctypes.c_uint8), ctypes.POINTER(ctypes.c_size_t)])
        setter.Set("pychip_ConvertChipCertToX509Cert", chip.native.PyChipError, [ctypes.POINTER(
            ctypes.c_uint8), ctypes.c_size_t, ctypes.POINTER(ctypes.c_uint8), ctypes.POINTER(ctypes.c_size_t)])
    return handle


def convert_x509_cert_to_chip_cert(x509Cert: bytes) -> bytes:
    """Converts a x509 certificate to CHIP Certificate."""
    output_buffer = (ctypes.c_uint8 * 1024)()
    output_size = ctypes.c_size_t(1024)

    _handle().pychip_ConvertX509CertToChipCert(x509Cert, len(x509Cert), output_buffer, ctypes.byref(output_size)).raise_on_error()

    return bytes(output_buffer)[:output_size.value]


def convert_chip_cert_to_x509_cert(chipCert: bytes) -> bytes:
    """Converts a x509 certificate to CHIP Certificate."""
    output_buffer = (ctypes.c_byte * 1024)()
    output_size = ctypes.c_size_t(1024)

    _handle().pychip_ConvertChipCertToX509Cert(chipCert, len(chipCert), output_buffer, ctypes.byref(output_size)).raise_on_error()

    return bytes(output_buffer)[:output_size.value]
