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

#include "MyUserPrompter-JNI.h"
#include <jni.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CHIPJNIError.h>
#include <lib/support/JniReferences.h>
#include <lib/support/JniTypeWrappers.h>

using namespace chip;

JNIMyUserPrompter::JNIMyUserPrompter(jobject provider)
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(Zcl, "Failed to GetEnvForCurrentThread for JNIMyUserPrompter"));

    VerifyOrReturn(mJNIMyUserPrompterObject.Init(provider) == CHIP_NO_ERROR,
                   ChipLogError(Zcl, "Failed to init mJNIMyUserPrompterObject"));

    jclass JNIMyUserPrompterClass = env->GetObjectClass(provider);
    VerifyOrReturn(JNIMyUserPrompterClass != nullptr, ChipLogError(Zcl, "Failed to get JNIMyUserPrompter Java class"));

    mPromptForCommissionOKPermissionMethod =
        env->GetMethodID(JNIMyUserPrompterClass, "promptForCommissionOkPermission", "(IILjava/lang/String;)V");
    if (mPromptForCommissionOKPermissionMethod == nullptr)
    {
        ChipLogError(Zcl, "Failed to access JNIMyUserPrompter 'promptForCommissionOkPermission' method");
        env->ExceptionClear();
    }

    mPromptForCommissionPincodeMethod =
        env->GetMethodID(JNIMyUserPrompterClass, "promptForCommissionPinCode", "(IILjava/lang/String;)V");
    if (mPromptForCommissionPincodeMethod == nullptr)
    {
        ChipLogError(Zcl, "Failed to access JNIMyUserPrompter 'promptForCommissionPinCode' method");
        env->ExceptionClear();
    }

    mPromptCommissioningSucceededMethod =
        env->GetMethodID(JNIMyUserPrompterClass, "promptCommissioningSucceeded", "(IILjava/lang/String;)V");
    if (mPromptCommissioningSucceededMethod == nullptr)
    {
        ChipLogError(Zcl, "Failed to access JNIMyUserPrompter 'promptCommissioningSucceeded' method");
        env->ExceptionClear();
    }

    mPromptCommissioningFailedMethod =
        env->GetMethodID(JNIMyUserPrompterClass, "promptCommissioningFailed", "(Ljava/lang/String;Ljava/lang/String;)V");
    if (mPromptCommissioningFailedMethod == nullptr)
    {
        ChipLogError(Zcl, "Failed to access JNIMyUserPrompter 'promptCommissioningFailed' method");
        env->ExceptionClear();
    }
}

/*
 *  Called to prompt the user for consent to allow the given commissioneeName/vendorId/productId to be commissioned.
 * For example "[commissioneeName] is requesting permission to cast to this TV, approve?"
 *
 * If user responds with OK then implementor calls UserPrompterResolver.OnPromptAccepted;
 * If user responds with Cancel then implementor calls calls UserPrompterResolver.OnPromptDeclined();
 *
 */
void JNIMyUserPrompter::PromptForCommissionOKPermission(uint16_t vendorId, uint16_t productId, const char * commissioneeName)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env   = JniReferences::GetInstance().GetEnvForCurrentThread();
    std::string stringCommissioneeName(commissioneeName);

    VerifyOrExit(mJNIMyUserPrompterObject.HasValidObjectRef(), err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mPromptForCommissionOKPermissionMethod != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(env != nullptr, err = CHIP_JNI_ERROR_NO_ENV);

    {
        UtfString jniCommissioneeName(env, stringCommissioneeName.data());
        env->ExceptionClear();
        env->CallVoidMethod(mJNIMyUserPrompterObject.ObjectRef(), mPromptForCommissionOKPermissionMethod,
                            static_cast<jint>(vendorId), static_cast<jint>(productId), jniCommissioneeName.jniValue());
        if (env->ExceptionCheck())
        {
            ChipLogError(DeviceLayer, "Java exception in PromptForCommissionOKPermission");
            env->ExceptionDescribe();
            env->ExceptionClear();
            err = CHIP_ERROR_INCORRECT_STATE;
            goto exit;
        }
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "PromptForCommissionOKPermission error: %s", err.AsString());
    }
}

/*
 *  Called to prompt the user to enter the setup pincode displayed by the given commissioneeName/vendorId/productId to be
 * commissioned. For example "Please enter pin displayed in casting app."
 *
 * If user responds with OK then implementor calls UserPrompterResolver.OnPinCodeEntered();
 * If user responds with Cancel then implementor calls UserPrompterResolver.OnPinCodeDeclined();
 *
 */
void JNIMyUserPrompter::PromptForCommissionPasscode(uint16_t vendorId, uint16_t productId, const char * commissioneeName,
                                                    uint16_t pairingHint, const char * pairingInstruction)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env   = JniReferences::GetInstance().GetEnvForCurrentThread();
    std::string stringCommissioneeName(commissioneeName);

    VerifyOrExit(mJNIMyUserPrompterObject.HasValidObjectRef(), err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mPromptForCommissionPincodeMethod != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(env != nullptr, err = CHIP_JNI_ERROR_NO_ENV);

    {
        UtfString jniCommissioneeName(env, stringCommissioneeName.data());
        env->ExceptionClear();
        env->CallVoidMethod(mJNIMyUserPrompterObject.ObjectRef(), mPromptForCommissionPincodeMethod, static_cast<jint>(vendorId),
                            static_cast<jint>(productId), jniCommissioneeName.jniValue());
        if (env->ExceptionCheck())
        {
            ChipLogError(Zcl, "Java exception in PromptForCommissionPincode");
            env->ExceptionDescribe();
            env->ExceptionClear();
            err = CHIP_ERROR_INCORRECT_STATE;
            goto exit;
        }
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "PromptForCommissionPincode error: %s", err.AsString());
    }
}

/**
 *   Called to when CancelCommissioning is received via UDC.
 * Indicates that commissioner can stop showing the passcode entry or display dialog.
 * For example, can show text such as "Commissioning cancelled by client" before hiding dialog.
 */
void JNIMyUserPrompter::HidePromptsOnCancel(uint16_t vendorId, uint16_t productId, const char * commissioneeName)
{
    // TODO
    ChipLogError(Zcl, "JNIMyUserPrompter::HidePromptsOnCancel Needs Implementation");
}

/**
 *   Return true if this UserPrompter displays QR code along with passcode
 * When PromptWithCommissionerPasscode is called during Commissioner Passcode functionality.
 */
bool JNIMyUserPrompter::DisplaysPasscodeAndQRCode()
{
    // TODO
    ChipLogError(Zcl, "JNIMyUserPrompter::DisplaysPasscodeAndQRCode Needs Implementation");
    return false;
}

/**
 *   Called to display the given setup passcode to the user,
 * for commissioning the given commissioneeName with the given vendorId and productId,
 * and provide instructions for where to enter it in the commissionee (when pairingHint and pairingInstruction are provided).
 * For example "Casting Passcode: [passcode]. For more instructions, click here."
 */
void JNIMyUserPrompter::PromptWithCommissionerPasscode(uint16_t vendorId, uint16_t productId, const char * commissioneeName,
                                                       uint32_t passcode, uint16_t pairingHint, const char * pairingInstruction)
{
    // TODO
    ChipLogError(Zcl, "JNIMyUserPrompter::PromptWithCommissionerPasscode Needs Implementation");
}

/**
 *   Called to alert the user that commissioning has begun."
 */
void JNIMyUserPrompter::PromptCommissioningStarted(uint16_t vendorId, uint16_t productId, const char * commissioneeName)
{
    // TODO
    ChipLogError(Zcl, "JNIMyUserPrompter::PromptCommissioningStarted Needs Implementation");
}

/*
 *   Called to notify the user that commissioning succeeded. It can be in form of UI Notification.
 */
void JNIMyUserPrompter::PromptCommissioningSucceeded(uint16_t vendorId, uint16_t productId, const char * commissioneeName)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env   = JniReferences::GetInstance().GetEnvForCurrentThread();
    std::string stringCommissioneeName(commissioneeName);

    VerifyOrExit(mJNIMyUserPrompterObject.HasValidObjectRef(), err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mPromptCommissioningSucceededMethod != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(env != nullptr, err = CHIP_JNI_ERROR_NO_ENV);

    {
        UtfString jniCommissioneeName(env, stringCommissioneeName.data());
        env->ExceptionClear();
        env->CallVoidMethod(mJNIMyUserPrompterObject.ObjectRef(), mPromptCommissioningSucceededMethod, static_cast<jint>(vendorId),
                            static_cast<jint>(productId), jniCommissioneeName.jniValue());

        if (env->ExceptionCheck())
        {
            ChipLogError(Zcl, "Java exception in PromptCommissioningSucceeded");
            env->ExceptionDescribe();
            env->ExceptionClear();
            err = CHIP_ERROR_INCORRECT_STATE;
            goto exit;
        }
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "PromptCommissioningSucceeded error: %s", err.AsString());
    }
}

/*
 *   Called to notify the user that commissioning failed. It can be in form of UI Notification.
 */
void JNIMyUserPrompter::PromptCommissioningFailed(const char * commissioneeName, CHIP_ERROR error)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env   = JniReferences::GetInstance().GetEnvForCurrentThread();
    std::string stringCommissioneeName(commissioneeName);

    VerifyOrExit(mJNIMyUserPrompterObject.HasValidObjectRef(), err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mPromptCommissioningFailedMethod != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(env != nullptr, err = CHIP_JNI_ERROR_NO_ENV);

    {
        std::string stringError(error.AsString());
        UtfString jniCommissioneeName(env, stringCommissioneeName.data());
        UtfString jniCommissioneeError(env, stringError.data());
        env->ExceptionClear();
        env->CallVoidMethod(mJNIMyUserPrompterObject.ObjectRef(), mPromptCommissioningFailedMethod, jniCommissioneeName.jniValue(),
                            jniCommissioneeError.jniValue());

        if (env->ExceptionCheck())
        {
            ChipLogError(Zcl, "Java exception in PromptCommissioningFailed");
            env->ExceptionDescribe();
            env->ExceptionClear();
            err = CHIP_ERROR_INCORRECT_STATE;
            goto exit;
        }
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "PromptCommissioningFailed error: %s", err.AsString());
    }
}
