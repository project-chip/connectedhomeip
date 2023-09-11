#
# SPDX-FileCopyrightText: 2021-2022 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
#

from typing import Optional

# Represents the node ID that is to be used when creating device
# controllers/commissioning devices
_local_node_id: Optional[int] = None
_local_cat: Optional[int] = None

DEFAULT_LOCAL_NODE_ID = 12345
DEFAULT_COMMISSIONER_CAT = 0xABCD0010


def SetLocalNodeId(node_id: int):
    """Local node id. Can be set at the start of scripts, however once set
       it cannot be reassigned.
    """
    global _local_node_id

    if _local_node_id is not None:
        raise Exception('Local node id is already set.')

    _local_node_id = node_id


def GetLocalNodeId() -> int:
    """Returns the current local node id. If none has been set, a default is set and
    used."""
    global _local_node_id

    if _local_node_id is None:
        SetLocalNodeId(DEFAULT_LOCAL_NODE_ID)

    return _local_node_id


def SetCommissionerCAT(cat: int):
    """Local (controllers/commissioning) device CASE Authenticated Tag (CAT).
       Can be set at the start of scripts, however once set it cannot be reassigned.
    """
    global _local_cat

    if _local_cat is not None:
        raise Exception('Local CAT is already set.')

    _local_cat = cat


def GetCommissionerCAT() -> int:
    """Returns the current local (controllers/commissioning) device CAT. If none has been set,
       a default is set and used."""
    global _local_cat

    if _local_cat is None:
        SetCommissionerCAT(DEFAULT_COMMISSIONER_CAT)

    return _local_cat
