# SPDX-FileCopyrightText: 2023 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0

from abc import ABC, abstractmethod


class TestAdapter(ABC):
    """
    TestAdapter is an abstract interface that defines the set of methods an adapter
    should implement.

    Adapters are used to translate test step requests and test responses back and forth
    between the format used by the matter_yamltests package and the format used by the
    adapter target.

    Some examples of adapters includes chip-repl, chip-tool and the placeholder applications.
    """

    @abstractmethod
    def encode(self, request):
        """Encode a test step request from the matter_yamltests format to the adapter format."""
        pass

    @abstractmethod
    def decode(self, response):
        """
        Decode a test step response from the adapter format to the matter_yamltests format.

        This method returns a tuple containing both the decoded response and additional logs
        from the adapter.
        """
        pass
