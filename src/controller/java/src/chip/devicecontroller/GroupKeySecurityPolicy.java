/*
 *   Copyright (c) 2023 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

package chip.devicecontroller;

public enum GroupKeySecurityPolicy {
  TrustFirst(0),
  CacheAndSync(1);

  private final int id;

  GroupKeySecurityPolicy(int id) {
    this.id = id;
  }

  public int getID() {
    return id;
  }

  public static GroupKeySecurityPolicy value(int id) throws IllegalArgumentException {
    for (GroupKeySecurityPolicy policy : GroupKeySecurityPolicy.values()) {
      if (policy.id == id) {
        return policy;
      }
    }
    // Throw an exception or return a default value if no match is found.
    throw new IllegalArgumentException("Invalid id: " + id);
  }
}
