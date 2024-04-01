/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <memory>

#include <gio/gio.h>
#include <glib.h>

namespace chip {

template <typename T, typename Deleter>
class UniquePointerReceiver
{
public:
    UniquePointerReceiver(std::unique_ptr<T, Deleter> & target) : mTarget(target) {}
    ~UniquePointerReceiver() { mTarget.reset(mValue); }

    T *& Get() { return mValue; }
    T ** operator&() { return &mValue; }

private:
    std::unique_ptr<T, Deleter> & mTarget;
    T * mValue = nullptr;
};

template <typename T, typename Deleter>
UniquePointerReceiver<T, Deleter> MakeUniquePointerReceiver(std::unique_ptr<T, Deleter> & target)
{
    return UniquePointerReceiver<T, Deleter>(target);
}

struct GFree
{
    void operator()(gpointer object) { g_free(object); }
};

struct GObjectDeleter
{
    void operator()(gpointer object) { g_object_unref(object); }
};

struct GErrorDeleter
{
    void operator()(GError * object) { g_error_free(object); }
};

struct GIOChannelDeleter
{
    void operator()(GIOChannel * object) { g_io_channel_unref(object); }
};

struct GSourceDeleter
{
    void operator()(GSource * object) { g_source_unref(object); }
};

struct GVariantDeleter
{
    void operator()(GVariant * object) { g_variant_unref(object); }
};

struct GVariantIterDeleter
{
    void operator()(GVariantIter * object) { g_variant_iter_free(object); }
};

struct GBytesDeleter
{
    void operator()(GBytes * object) { g_bytes_unref(object); }
};

template <typename T>
struct GAutoPtrDeleter
{
};

template <>
struct GAutoPtrDeleter<char>
{
    using deleter = GFree;
};

template <>
struct GAutoPtrDeleter<const char *>
{
    using deleter = GFree;
};

template <>
struct GAutoPtrDeleter<GBytes>
{
    using deleter = GBytesDeleter;
};

template <>
struct GAutoPtrDeleter<GCancellable>
{
    using deleter = GObjectDeleter;
};

template <>
struct GAutoPtrDeleter<GDBusConnection>
{
    using deleter = GObjectDeleter;
};

template <>
struct GAutoPtrDeleter<GDBusObjectManager>
{
    using deleter = GObjectDeleter;
};

template <>
struct GAutoPtrDeleter<GDBusObjectManagerServer>
{
    using deleter = GObjectDeleter;
};

template <>
struct GAutoPtrDeleter<GError>
{
    using deleter = GErrorDeleter;
};

template <>
struct GAutoPtrDeleter<GIOChannel>
{
    using deleter = GIOChannelDeleter;
};

template <>
struct GAutoPtrDeleter<GSource>
{
    using deleter = GSourceDeleter;
};

template <>
struct GAutoPtrDeleter<GVariant>
{
    using deleter = GVariantDeleter;
};

template <>
struct GAutoPtrDeleter<GVariantIter>
{
    using deleter = GVariantIterDeleter;
};

template <typename T>
class GAutoPtr : public std::unique_ptr<T, typename GAutoPtrDeleter<T>::deleter>
{
public:
    using deleter = typename GAutoPtrDeleter<T>::deleter;
    using std::unique_ptr<T, deleter>::unique_ptr;

    // Convenience method to get a UniquePointerReceiver for this object.
    UniquePointerReceiver<T, deleter> GetReceiver() { return MakeUniquePointerReceiver(*this); }
};

} // namespace chip
