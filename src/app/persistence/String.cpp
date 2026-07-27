/*
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
#include <app/persistence/String.h>

namespace chip::app::Storage::Internal {

bool ShortString::SetContent(CharSpan value)
{

    ShortPascalString view = AsPascal();

    if (!view.SetValue(value))
    {
        view.SetValue(""_span);
        NullTerminate();
        return false;
    }
    NullTerminate();
    return true;
}

bool ShortStringInputAdapter::FinalizeRead(ByteSpan actuallyRead)
{
    if (!ShortPascalString::IsValid(actuallyRead))
    {
        mValue.SetContent(""_span);
        return false;
    }
    mValue.NullTerminate();
    return true;
}

} // namespace chip::app::Storage::Internal
