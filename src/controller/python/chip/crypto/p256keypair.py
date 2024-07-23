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

import abc
import hashlib
from ctypes import (CFUNCTYPE, POINTER, _Pointer, c_bool, c_char, c_size_t, c_uint8, c_uint32, c_void_p, cast, memmove, py_object,
                    string_at)
from typing import TYPE_CHECKING

from chip import native
from ecdsa import ECDH, NIST256p, SigningKey  # type: ignore

# WORKAROUND: Create a subscriptable pointer type (with square brackets) to ensure compliance of type hinting with ctypes
if not TYPE_CHECKING:
    class pointer_fix:
        @classmethod
        def __class_getitem__(cls, item):
            return POINTER(item)
    _Pointer = pointer_fix


_pychip_P256Keypair_ECDSA_sign_msg_func = CFUNCTYPE(
    c_bool, py_object, POINTER(c_uint8), c_size_t, POINTER(c_uint8), POINTER(c_size_t))

_pychip_P256Keypair_ECDH_derive_secret_func = CFUNCTYPE(c_bool, py_object, POINTER(c_uint8), POINTER(c_uint8), POINTER(c_size_t))

P256_PUBLIC_KEY_LENGTH = 2 * 32 + 1


@ _pychip_P256Keypair_ECDSA_sign_msg_func
def _pychip_ECDSA_sign_msg(self_: 'P256Keypair', message_buf: _Pointer[c_uint8], message_size: int, signature_buf: _Pointer[c_uint8], signature_buf_size: _Pointer[c_size_t]) -> bool:
    res = self_.ECDSA_sign_msg(string_at(message_buf, message_size)[:])
    memmove(signature_buf, res, len(res))
    signature_buf_size.contents.value = len(res)
    return True


@ _pychip_P256Keypair_ECDH_derive_secret_func
def _pychip_ECDH_derive_secret(self_: 'P256Keypair', remote_pubkey: _Pointer[c_uint8], out_secret_buf: _Pointer[c_uint8], out_secret_buf_size: _Pointer[c_uint32]) -> bool:
    res = self_.ECDH_derive_secret(string_at(remote_pubkey, P256_PUBLIC_KEY_LENGTH)[:])
    memmove(out_secret_buf, res, len(res))
    out_secret_buf_size.contents.value = len(res)
    return True


class P256Keypair:
    """Represented a P256Keypair, should live longer than the one using it.

    Users are expected to hold a reference to the Keypair object.

    """

    def __init__(self):
        self._native_obj = None

    def __copy__(self):
        raise NotImplementedError("P256Keypair should not be copied.")

    def __deepcopy__(self, _=None):
        raise NotImplementedError("P256Keypair should not be copied.")

    def _create_native_object(self) -> c_void_p:
        handle = native.GetLibraryHandle()
        if not handle.pychip_NewP256Keypair.argtypes:
            setter = native.NativeLibraryHandleMethodArguments(handle)
            setter.Set("pychip_NewP256Keypair", c_void_p, [py_object,
                       _pychip_P256Keypair_ECDSA_sign_msg_func, _pychip_P256Keypair_ECDH_derive_secret_func])
            setter.Set("pychip_P256Keypair_UpdatePubkey", native.PyChipError, [c_void_p, POINTER(c_char), c_size_t])
            setter.Set("pychip_DeleteP256Keypair", None, [c_void_p])
        self._native_obj = handle.pychip_NewP256Keypair(
            py_object(self), _pychip_ECDSA_sign_msg, _pychip_ECDH_derive_secret)

        self.UpdatePublicKey()
        return self._native_obj

    def __del__(self):
        if self._native_obj is not None:
            handle = native.GetLibraryHandle()
            handle.pychip_DeleteP256Keypair(c_void_p(self._native_obj))
            self._native_obj = None

    @property
    def native_object(self) -> c_void_p:
        if self._native_obj is None:
            return self._create_native_object()
        return self._native_obj

    def UpdatePublicKey(self) -> None:
        ''' Update the PublicKey in the underlying C++ object.

        This function should be called when the implementation
        generates a new keypair.
        '''
        handle = native.GetLibraryHandle()
        handle.pychip_P256Keypair_UpdatePubkey(cast(self._native_obj, c_void_p),
                                               self.public_key, len(self.public_key)).raise_on_error()

    @abc.abstractproperty
    def public_key(self) -> bytes:
        ''' Returns the public key of the key pair

        The return value should conform with the uncompressed format of
        Section 2.3.3 of the SECG SEC 1 ("Elliptic Curve Cryptography")
        standard. (i.e. 0x04 || X || Y)

        For P256Keypair, the output length should be exactly 65 bytes.
        '''
        raise NotImplementedError()

    @abc.abstractmethod
    def ECDSA_sign_msg(self, message: bytes) -> bytes:
        raise NotImplementedError()

    @abc.abstractmethod
    def ECDH_derive_secret(self, remote_pubkey: bytes) -> bytes:
        ''' Derive shared secret from the local private key and remote public key.

        remote_pubkey will be a public key conforms with the uncompressed
        format of section 2.3.3 of the SECG SEC 1 standard.
        '''
        raise NotImplementedError()


class TestP256Keypair(P256Keypair):
    ''' The P256Keypair for testing purpose. It is not safe for any productions use
    '''

    def __init__(self, private_key: SigningKey = None):
        super().__init__()

        if private_key is None:
            self._key = SigningKey.generate(NIST256p)
        else:
            self._key = private_key

        self._pubkey = self._key.verifying_key.to_string(encoding='uncompressed')

    @property
    def public_key(self) -> bytes:
        return self._pubkey

    def ECDSA_sign_msg(self, message: bytes) -> bytes:
        return self._key.sign_deterministic(message, hashfunc=hashlib.sha256)

    def ECDH_derive_secret(self, remote_pubkey: bytes) -> bytes:
        ecdh = ECDH(curve=NIST256p)
        ecdh.load_private_key(self._key)
        ecdh.load_received_public_key_bytes(remote_pubkey[1:])
        return ecdh.ecdh1.generate_sharedsecret_bytes()
