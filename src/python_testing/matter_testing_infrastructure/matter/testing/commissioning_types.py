#
#    Copyright (c) 2026 Project CHIP Authors
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

"""Lightweight commissioning-related types (no controller / cluster imports).


This module provides :class:`CustomCommissioningParameters` and other common
data structures for commissioning procedures that are intentionally decoupled
from heavy dependencies such as `matter.clusters` or the `ChipDeviceController`.
That keeps Pigweed-isolated tests (e.g. ``test_matter_asserts``) working when the full ``matter`` package is not
on ``sys.path``.
"""

from __future__ import annotations

from dataclasses import dataclass
from typing import Any, Optional


@dataclass
class CustomCommissioningParameters:
    """
    Wraps commissioning-window output from the SDK plus optional standalone pairing data.

    For the **administrator commissioning window** path, this matches the historical
    shape: ``OpenCommissioningWindow`` returns ``CommissioningParameters`` and tests
    pair it with ``randomDiscriminator`` for ``CommissionOnNetwork`` discovery.

    For **standalone PASE** (e.g. composition tests with ``first_setup_code`` only),
    use ``setup_code`` and/or ``discriminator`` + ``passcode``; leave the ECM fields
    unset. Use :meth:`resolve_setup_code` for the string passed to
    ``FindOrEstablishPASESession``.

    Attributes:
        commissioningParameters (Optional[Any]):
            SDK commissioning parameters from ``OpenCommissioningWindow`` for the ECM path;
            otherwise None.
        randomDiscriminator (Optional[int]):
            Long discriminator used with ``CommissionOnNetwork`` when using the ECM path;
            otherwise None.
        setup_code (Optional[str]): QR or manual pairing code string for PASE-only flows.
        discriminator (Optional[int]): With ``passcode``, used to build a manual code when
            ``setup_code`` is not set and ECM fields are not used.
        passcode (Optional[int]): With ``discriminator``, for manual code synthesis.
    """

    commissioningParameters: Optional[Any] = None
    randomDiscriminator: Optional[int] = None
    setup_code: Optional[str] = None
    discriminator: Optional[int] = None
    passcode: Optional[int] = None

    def resolve_setup_code(self, dev_ctrl: Any) -> Optional[str]:
        """Return a setup string for PASE, or None if credentials are incomplete."""
        if self.setup_code:
            return self.setup_code

        cp = self.commissioningParameters
        if cp is not None:
            qr = getattr(cp, "setupQRCode", None) or ""
            if qr:
                return str(qr)
            manual = getattr(cp, "setupManualCode", None) or ""
            if manual:
                return str(manual)
            pin = getattr(cp, "setupPinCode", None)
            if pin is not None and self.randomDiscriminator is not None:
                return dev_ctrl.CreateManualCode(self.randomDiscriminator, pin)

        if self.passcode is not None and self.discriminator is not None:
            return dev_ctrl.CreateManualCode(self.discriminator, self.passcode)
        return None
