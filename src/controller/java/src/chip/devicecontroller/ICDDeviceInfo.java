/*
 *   Copyright (c) 2020-2023 Project CHIP Authors
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

/** Class for holding ICD Device information. */
public class ICDDeviceInfo {
    private byte[] symmetricKey;

    public ICDDeviceInfo(byte[] symmetricKey) {
        this.symmetricKey = symmetricKey;
    }

    /** Returns the 16 bytes ICD symmetric key. */
    public byte[] getSymmetricKey() {
        return symmetricKey;
    }
}
