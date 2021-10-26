/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <transport/SecureSession.h>
#include <transport/SessionHandle.h>
#include <transport/SessionManager.h>

namespace chip {

using namespace Transport;

Session * SessionHandle::operator->() const {
    if (mSession.Is<VariantSecureSession>())
    {
        return AsSecureSession();
    }
    else if (mSession.Is<VariantUnauthenticatedSession>())
    {
        return &AsUnauthenticatedSession().Get();
    }

    VerifyOrDie(false);
    return nullptr;
}

Transport::SecureSession * SessionHandle::AsSecureSession() const
{
    const VariantSecureSession & session = mSession.Get<VariantSecureSession>();
    return session.mSessionManager.GetSecureSession(session.mLocalSessionId);
}

Transport::UnauthenticatedSessionHandle SessionHandle::AsUnauthenticatedSession() const
{
    return mSession.Get<VariantUnauthenticatedSession>().mUnauthenticatedSessionHandle;
}

} // namespace chip
