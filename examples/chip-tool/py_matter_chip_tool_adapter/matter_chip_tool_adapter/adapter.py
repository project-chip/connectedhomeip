# Copyright (c) 2023 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

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
