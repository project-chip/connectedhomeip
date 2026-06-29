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
#include <gio/gunixfdlist.h>
#include <glib.h>

namespace chip {

/**
 *  @brief
 *    RAII adapter that lets a `std::unique_ptr` receive ownership of
 *    a raw pointer written through a C-style `T **` output parameter.
 *
 *  Many GLib (and other C) APIs return a freshly-allocated,
 *  caller-owned object by writing it through a `T **` out-parameter
 *  (for example, `GError**` on the various `..._call_sync()`
 *  functions).  `std::unique_ptr` exposes no writable `T **` of its
 *  own, so this adapter supplies one: @ref operator&() hands the C
 *  function the address of an internal raw-pointer slot, and the
 *  destructor transfers whatever was stored there into the owning
 *  smart pointer via `reset()`.
 *
 *  @par Deferred Commit
 *    The captured pointer is moved into `target` only when the
 *    receiver is destroyed, **not** at the moment the C call
 *    returns. In the idiomatic form, the receiver is an unnamed
 *    temporary (see @ref GAutoPtr::GetReceiver), so it dies at the
 *    end of the enclosing full-expression and `target` is populated
 *    before the next statement (including before the body of an `if`
 *    whose condition contains the call).
 *
 *  @warning
 *    Do not bind the receiver to a named variable. A named receiver
 *    lives until the end of its scope, so the commit to `target` is
 *    deferred that long and `target` reads as empty in the interim:
 *
 *    @code
 *    GAutoPtr<GError> err;
 *    auto receiver = err.GetReceiver();        // lives to end of scope
 *    some_call(..., &receiver);
 *    if (err) { ... }                          // BUG: not committed yet; err is empty
 *    @endcode
 *
 *    Use the temporary form, which commits at the `;` :
 *
 *    @code
 *    GAutoPtr<GError> err;
 *    if (!some_call(..., &err.GetReceiver()))  // temporary destroyed at end of condition
 *    {
 *        ChipLogError(..., err->message);      // OK: err is populated here
 *    }
 *    @endcode
 *
 *  @pre
 *    The C out-parameter must transfer ownership of the written
 *    pointer to the caller, because the bound `target` releases
 *    whatever is committed using its deleter. Do **not** feed a
 *    receiver a *borrowed* pointer: for example, the no-copy,
 *    "&"-prefixed `g_variant_get()` extractions (`&s`, `&o`, `&g`,
 *    byte-array forms) return pointers into the variant's own storage
 *    and must not be wrapped (doing so double-frees). Use a receiver
 *    only where the API hands back a caller-owned object.
 *
 *  @note
 *    Multiple receivers may appear in one call (for example an
 *    out-pointer alongside an out-`GError`). Each is an independent
 *    temporary that commits to its own `target` at the end of the
 *    full-expression; their relative destruction order is unspecified
 *    but immaterial, since the targets are distinct and neither
 *    commit affects the other.
 *
 *  @note
 *    `target` is captured by reference and must outlive the
 *    receiver. The `reset()` performed at destruction also releases
 *    any object the target previously held, so reusing a non-empty
 *    target is safe; if the C call wrote nothing the slot stays @c
 *    nullptr and the target is simply emptied.
 *
 *  @note
 *    @ref operator&() deliberately overrides `operator&` to yield `T
 *    **` rather than the receiver's own address; obtain the latter
 *    with `std::addressof` if ever required.
 *
 *  @tparam T
 *    The pointee type; the managed/written pointer is `T *`.
 *
 *  @tparam Deleter
 *    The deleter type of the target `std::unique_ptr`.
 *
 *  @sa GAutoPtr::GetReceiver
 *  @sa MakeUniquePointerReceiver
 *
 */
template <typename T, typename Deleter>
class UniquePointerReceiver
{
public:
    /**
     *  @brief
     *    Construct a receiver bound to `target`.
     *
     *  @param[in,out] target
     *    The smart pointer that will receive ownership of the
     *    captured raw pointer when this receiver is
     *    destroyed. Captured by reference; must outlive the receiver.
     */
    UniquePointerReceiver(std::unique_ptr<T, Deleter> & target) : mTarget(target) {}

    /**
     *  @brief
     *    Commit the captured raw pointer to the bound target.
     *
     *  Transfers the internal slot into the target via `reset()`,
     *  releasing any object the target previously held. If nothing
     *  was written the slot is `nullptr` and the target is emptied.
     */
    ~UniquePointerReceiver() { mTarget.reset(mValue); }

    /**
     *  @brief
     *    Access the internal raw-pointer slot as a mutable lvalue.
     *
     *  @returns
     *    A reference to the captured `T *`, allowing it to be read or
     *    assigned before the receiver is destroyed and the value
     *    committed to the target.
     */
    T *& Get() { return mValue; }

    /**
     *  @brief
     *    Yield the address of the internal raw-pointer slot for a C
     *    `T **` output parameter.
     *
     *  @returns
     *    `T **` pointing at the internal slot. Pass this to a C API
     *    that writes a newly-allocated, caller-owned `T *` through a
     *    `T **` out-parameter; the written pointer is committed to
     *    the target when the receiver is destroyed.
     */
    T ** operator&() { return &mValue; }

private:
    std::unique_ptr<T, Deleter> & mTarget;
    T * mValue = nullptr;
};

/**
 *  @brief
 *    Construct a @ref UniquePointerReceiver, deducing its template
 *    arguments from `target`.
 *
 *  @tparam T
 *    Deduced pointee type of `target`.
 *
 *  @tparam Deleter
 *    Deduced deleter type of `target`.
 *
 *  @param[in,out] target
 *    The smart pointer to receive ownership; captured by reference by
 *    the constructed receiver.
 *
 *  @returns
 *    A @ref UniquePointerReceiver bound to `target`.
 *
 *  @sa GAutoPtr::GetReceiver
 *
 */
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

struct GHashTableDeleter
{
    void operator()(GHashTable * hash) { g_hash_table_destroy(hash); }
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

struct GStrvDeleter
{
    void operator()(gchar ** object) { g_strfreev(object); }
};

/**
 *  @brief
 *    Trait mapping a GLib-managed pointee type `T` to the functor
 *    that releases it.
 *
 *  The primary template is intentionally empty: instantiating @c
 *  GAutoPtr with an unsupported `T` fails to compile because there is
 *  no nested `deleter` to name. Add support by providing an explicit
 *  specialization.
 *
 *  @note
 *    `T` is the *pointee* (the type inside the `GAutoPtr<>` angle
 *    brackets), **not** the pointer that is stored and freed. The
 *    stored pointer is `T *`. For the string-array specializations
 *    this means the angle-bracket type is the *element* while the
 *    object owned is the *array* (this is a potential source of code
 *    reviewer confusion).
 *
 *  @tparam T
 *    The pointee type managed by `GAutoPtr<T>`.
 *
 *  @sa GAutoPtr
 *
 */
template <typename T>
struct GAutoPtrDeleter
{
};

/**
 *  @brief
 *    Deleter trait for a single, caller-owned C string.
 *

 *  `GAutoPtr<char>` manages a `char *` pointing at one
 *  null-terminated string from the GLib allocator (for example, @c
 *  g_strdup(), `g_strdup_printf()`, or a generated `*_dup_*()`
 *  accessor). Released with a single `g_free()`.
 *
 *  Stored pointer: `char *`. Released with: `g_free()`.
 *
 *  @note
 *    Scalar case only. Do not conflate with the string-*array* cases
 *    @ref GAutoPtrDeleter<gchar *> (owned vector, `g_strfreev()`) and
 *    @ref GAutoPtrDeleter<const char *> (borrowed vector,
 *    container-only `g_free()`).
 *
 *  @sa GFree
 *
 */
template <>
struct GAutoPtrDeleter<char>
{
    using deleter = GFree;
};

/**
 *  @brief
 *    Deleter trait for a *borrowed* null-terminated string array: the
 *    container is caller-owned, the elements are not.
 *
 *  `GAutoPtr<const char *>` manages a `const char **` from APIs that
 *  return a shallow copy of a string array while retaining ownership
 *  of the strings. Canonical producer: `g_variant_get_strv()`, whose
 *  array must be released with `g_free()` while its strings stay
 *  owned by the `GVariant` and must **not** be freed.
 *
 *  Stored pointer: `const char **` (== `const gchar **`). Released
 *  with: `g_free()` on the array only.
 *
 *  @warning
 *    Do **not** change this to `g_strfreev()` and do **not** drop the
 *    `const`. `g_strfreev()` would free strings this code does not
 *    own. The `const` is load-bearing: because `const gchar **` does
 *    not convert to `gchar **` (and vice versa), it is a compile
 *    error to route a borrowed array into the owning @ref
 *    GAutoPtrDeleter<gchar *> deleter, or an owned array into this
 *    one. That mismatch is caught at build time precisely because the
 *    two specializations differ in `const`.
 *
 *  @note
 *    `gchar` is `ypedef`'d to `char`, so `GAutoPtr<const gchar *>`
 *    and `GAutoPtr<const char *>` name the same instantiation and
 *    select this specialization; the call-site spelling difference is
 *    cosmetic.
 *
 *  @sa GFree
 *  @sa GAutoPtrDeleter<gchar *>
 *
 */
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

/**
 *  @brief
 *    Deleter trait for a fully caller-owned null-terminated string
 *    array (a GLib "strv").
 *
 *  `GAutoPtr<gchar *>` manages a `gchar **` from deep-copy APIs that
 *  transfer ownership of both the array and every string: @c
 *  g_strsplit(), `g_strdupv()`, `g_variant_dup_strv()`. The whole
 *  vector is released with `g_strfreev()`.
 *
 *  Stored pointer: `gchar **` (== `char **`). Released with:
 *  `g_strfreev()`.
 *
 *  @note
 *    Choose between this and @ref GAutoPtrDeleter<const char *> by
 *    the const-ness of the GLib return type: `gchar **` (for example,
 *    `g_variant_dup_strv`) selects this trait; `const gchar **` (for
 *    example, `g_variant_get_strv`) selects the const one. The
 *    pointer conversion rules enforce that choice at compile time.
 *
 *  @sa GStrvDeleter
 *  @sa GAutoPtrDeleter<const char *>
 *
 */
template <>
struct GAutoPtrDeleter<gchar *>
{
    using deleter = GStrvDeleter;
};

template <>
struct GAutoPtrDeleter<GHashTable>
{
    using deleter = GHashTableDeleter;
};

template <>
struct GAutoPtrDeleter<GIOChannel>
{
    using deleter = GIOChannelDeleter;
};

template <>
struct GAutoPtrDeleter<GObject>
{
    using deleter = GObjectDeleter;
};

template <>
struct GAutoPtrDeleter<GSource>
{
    using deleter = GSourceDeleter;
};

template <>
struct GAutoPtrDeleter<GUnixFDList>
{
    using deleter = GObjectDeleter;
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

/**
 *  @brief
 *    A `std::unique_ptr` that supplies the correct GLib deleter for
 *    `T` via @ref GAutoPtrDeleter.
 *
 *  @tparam T
 *    The pointee. The *managed* pointer is `T *`, so for the
 *    string-array specializations the angle-bracket type is the
 *    element while the owned object is the array:
 *
 *      - `GAutoPtr<char>`         owns `char *`        (one string; `g_free`)
 *      - `GAutoPtr<gchar *>`      owns `gchar **`      (owned strv; `g_strfreev`)
 *      - `GAutoPtr<const char *>` owns `const char **` (borrowed strv; `g_free` on the container only)
 *
 *  @sa GAutoPtrDeleter
 *  @sa UniquePointerReceiver
 *
 */
template <typename T>
class GAutoPtr : public std::unique_ptr<T, typename GAutoPtrDeleter<T>::deleter>
{
public:
    using deleter = typename GAutoPtrDeleter<T>::deleter;
    using std::unique_ptr<T, deleter>::unique_ptr;

    /**
     *  @brief
     *    Obtain a @ref UniquePointerReceiver bound to this smart
     *    pointer.
     *
     *  Intended to be used as an unnamed temporary so that ownership
     *  of a pointer written through a C `T **` out-parameter is
     *  committed to `*this` at the end of the enclosing
     *  full-expression:
     *
     *  @code
     *  GAutoPtr<GError> err;
     *  g_dbus_proxy_call_sync(proxy, ..., &err.GetReceiver());
     *  @endcode
     *
     *  @returns
     *    A receiver bound to `*this` by reference.
     *
     *  @warning
     *    Do not store the result in a named variable; see @ref
     *    UniquePointerReceiver for the deferred-commit hazard.
     *
     *  @sa UniquePointerReceiver
     *
     */
    UniquePointerReceiver<T, deleter> GetReceiver() { return MakeUniquePointerReceiver(*this); }
};

} // namespace chip
