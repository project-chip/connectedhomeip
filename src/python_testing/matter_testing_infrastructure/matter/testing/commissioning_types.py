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


This module provides common data structures for commissioning procedures that
are intentionally decoupled from heavy dependencies such as `matter.clusters`
or the `ChipDeviceController`. That keeps Pigweed-isolated tests
(e.g. ``test_matter_asserts``) working when the full ``matter`` package is not
on ``sys.path``.
"""

from __future__ import annotations

from dataclasses import dataclass
from typing import Any, Optional


@dataclass
class PaseParams:
    """
    Optional credentials for establishing a PASE session.

    Used when a secure connection via operational certificates (CASE) is unavailable, typically during initial commissioning. Provide either the ``setup_code`` or both the ``discriminator`` and ``passcode``.

    Attributes:
        setup_code: The full QR or manual pairing code string.
        discriminator: The device's long or short discriminator.
        passcode: The setup PIN code (passcode).
    """
    setup_code: Optional[str] = None
    discriminator: Optional[int] = None
    passcode: Optional[int] = None

    def resolve_setup_code(self, dev_ctrl: Any) -> Optional[str]:
        """Resolve manual pairing code via the controller when needed."""
        if self.setup_code:
            return self.setup_code
        if self.passcode is not None and self.discriminator is not None:
            return dev_ctrl.CreateManualCode(self.discriminator, self.passcode)
        return None
