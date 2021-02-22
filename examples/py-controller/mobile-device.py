"""
Copyright (c) 2021 Project CHIP Authors
All rights reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
"""

"""
mobile-device.py
  Simulates a mobile device node and send messages to device using
  Python Device Controller API. Cirque tests will use this file for E2E test.
  This file also acts as a example of using the raw python controller API.
"""




import chip
def main(argv: List[str]) -> None:
    if len(argv) > 1:
        raise app.UsageError('Too many command-line arguments.')


if __name__ == '__main__':
    app.run(main)
