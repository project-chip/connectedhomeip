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

    if (printHeader)
    {
        char str[64];
        sprintf(str, "udc-commission %ld\r\n", static_cast<long>(index));
        strcat(response, str);
    }

    DeviceCommissioner * commissioner = GetDeviceCommissioner();
    UDCClientState * state = commissioner->GetUserDirectedCommissioningServer()->GetUDCClients().GetUDCClientState(index);
    if (state == nullptr)
    {
        char str[64];
        sprintf(str, "udc client[%ld] null \r\n", static_cast<long>(index));
        strcat(response, str);
    }
    else
    {
        ContentApp * app = ContentAppPlatform::GetInstance().LoadContentAppByClient(state->GetVendorId(), state->GetProductId());
        if (app == nullptr)
        {
            char str[64];
            sprintf(str, "no app found for vendor id=%d \r\n", state->GetVendorId());
            strcat(response, str);
            return CHIP_ERROR_BAD_REQUEST;
        }

        if (app->GetAccountLoginDelegate() == nullptr)
        {
            char str[64];
            sprintf(str, "no AccountLogin cluster for app with vendor id=%d \r\n", state->GetVendorId());
            strcat(response, str);
            return CHIP_ERROR_BAD_REQUEST;
        }

        char rotatingIdString[chip::Dnssd::kMaxRotatingIdLen * 2 + 1] = "";
        Encoding::BytesToUppercaseHexString(state->GetRotatingId(), state->GetRotatingIdLength(), rotatingIdString,
                                            sizeof(rotatingIdString));

        CharSpan rotatingIdSpan = CharSpan(rotatingIdString, strlen(rotatingIdString));

        static const size_t kSetupPinSize = 12;
        char setupPin[kSetupPinSize];

        app->GetAccountLoginDelegate()->GetSetupPin(setupPin, kSetupPinSize, rotatingIdSpan);
        std::string pinString(setupPin);

        char * eptr;
        uint32_t pincode = (uint32_t) strtol(pinString.c_str(), &eptr, 10);
        if (pincode == 0)
        {
            char str[64];
            sprintf(str, "udc no pin returned for vendor id=%d rotating ID=%s \r\n", state->GetVendorId(), rotatingIdString);
            strcat(response, str);
            return CHIP_ERROR_BAD_REQUEST;
        }

        return CommissionerPairUDC(pincode, index);
    }
    return CHIP_NO_ERROR;
}

void DumpAccessControlEntry(const Access::AccessControl::Entry & entry)
{

    CHIP_ERROR err;

    {
        FabricIndex fabricIndex;
        SuccessOrExit(err = entry.GetFabricIndex(fabricIndex));
        char str[64];
        sprintf(str, "fabricIndex: %u\n", fabricIndex);
        strcat(response, str);
    }

    {
        Privilege privilege;
        SuccessOrExit(err = entry.GetPrivilege(privilege));
        char str[64];
        sprintf(str, "privilege: %d\n", to_underlying(privilege));
        strcat(response, str);
    }

    {
        AuthMode authMode;
        SuccessOrExit(err = entry.GetAuthMode(authMode));
        char str[64];
        sprintf(str, "authMode: %d\n", to_underlying(authMode));
        strcat(response, str);
    }

    {
        size_t count;
        SuccessOrExit(err = entry.GetSubjectCount(count));
        if (count)
        {

            char str[64];
            sprintf(str, "subjects: %u\n", static_cast<unsigned>(count));
            strcat(response, str);

            for (size_t i = 0; i < count; ++i)
            {
                NodeId subject;
                SuccessOrExit(err = entry.GetSubject(i, subject));

                char buffer[64];
                sprintf(buffer, "  %u: 0x" ChipLogFormatX64, static_cast<unsigned>(i), ChipLogValueX64(subject));
                strcat(response, buffer);
            }
        }
    }

    {
        size_t count;
        SuccessOrExit(err = entry.GetTargetCount(count));
        if (count)
        {

            char str[64];
            sprintf(str, "\ntargets: %u\n", static_cast<unsigned>(count));
            strcat(response, str);

            for (size_t i = 0; i < count; ++i)
            {
                Access::AccessControl::Entry::Target target;
                SuccessOrExit(err = entry.GetTarget(i, target));
                char buffer[64];
                if (target.flags & Access::AccessControl::Entry::Target::kCluster)
                {
                    sprintf(buffer, "  %u: cluster: 0x" ChipLogFormatMEI, static_cast<unsigned>(i),
                            ChipLogValueMEI(target.cluster));
                    strcat(buffer, "\n");
                    strcat(response, buffer);
                }
                if (target.flags & Access::AccessControl::Entry::Target::kEndpoint)
                {
                    sprintf(buffer, "  %u: endpoint: %u", static_cast<unsigned>(i), target.endpoint);
                    strcat(buffer, "\n");
                    strcat(response, buffer);
                }
                if (target.flags & Access::AccessControl::Entry::Target::kDeviceType)
                {
                    sprintf(buffer, "  %u: deviceType: 0x" ChipLogFormatMEI, static_cast<unsigned>(i),
                            ChipLogValueMEI(target.deviceType));
                    strcat(buffer, "\n");
                    strcat(response, buffer);
                }
            }
        }
    }

    strcat(response, "----- END ENTRY -----\n");

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "DumpAccessControlEntry: dump failed %" CHIP_ERROR_FORMAT, err.Format());
        strcpy(response, "Error occurred");
    }
}

char * AppPlatformHandler(int argc, char ** argv)
{
    CHIP_ERROR err;

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
    else if (strcmp(argv[0], "print-app-access") == 0)
    {
        Access::AccessControl::EntryIterator iterator;
        SuccessOrExit(err = Access::GetAccessControl().Entries(GetDeviceCommissioner()->GetFabricIndex(), iterator));
        // clear entry
        strcpy(response, "");
        Access::AccessControl::Entry entry;
        while (iterator.Next(entry) == CHIP_NO_ERROR)
        {
            DumpAccessControlEntry(entry);
        }
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
        SuccessOrExit(err = pairApp(true, index));
        return response;
    }
    else
    {
        strcpy(response, "invalid argument");
        return response;
    }
    return response;
exit:
    ChipLogError(DeviceLayer, "Error: %" CHIP_ERROR_FORMAT, err.Format());
    strcpy(response, "Error occurred");
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
        jstring string = (jstring) (env->GetObjectArrayElement(stringArray, i));
        argv[i]        = (char *) env->GetStringUTFChars(string, 0);
    }

    // Store response to show it to the users
    char * buf = AppPlatformHandler(argc, argv);

    // Release UTF Chars
    for (int i = 0; i < argc; i++)
    {
        ChipLogProgress(DeviceLayer, " Value=%s ", argv[i]);
        jstring string = (jstring) (env->GetObjectArrayElement(stringArray, i));
        env->ReleaseStringUTFChars(string, argv[i]);
    }

    return env->NewStringUTF(buf);
}
