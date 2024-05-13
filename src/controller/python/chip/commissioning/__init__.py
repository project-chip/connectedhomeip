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
import dataclasses
import enum
import os
from typing import Set, Tuple, Union

ROOT_ENDPOINT_ID = 0


@dataclasses.dataclass
class CommissioneeInfo:
    endpoints: Set[int]
    is_thread_device: bool = False
    is_wifi_device: bool = False
    is_ethernet_device: bool = False


class RegulatoryLocationType(enum.IntEnum):
    INDOOR = 0
    OUTDOOR = 1
    INDOOR_OUTDOOR = 2


@dataclasses.dataclass
class RegulatoryConfig:
    location_type: RegulatoryLocationType
    country_code: str


@dataclasses.dataclass
class PaseParameters:
    setup_pin: int
    temporary_nodeid: int


@dataclasses.dataclass
class PaseOverBLEParameters(PaseParameters):
    discriminator: int

    def __str__(self):
        return f"BLE:0x{self.discriminator:03x}"


@dataclasses.dataclass
class PaseOverIPParameters(PaseParameters):
    long_discriminator: int

    def __str__(self):
        return f"Discriminator:0x{self.long_discriminator:03x}"


@dataclasses.dataclass
class WiFiCredentials:
    ssid: bytes
    passphrase: bytes


@dataclasses.dataclass
class Parameters:
    pase_param: Union[PaseOverBLEParameters, PaseOverIPParameters]
    regulatory_config: RegulatoryConfig
    fabric_label: str
    commissionee_info: CommissioneeInfo
    wifi_credentials: WiFiCredentials
    thread_credentials: bytes
    failsafe_expiry_length_seconds: int = 600


class NetworkCommissioningFeatureMap(enum.IntEnum):
    WIFI_NETWORK_FEATURE_MAP = 1
    THREAD_NETWORK_FEATURE_MAP = 2


class CommissionFailure(Exception):
    def __init__(self, msg):
        self.msg = msg

    def __str__(self):
        return f"CommissionFailure({self.msg})"


@dataclasses.dataclass
class GetCommissioneeCredentialsRequest:
    dac: bytes
    pai: bytes
    attestation_signature: bytes
    attestation_nonce: bytes
    attestation_elements: bytes
    csr_signature: bytes
    csr_nonce: bytes
    csr_elements: bytes
    vendor_id: int
    product_id: int


@dataclasses.dataclass
class GetCommissioneeCredentialsResponse:
    rcac: bytes
    noc: bytes
    icac: bytes
    ipk: bytes
    case_admin_node: int
    admin_vendor_id: int
    node_id: int = None
    fabric_id: int = None


class CredentialProvider:
    async def get_commissionee_nonces(self) -> Tuple[bytes, bytes]:
        ''' Returns the `attestation_nonce` and `csr_nonce` for the commissionee.
        '''
        return os.urandom(32), os.urandom(32)

    @abc.abstractmethod
    async def get_commissionee_credentials(self, request: GetCommissioneeCredentialsRequest) -> GetCommissioneeCredentialsResponse:
        ''' Returns certifications and infomations for the commissioning.
        '''
        raise NotImplementedError()


class ExampleCredentialProvider:
    async def get_commissionee_credentials(self, request: GetCommissioneeCredentialsRequest) -> GetCommissioneeCredentialsResponse:
        pass
