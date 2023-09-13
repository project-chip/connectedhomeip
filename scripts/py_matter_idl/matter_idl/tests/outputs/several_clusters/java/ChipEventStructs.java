/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */
package chip.devicecontroller;

import javax.annotation.Nullable;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Optional;

public class ChipEventStructs {
  private interface ArrayCloneFunction<T> {
    T cloneFunction(T input) throws CloneNotSupportedException;
  }

  private static<T> ArrayList<T> arrayClone(ArrayList<T> inputArray, ArrayCloneFunction<T> func) {
    ArrayList<T> ret = new ArrayList<T>(inputArray.size());
    for (T it : inputArray) {
      try {
        ret.add(func.cloneFunction(it));
      } catch (CloneNotSupportedException e) {
        // Ignore
      }
    }
    return ret;
  }
}
