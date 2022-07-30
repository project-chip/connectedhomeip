/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include "AppPlatformShellCommands-JNI.h"
#include "AppImpl.h"
#include "TvApp-JNI.h"
#include <access/AccessControl.h>
#include <jni.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CHIPJNIError.h>
#include <lib/support/JniReferences.h>
#include <lib/support/JniTypeWrappers.h>

#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
#include <app/app-platform/ContentAppPlatform.h>
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED

using namespace chip;

char response[1024];

using namespace ::chip::Controller;
#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
using namespace chip::AppPlatform;
using namespace chip::Access;
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
using namespace chip::app::Clusters;

#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED

static CHIP_ERROR pairApp(bool printHeader, size_t index)
{
    // TODO: add pair app
    return CHIP_NO_ERROR;
}

char * AppPlatformHandler(int argc, char ** argv)
{
    if (argc == 0 || strcmp(argv[0], "help") == 0)
    {
        strcpy(response, "check usage instructions on the UI");
    }
    else if (strcmp(argv[0], "add-admin-vendor") == 0)
    {
        if (argc < 2)
        {
            strcpy(response, "check usage instructions on the UI");
        }
        char * eptr;

        uint16_t vid                    = (uint16_t) strtol(argv[1], &eptr, 10);
        ContentAppFactoryImpl * factory = GetContentAppFactoryImpl();
        factory->AddAdminVendorId(vid);

        ChipLogProgress(DeviceLayer, "added admin-vendor");

        strcpy(response, "added admin-vendor");

        return response;
    }
    else if (strcmp(argv[0], "add") == 0)
    {
        if (argc < 2)
        {
            strcpy(response, "check usage instructions on the UI");
        }
        char * eptr;

        uint16_t vid = (uint16_t) strtol(argv[1], &eptr, 10);
        uint16_t pid = 0;
        if (argc >= 3)
        {
            pid = (uint16_t) strtol(argv[2], &eptr, 10);
        }
        ContentAppPlatform::GetInstance().LoadContentAppByClient(vid, pid);

        ChipLogProgress(DeviceLayer, "added app");

        strcpy(response, "added app");

        return response;
    }
    else if (strcmp(argv[0], "remove-app-access") == 0)
    {
        Access::GetAccessControl().DeleteAllEntriesForFabric(GetDeviceCommissioner()->GetFabricIndex());
        strcpy(response, "removed app access");

        return response;
    }
    else if (strcmp(argv[0], "remove") == 0)
    {
        if (argc < 2)
        {
            strcpy(response, "check usage instructions on the UI");
        }
        char * eptr;

        uint16_t endpoint = (uint16_t) strtol(argv[1], &eptr, 10);
        ContentApp * app  = ContentAppPlatform::GetInstance().GetContentApp(endpoint);
        if (app == nullptr)
        {
            ChipLogProgress(DeviceLayer, "app not found");

            strcpy(response, "app not found");
            return response;
        }
        ContentAppPlatform::GetInstance().RemoveContentApp(app);

        ChipLogProgress(DeviceLayer, "removed app");

        strcpy(response, "removed app");

        return response;
    }
    else if (strcmp(argv[0], "setpin") == 0)
    {
        if (argc < 3)
        {
            strcpy(response, "check usage instructions on the UI");
        }
        char * eptr;

        uint16_t endpoint = (uint16_t) strtol(argv[1], &eptr, 10);
        char * pincode    = argv[2];
        ContentApp * app  = ContentAppPlatform::GetInstance().GetContentApp(endpoint);
        if (app == nullptr)
        {
            ChipLogProgress(DeviceLayer, "app not found");
            strcpy(response, "app not found");
            return response;
        }
        if (app->GetAccountLoginDelegate() == nullptr)
        {
            ChipLogProgress(DeviceLayer, "no AccountLogin cluster for app with endpoint id=%d ", endpoint);

            strcpy(response, "no AccountLogin cluster for app with endpoint");
            return response;
        }
        app->GetAccountLoginDelegate()->SetSetupPin(pincode);

        ChipLogProgress(DeviceLayer, "set pin success");

        strcpy(response, "set pin success");

        return response;
    }
    else if (strcmp(argv[0], "commission") == 0)
    {
        if (argc < 2)
        {
            strcpy(response, "check usage instructions on the UI");
        }
        char * eptr;
        size_t index = (size_t) strtol(argv[1], &eptr, 10);
        pairApp(true, index);
        strcpy(response, "no supported atm");
        return response;
    }
    else
    {
        strcpy(response, "invalid argument");
        return response;
    }
    return response;
}

#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED

#define JNI_METHOD(RETURN, METHOD_NAME)                                                                                            \
    extern "C" JNIEXPORT RETURN JNICALL Java_com_matter_tv_server_tvapp_AppPlatformShellCommands_##METHOD_NAME

JNI_METHOD(jstring, OnExecuteCommand)(JNIEnv * env, jobject, jobjectArray stringArray)
{
    int argc     = env->GetArrayLength(stringArray);
    char ** argv = new char *[(uint) argc];

    // Fill in argv
    for (int i = 0; i < argc; i++)
    {
        jstring string = (jstring)(env->GetObjectArrayElement(stringArray, i));
        argv[i]        = (char *) env->GetStringUTFChars(string, 0);
    }

    // Store response to show it to the users
    char * buf = AppPlatformHandler(argc, argv);

    // Release UTF Chars
    for (int i = 0; i < argc; i++)
    {
        ChipLogProgress(DeviceLayer, " Value=%s ", argv[i]);
        jstring string = (jstring)(env->GetObjectArrayElement(stringArray, i));
        env->ReleaseStringUTFChars(string, argv[i]);
    }

    return env->NewStringUTF(buf);
}
