/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
struct GAutoPtrDeleter<GBytes>
{
    using deleter = GBytesDeleter;
};

template <>
struct GAutoPtrDeleter<GDBusConnection>
{
    using deleter = GObjectDeleter;
};

template <>
struct GAutoPtrDeleter<GError>
{
    using deleter = GErrorDeleter;
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
using GAutoPtr = std::unique_ptr<T, typename GAutoPtrDeleter<T>::deleter>;

} // namespace chip
