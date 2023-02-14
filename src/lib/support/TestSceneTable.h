/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <app/clusters/scenes/SceneTableImpl.h>

namespace chip {

namespace SceneTesting {

using FabricIndex        = chip::FabricIndex;
using SceneTableEntry    = chip::scenes::DefaultSceneTableImpl::SceneTableEntry;
using SceneTableImpl     = chip::scenes::DefaultSceneTableImpl;
using SceneStorageId     = chip::scenes::DefaultSceneTableImpl::SceneStorageId;
using SceneData          = chip::scenes::DefaultSceneTableImpl::SceneData;
using ExtensionFieldsSet = chip::scenes::ExtensionFieldsSet;

static const ExtensionFieldsSet onOffEFS1        = ExtensionFieldsSet(0x0006, (uint8_t *) "1", 1);
static const ExtensionFieldsSet levelControlEFS1 = ExtensionFieldsSet(0x0008, (uint8_t *) "511", 3);

static CHIP_ERROR test_on_off_from_cluster_callback(ExtensionFieldsSet & fields)
{
    ReturnErrorOnFailure(fields = onOffEFS1);
    return CHIP_NO_ERROR;
}
static CHIP_ERROR test_on_off_to_cluster_callback(ExtensionFieldsSet & fields)
{
    VerifyOrReturnError(fields == onOffEFS1, CHIP_ERROR_WRITE_FAILED);
    return CHIP_NO_ERROR;
}
static CHIP_ERROR test_level_control_from_cluster_callback(ExtensionFieldsSet & fields)
{
    ReturnErrorOnFailure(fields = levelControlEFS1);
    return CHIP_NO_ERROR;
}
static CHIP_ERROR test_level_control_to_cluster_callback(ExtensionFieldsSet & fields)
{
    VerifyOrReturnError(fields == levelControlEFS1, CHIP_ERROR_WRITE_FAILED);
    return CHIP_NO_ERROR;
}

CHIP_ERROR scene_store_test(SceneTableImpl * provider, FabricIndex fabric_index, SceneTableEntry & entry)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    SceneTableEntry temp;

    LogErrorOnFailure(provider->SetSceneTableEntry(fabric_index, entry));
    LogErrorOnFailure(provider->GetSceneTableEntry(fabric_index, entry.mStorageId, temp));
    VerifyOrReturnError(temp.mStorageId == entry.mStorageId, CHIP_ERROR_WRITE_FAILED);
    VerifyOrReturnError(temp.mStorageData == entry.mStorageData, CHIP_ERROR_WRITE_FAILED);

    return err;
}

CHIP_ERROR scene_iterator_test(SceneTableImpl * provider, FabricIndex fabric_index, const SceneTableEntry & entry1,
                               const SceneTableEntry & entry2, const SceneTableEntry & entry3)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    SceneTableEntry temp;

    auto * iterator = provider->IterateSceneEntry(fabric_index);
    if (iterator)
    {
        VerifyOrReturnError(iterator->Count() == 3, CHIP_ERROR_INVALID_ARGUMENT);

        VerifyOrReturnError(iterator->Next(temp), CHIP_ERROR_INVALID_ACCESS_TOKEN);
        VerifyOrReturnError(temp.mStorageId == entry1.mStorageId, CHIP_ERROR_INVALID_ARGUMENT);

        VerifyOrReturnError(iterator->Next(temp), CHIP_ERROR_INVALID_ACCESS_TOKEN);
        VerifyOrReturnError(temp.mStorageId == entry2.mStorageId, CHIP_ERROR_INVALID_ARGUMENT);

        VerifyOrReturnError(iterator->Next(temp), CHIP_ERROR_INVALID_ACCESS_TOKEN);
        VerifyOrReturnError(temp.mStorageId == entry3.mStorageId, CHIP_ERROR_INVALID_ARGUMENT);

        // Iterator should return false here
        VerifyOrReturnError(iterator->Next(temp) == false, CHIP_ERROR_INVALID_ACCESS_TOKEN);

        iterator->Release();
    }

    return err;
}

CHIP_ERROR scene_remove_test(SceneTableImpl * provider, FabricIndex fabric_index, SceneTableEntry & entry1,
                             SceneTableEntry & entry2, SceneTableEntry & entry3)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    SceneTableEntry temp;

    LogErrorOnFailure(provider->RemoveSceneTableEntry(fabric_index, entry2.mStorageId));

    auto * iterator = provider->IterateSceneEntry(fabric_index);
    VerifyOrReturnError(iterator->Count() == 2, CHIP_ERROR_INVALID_ARGUMENT);
    iterator->Next(temp);
    VerifyOrReturnError(temp.mStorageId == entry1.mStorageId, CHIP_ERROR_INVALID_ARGUMENT);
    iterator->Release();

    LogErrorOnFailure(provider->RemoveSceneTableEntry(fabric_index, entry1.mStorageId));
    iterator = provider->IterateSceneEntry(fabric_index);
    VerifyOrReturnError(iterator->Count() == 1, CHIP_ERROR_INVALID_ARGUMENT);
    iterator->Next(temp);
    VerifyOrReturnError(temp.mStorageId == entry3.mStorageId, CHIP_ERROR_INVALID_ARGUMENT);

    LogErrorOnFailure(provider->RemoveSceneTableEntry(fabric_index, entry3.mStorageId));
    iterator = provider->IterateSceneEntry(fabric_index);
    VerifyOrReturnError(iterator->Count() == 0, CHIP_ERROR_INVALID_ARGUMENT);

    // Iterator should return false here
    VerifyOrReturnError(iterator->Next(temp) == false, CHIP_ERROR_INVALID_ACCESS_TOKEN);
    iterator->Release();

    return err;
}

CHIP_ERROR TestSceneData(SceneTableImpl * provider, FabricIndex fabric_index)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    // Scene storage ID
    static const SceneStorageId sceneId1(1, 0xAA, 0x101);
    static const SceneStorageId sceneId2(1, 0xBB, 0x00);
    static const SceneStorageId sceneId3(1, 0xCC, 0x102);

    // Scene data
    static const SceneData sceneData1(CharSpan("Scene #1", sizeof("Scene #1")));
    static const SceneData sceneData2(CharSpan("Scene #2", sizeof("Scene #2")), 2, 5);
    static const SceneData sceneData3(CharSpan(), 25);

    // Scenes
    SceneTableEntry scene1(sceneId1, sceneData1);
    SceneTableEntry scene2(sceneId2, sceneData2);
    SceneTableEntry scene3(sceneId3, sceneData3);

    err = provider->registerHandler(onOffEFS1.mID, &test_on_off_from_cluster_callback, &test_on_off_to_cluster_callback);
    LogErrorOnFailure(err);
    err = provider->registerHandler(levelControlEFS1.mID, &test_level_control_from_cluster_callback,
                                    &test_level_control_to_cluster_callback);
    LogErrorOnFailure(err);
    err = provider->EFSValuesFromCluster(scene1.mStorageData.mExtentsionFieldsSets);
    LogErrorOnFailure(err);

    // Tests
    err = scene_store_test(provider, fabric_index, scene1);
    LogErrorOnFailure(err);

    err = provider->EFSValuesToCluster(scene1.mStorageData.mExtentsionFieldsSets);
    LogErrorOnFailure(err);

    err = scene_store_test(provider, fabric_index, scene2);
    LogErrorOnFailure(err);
    err = scene_store_test(provider, fabric_index, scene3);
    LogErrorOnFailure(err);

    err = scene_iterator_test(provider, fabric_index, scene1, scene2, scene3);
    LogErrorOnFailure(err);
    err = scene_remove_test(provider, fabric_index, scene1, scene2, scene3);
    LogErrorOnFailure(err);

    return err;
}

} // namespace SceneTesting

} // namespace chip
