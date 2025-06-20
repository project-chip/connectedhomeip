/**
 *    Copyright (c) 2025 Project CHIP Authors
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
#include <data-model-providers/codegen/EmberAttributeStorageImpl.h>

#include <app/util/persistence/AttributePersistenceProvider.h>

namespace chip {
namespace app {
namespace Storage {

CHIP_ERROR EmberAttributeStorageImpl::Write(const ConcreteAttributePath & path, const Value & value) {
    // FIXME: implement
    return CHIP_ERROR_NOT_IMPLEMENTED;

}
CHIP_ERROR EmberAttributeStorageImpl::Read(const ConcreteAttributePath & path, Buffer &buffer) {
    // FIXME: implement
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

} // namespace Storage
} // namespace app
} // namespace chip
