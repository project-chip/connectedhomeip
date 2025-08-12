# src/python_testing/matter_testing_infrastructure/matter/typings/matter/testing/decorators.pyi

from enum import IntFlag
from typing import TYPE_CHECKING, Callable

# Assume types from matter.clusters can be imported.
from matter.clusters import Attribute
from matter.clusters import ClusterObjects as ClusterObjects

# Type alias matching the one in decorators.py
EndpointCheckFunction = Callable[[Attribute.AsyncReadTransaction.ReadResponse, int], bool]

# --- Public Factory Functions ---


def has_cluster(cluster: ClusterObjects.ClusterObjectDescriptor) -> EndpointCheckFunction: ...


def has_attribute(attribute: ClusterObjects.ClusterAttributeDescriptor) -> EndpointCheckFunction: ...


def has_command(command: ClusterObjects.ClusterCommand) -> EndpointCheckFunction: ...

# The 'cluster' parameter is the descriptor (e.g., Clusters.OnOff)


def has_feature(cluster: ClusterObjects.ClusterObjectDescriptor, feature: IntFlag) -> EndpointCheckFunction: ...


# --- Other Public Decorators/Functions ---

# Forward reference MatterBaseTest using string to avoid circular import issues if necessary
if TYPE_CHECKING:
    pass


def async_test_body(body: Callable) -> Callable: ...


def run_if_endpoint_matches(accept_function: EndpointCheckFunction) -> Callable: ...


def run_on_singleton_matching_endpoint(accept_function: EndpointCheckFunction) -> Callable: ...

# NOTE: We don't define the internal functions (_has_cluster, _has_feature, etc.)
# in the stub file, as mypy primarily checks the external interface based on the stub.
# Type errors *within* the actual decorators.py implementation might still occur
# if the internal logic clashes with types inferred from the outside context provided by this stub.
