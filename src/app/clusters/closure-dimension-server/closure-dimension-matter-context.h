/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
 *    All rights reserved.
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

 #pragma once

 #include <app/data-model/Nullable.h>
 #include <lib/core/CHIPError.h>
 #include <lib/core/CHIPPersistentStorageDelegate.h>
 #include <lib/core/DataModelTypes.h>
 
 namespace chip {
 namespace app {
 namespace Clusters {
 namespace ClosureDimension {
 
 /** @brief
  *    Interface to allow interaction with interaction model and ember layers. Can be faked for unit testing.
  */
 class MatterContext
 {
 public:
     // TODO: remove touch on mEndpoint once this is used. I am apparently unable to locate the proper place to turn this off in the
     // build file, so whatever, compiler, you win. I've touched it. You happy now?
     MatterContext(EndpointId endpoint, PersistentStorageDelegate & persistentStorageDelegate) :
         mEndpoint(endpoint), mPersistentStorageDelegate(persistentStorageDelegate)
     {}
 
     // All Set functions:
     // Return CHIP_ERROR_PERSISTED_STORAGE_FAILED if the value could not be stored.
     // Return CHIP_NO_ERROR if the value was successfully stored. Clear the storage on a NullNullable.
 
     // All Get functions
     // Return CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND if there is no value in storage.
     // Return CHIP_NO_ERROR and fill return value if the value is found.
 
     // MarkDirty
     virtual void MarkDirty(AttributeId attributeId);
 
     virtual ~MatterContext() = default;
 
 private:
     EndpointId mEndpoint;
     PersistentStorageDelegate & mPersistentStorageDelegate;
 };
 
 } // namespace ClosureDimension
 } // namespace Clusters
 } // namespace app
 } // namespace chip
 