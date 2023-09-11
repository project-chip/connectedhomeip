# SPDX-FileCopyrightText: 2023 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0

from .decoder import Decoder
from .encoder import Encoder


class Adapter:
    def __init__(self, specifications):
        self.encoder = Encoder(specifications)
        self.decoder = Decoder(specifications)

    def encode(self, request):
        return self.encoder.encode(request)

    def decode(self, response):
        return self.decoder.decode(response)
