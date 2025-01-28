"""Data models for Matter testing infrastructure."""

import pathlib
import typing
from dataclasses import dataclass, field
from datetime import datetime, timedelta
from enum import Enum
from typing import Any, List, Optional, Union

import chip.clusters as Clusters
from chip.discovery import FilterType
from chip.setup_payload import SetupPayload
from chip.testing.global_attribute_ids import GlobalAttributeIds

from .constants import _DEFAULT_ADMIN_VENDOR_ID, _DEFAULT_CONTROLLER_NODE_ID, _DEFAULT_TRUST_ROOT_INDEX

# isort: off

from chip import ChipDeviceCtrl  # Needed before chip.FabricAdmin
from chip.ChipDeviceCtrl import CommissioningParameters

# isort: on

DiscoveryFilterType = ChipDeviceCtrl.DiscoveryFilterType


@dataclass
class TestStep:
    """Represents a single test step with its description and expectations."""
    test_plan_number: Union[int, str]
    description: str
    expectation: str = ""
    is_commissioning: bool = False

    def __str__(self):
        return f'{self.test_plan_number}: {self.description}\tExpected outcome: {self.expectation}'


@dataclass
class TestInfo:
    """Contains information about a test case."""
    function: str
    desc: str
    steps: List[TestStep]
    pics: List[str]


@dataclass
class AttributeValue:
    """Represents an attribute value with its metadata."""
    endpoint_id: int
    attribute: Any  # ClusterObjects.ClusterAttributeDescriptor
    value: Any
    timestamp_utc: Optional[datetime] = None


@dataclass
class MatterTestConfig:
    storage_path: pathlib.Path = pathlib.Path(".")
    logs_path: pathlib.Path = pathlib.Path(".")
    paa_trust_store_path: Optional[pathlib.Path] = None
    ble_interface_id: Optional[int] = None
    commission_only: bool = False

    admin_vendor_id: int = _DEFAULT_ADMIN_VENDOR_ID
    case_admin_subject: Optional[int] = None
    global_test_params: dict = field(default_factory=dict)
    # List of explicit tests to run by name. If empty, all tests will run
    tests: List[str] = field(default_factory=list)
    timeout: typing.Union[int, None] = None
    endpoint: typing.Union[int, None] = 0
    app_pid: int = 0
    fail_on_skipped_tests: bool = False

    commissioning_method: Optional[str] = None
    in_test_commissioning_method: Optional[str] = None
    discriminators: List[int] = field(default_factory=list)
    setup_passcodes: List[int] = field(default_factory=list)
    commissionee_ip_address_just_for_testing: Optional[str] = None
    # By default, we start with maximized cert chains, as required for RR-1.1.
    # This allows cert tests to be run without re-commissioning for RR-1.1.
    maximize_cert_chains: bool = True

    # By default, let's set validity to 10 years
    certificate_validity_period = int(timedelta(days=10*365).total_seconds())

    qr_code_content: List[str] = field(default_factory=list)
    manual_code: List[str] = field(default_factory=list)

    wifi_ssid: Optional[str] = None
    wifi_passphrase: Optional[str] = None
    thread_operational_dataset: Optional[str] = None

    pics: dict[bool, str] = field(default_factory=dict)

    # Node ID for basic DUT
    dut_node_ids: List[int] = field(default_factory=list)
    # Node ID to use for controller/commissioner
    controller_node_id: int = _DEFAULT_CONTROLLER_NODE_ID
    # CAT Tags for default controller/commissioner
    # By default, we commission with CAT tags specified for RR-1.1
    # so the cert tests can be run without re-commissioning the device
    # for this one test. This can be overwritten from the command line
    controller_cat_tags: List[int] = field(default_factory=lambda: [0x0001_0001])

    # Fabric ID which to use
    fabric_id: int = 1

    # "Alpha" by default
    root_of_trust_index: int = _DEFAULT_TRUST_ROOT_INDEX

    # If this is set, we will reuse root of trust keys at that location
    chip_tool_credentials_path: Optional[pathlib.Path] = None

    trace_to: List[str] = field(default_factory=list)

    # Accepted Terms and Conditions if used
    tc_version_to_simulate: int = None
    tc_user_response_to_simulate: int = None


@dataclass
class SetupParameters:
    passcode: int
    vendor_id: int = 0xFFF1
    product_id: int = 0x8001
    discriminator: int = 3840
    custom_flow: int = 0
    capabilities: int = 0b0100
    version: int = 0

    @property
    def qr_code(self):
        return SetupPayload().GenerateQrCode(self.passcode, self.vendor_id, self.product_id, self.discriminator,
                                             self.custom_flow, self.capabilities, self.version)

    @property
    def manual_code(self):
        return SetupPayload().GenerateManualPairingCode(self.passcode, self.vendor_id, self.product_id, self.discriminator,
                                                        self.custom_flow, self.capabilities, self.version)


@dataclass
class SetupPayloadInfo:
    """Information about setup payload."""
    filter_type: FilterType = FilterType.LONG_DISCRIMINATOR
    filter_value: int = 0
    passcode: int = 0


@dataclass
class CustomCommissioningParameters:
    commissioningParameters: CommissioningParameters
    randomDiscriminator: int


@dataclass
class ClusterPathLocation:
    """Location information for a cluster."""
    endpoint_id: int
    cluster_id: int

    def __str__(self):
        return (f'\n       Endpoint: {self.endpoint_id},'
                f'\n       Cluster:  {self.cluster_id}')


class ClusterMapper:
    """Describe clusters/attributes using schema names."""

    def __init__(self, legacy_cluster_mapping) -> None:
        self._mapping = legacy_cluster_mapping

    def get_cluster_string(self, cluster_id: int) -> str:
        mapping = self._mapping._CLUSTER_ID_DICT.get(cluster_id, None)
        if not mapping:
            return f"Cluster Unknown ({cluster_id}, 0x{cluster_id:08X})"
        else:
            name = mapping["clusterName"]
            return f"Cluster {name} ({cluster_id}, 0x{cluster_id:04X})"

    def get_attribute_string(self, cluster_id: int, attribute_id) -> str:
        global_attrs = [item.value for item in GlobalAttributeIds]
        if attribute_id in global_attrs:
            return f"Attribute {GlobalAttributeIds(attribute_id).to_name()} {attribute_id}, 0x{attribute_id:04X}"
        mapping = self._mapping._CLUSTER_ID_DICT.get(cluster_id, None)
        if not mapping:
            return f"Attribute Unknown ({attribute_id}, 0x{attribute_id:08X})"
        else:
            attribute_mapping = mapping["attributes"].get(attribute_id, None)

            if not attribute_mapping:
                return f"Attribute Unknown ({attribute_id}, 0x{attribute_id:08X})"
            else:
                attribute_name = attribute_mapping["attributeName"]
                return f"Attribute {attribute_name} ({attribute_id}, 0x{attribute_id:04X})"


def id_str(id):
    return f'{id} (0x{id:02x})'


def cluster_id_str(id):
    if id in Clusters.ClusterObjects.ALL_CLUSTERS.keys():
        s = Clusters.ClusterObjects.ALL_CLUSTERS[id].__name__
    else:
        s = "Unknown cluster"
    try:
        return f'{id_str(id)} {s}'
    except TypeError:
        return 'HERE IS THE PROBLEM'


@dataclass
class AttributePathLocation(ClusterPathLocation):
    cluster_id: Optional[int] = None
    attribute_id: Optional[int] = None

    def as_cluster_string(self, mapper: ClusterMapper):
        desc = f"Endpoint {self.endpoint_id}"
        if self.cluster_id is not None:
            desc += f", {mapper.get_cluster_string(self.cluster_id)}"
        return desc

    def as_string(self, mapper: ClusterMapper):
        desc = self.as_cluster_string(mapper)
        if self.cluster_id is not None and self.attribute_id is not None:
            desc += f", {mapper.get_attribute_string(self.cluster_id, self.attribute_id)}"

        return desc

    def __str__(self):
        return (f'{super().__str__()}'
                f'\n      Attribute:{id_str(self.attribute_id)}')


@dataclass
class EventPathLocation(ClusterPathLocation):
    """Path location information for an event."""
    event_id: int

    def __str__(self):
        return (f'{super().__str__()}'
                f'\n       Event:    {self.event_id}')


@dataclass
class CommandPathLocation(ClusterPathLocation):
    """Path location information for a command."""
    command_id: int

    def __str__(self):
        return (f'{super().__str__()}'
                f'\n       Command:  {self.command_id}')


@dataclass
class FeaturePathLocation(ClusterPathLocation):
    """Path location information for a feature."""
    feature_code: str

    def __str__(self):
        return (f'{super().__str__()}'
                f'\n       Feature:  {self.feature_code}')


@dataclass
class DeviceTypePathLocation:
    """Path location information for a device type."""
    device_type_id: int
    cluster_id: Optional[int] = None

    def __str__(self):
        msg = f'\n       DeviceType: {self.device_type_id}'
        if self.cluster_id:
            msg += f'\n       ClusterID: {self.cluster_id}'
        return msg


ProblemLocation = typing.Union[ClusterPathLocation, DeviceTypePathLocation]

# ProblemSeverity is not using StrEnum, but rather Enum, since StrEnum only
# appeared in 3.11. To make it JSON serializable easily, multiple inheritance
# from `str` is used. See https://stackoverflow.com/a/51976841.


class ProblemSeverity(str, Enum):
    NOTE = "NOTE"
    WARNING = "WARNING"
    ERROR = "ERROR"


@dataclass
class ProblemNotice:
    """Notice about a problem encountered during testing."""
    test_name: str
    location: ProblemLocation
    severity: ProblemSeverity
    problem: str
    spec_location: str = ""

    def __str__(self):
        return (f'\nProblem: {str(self.severity)}'
                f'\n    test_name: {self.test_name}'
                f'\n    location: {str(self.location)}'
                f'\n    problem: {self.problem}'
                f'\n    spec_location: {self.spec_location}\n')
