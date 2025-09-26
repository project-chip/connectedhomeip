/*
 *
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

#pragma once

#include <data-model-providers/codegen/CodegenDataModelProvider.h>
#include <data-model-providers/codegen/Instance.h>

namespace chip {

namespace TestDataModel {

// TODO:(#36837) implementing its own provider instead of using "CodegenDataModelProvider"
// TestServerCommandDispatch should provide its own dedicated data model provider rather than using CodegenDataModelProvider
// provider. This class exists solely for one specific test scenario, on a temporary basis.
class DispatchTestDataModel : public app::CodegenDataModelProvider
{
public:
    static DispatchTestDataModel & Instance()
    {
        static DispatchTestDataModel instance;
        return instance;
    }

    // The Startup method initializes the data model provider with a given context.
    // This approach ensures that the test relies on a more controlled and explicit data model provider
    // rather than depending on the code-generated one with undefined modifications.
    CHIP_ERROR Startup(app::DataModel::InteractionModelContext context) override
    {
        ReturnErrorOnFailure(CodegenDataModelProvider::Startup(context));
        return CHIP_NO_ERROR;
    }

protected:
    // Since the current unit tests do not involve any cluster implementations, we override InitDataModelForTesting
    // to do nothing, thereby preventing calls to the Ember-specific InitDataModelHandler.
    void InitDataModelForTesting() override {}
};

} // namespace TestDataModel
} // namespace chip
