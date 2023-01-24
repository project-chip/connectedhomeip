#pragma once

#include <app/AttributeAccessInterface.h>
#include <app/data-model/Decode.h>
#include <app/data-model/Encode.h>
#include <app/data-model/Nullable.h>

#include <iterator>
#include <vector>

namespace chip {
namespace app {
namespace DataModel {

// Container requirements.
// To encode and decode as a list the following must be present
// void clear();
// size_type size();
// T& back();
// void emplace_back();
// iterator_type begin();
// iterator_type end();
// iterator_type must conform to LegacyForwardIterator
// The contained type must be default-constructible
template <typename X>
class IsList
{
    typedef char yes;
    typedef long no;

    template <typename U>
    static yes test(decltype(&U::size));
    template <typename U>
    static no test(...);

public:
    static constexpr bool value = sizeof(test<X>(0)) == sizeof(yes);
};

template <typename X>
struct IsOptionalOrNullable
{
    static constexpr bool value = false;
};
template <typename X>
struct IsOptionalOrNullable<Optional<X>>
{
    static constexpr bool value = true;
};
template <typename X>
struct IsOptionalOrNullable<Nullable<X>>
{
    static constexpr bool value = true;
};

static_assert(IsList<std::vector<unsigned char>>::value, "Vector of chars must be a list");

template <typename X,
          std::enable_if_t<!IsOptionalOrNullable<std::decay_t<X>>::value && !IsList<std::decay_t<X>>::value, bool> = true>
CHIP_ERROR Encode(const ConcreteReadAttributePath &, AttributeValueEncoder & aEncoder, const X & x)
{
    return aEncoder.Encode(x);
}

/*
 * @brief
 * Lists that are string-like should be encoded as char/byte spans.
 * Encode using a given AttributeValueEncoder.
 */
template <
    typename X,
    std::enable_if_t<IsList<std::decay_t<X>>::value && sizeof(std::decay_t<typename X::pointer>) == sizeof(char), bool> = true>
CHIP_ERROR Encode(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder, const X & x)
{
    return aEncoder.Encode(Span<std::decay_t<typename X::pointer>>(x.data(), x.size()));
}

/*
 * @brief
 * Lists that are string-like should be encoded as char/byte spans.
 * Encode using a given TLVWriter.
 */
template <
    typename X,
    std::enable_if_t<IsList<std::decay_t<X>>::value && sizeof(std::decay_t<typename X::pointer>) == sizeof(char), bool> = true>
CHIP_ERROR Encode(const ConcreteReadAttributePath & aPath, TLV::TLVWriter & writer, TLV::Tag tag, const X & x)
{
    return writer.Put(tag, Span<std::decay_t<typename X::pointer>>(x.data(), x.size()));
}

/*
 * @brief
 *
 * If an item is requested from a list, encode just that single item, or the entire list otherwise.
 *
 * Encodes items using a given AttributeValueEncoder.
 *
 * The object must satisfy the following constraints
 * size() must return an integer
 * begin() must return a type conforming to LegacyRandomAccessIterator
 *
 * This is const X& instead of X&& because it is "more specialized" and so this overload will
 * be chosen if possible.
 */
template <
    typename X,
    std::enable_if_t<IsList<std::decay_t<X>>::value && (sizeof(std::decay_t<typename X::pointer>) > sizeof(char)), bool> = true>
CHIP_ERROR Encode(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder, const X & x)
{
    if (aPath.mListIndex.HasValue())
    {
        uint16_t index = aPath.mListIndex.Value();
        if (index >= x.size())
            return CHIP_ERROR_INVALID_ARGUMENT;

        auto it = x.begin();
        std::advance(it, index);
        return aEncoder.Encode(*it);
    }
    return aEncoder.EncodeList([x](const auto & encoder) {
        CHIP_ERROR err = CHIP_NO_ERROR;
        for (auto & v : x)
        {
            err = encoder.Encode(v);
            if (err != CHIP_NO_ERROR)
                break;
        }
        return err;
    });
}

/*
 * @brief
 *
 * If an item is requested from a list, encode just that single item, or the entire list otherwise.
 *
 * Encodes items using a given TLVWriter.
 *
 * The object must satisfy the following constraints
 * size() must return an integer
 * begin() must return a type conforming to LegacyRandomAccessIterator
 *
 * This is const X& instead of X&& because it is "more specialized" and so this overload will
 * be chosen if possible.
 */
template <
    typename X,
    std::enable_if_t<IsList<std::decay_t<X>>::value && (sizeof(std::decay_t<typename X::pointer>) > sizeof(char)), bool> = true>
CHIP_ERROR Encode(const ConcreteReadAttributePath & aPath, TLV::TLVWriter & writer, TLV::Tag tag, const X & x)
{
    if (aPath.mListIndex.HasValue())
    {
        uint16_t index = aPath.mListIndex.Value();
        if (index >= x.size())
            return CHIP_ERROR_INVALID_ARGUMENT;

        auto it = x.begin();
        std::advance(it, index);
        return Encode(writer, tag, *it);
    }
    TLV::TLVType type;

    ReturnErrorOnFailure(writer.StartContainer(tag, TLV::kTLVType_Array, type));
    for (auto & item : x)
    {
        ReturnErrorOnFailure(Encode(writer, tag, item));
    }
    ReturnErrorOnFailure(writer.EndContainer(type));
    return CHIP_NO_ERROR;
}

/*
 * @brief
 * Set of overloaded encode methods that can be called from AttributeAccessInterface::Read
 */
template <typename X>
CHIP_ERROR Encode(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder, const Optional<X> & x)
{
    if (x.HasValue())
    {
        return Encode(aPath, aEncoder, x.Value());
    }
    // If no value, just do nothing.
    return CHIP_NO_ERROR;
}

/*
 * @brief
 *
 * Encodes a nullable value.
 */
template <typename X>
CHIP_ERROR Encode(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder, const Nullable<X> & x)
{
    if (x.IsNull())
    {
        return aEncoder.EncodeNull();
    }

    // Allow sending invalid values for nullables when
    // CONFIG_BUILD_FOR_HOST_UNIT_TEST is true, so we can test how the other side
    // responds.
#if !CONFIG_BUILD_FOR_HOST_UNIT_TEST
    if (!x.HasValidValue())
    {
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);
    }
#endif // !CONFIG_BUILD_FOR_HOST_UNIT_TEST

    // The -Wmaybe-uninitialized warning gets confused about the fact
    // that x.mValue is always initialized if x.IsNull() is not
    // true, so suppress it for our access to x.Value().
#pragma GCC diagnostic push
#if !defined(__clang__)
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif // !defined(__clang__)
    return Encode(aPath, aEncoder, x.Value());
#pragma GCC diagnostic pop
}

template <typename X,
          std::enable_if_t<!IsOptionalOrNullable<std::decay_t<X>>::value && !IsList<std::decay_t<X>>::value, bool> = true>
CHIP_ERROR Decode(const ConcreteDataAttributePath &, AttributeValueDecoder & aDecoder, X & x)
{
    return aDecoder.Decode(x);
}

/*
 * @brief
 * Lists that are string-like should be decoded as char/byte spans.
 */
template <
    typename X,
    std::enable_if_t<IsList<std::decay_t<X>>::value && sizeof(std::decay_t<typename X::pointer>) == sizeof(char), bool> = true>
CHIP_ERROR Decode(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder, X & x)
{
    Span<std::decay_t<typename X::pointer>> span;
    CHIP_ERROR err = aDecoder.Decode(span);
    if (err == CHIP_NO_ERROR)
    {
        x = X(span.data(), span.size());
    }
    return err;
}

template <
    typename X,
    std::enable_if_t<IsList<std::decay_t<X>>::value && (sizeof(std::decay_t<typename X::pointer>) > sizeof(char)), bool> = true>
CHIP_ERROR Decode(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder, X & x)
{
    switch (aPath.mListOp)
    {
    case ConcreteDataAttributePath::ListOperation::DeleteItem:
        if (aPath.mListIndex >= x.size())
        {
            return CHIP_ERROR_INVALID_LIST_LENGTH;
        }
        else
        {
            auto it = x.begin();
            std::advance(it, aPath.mListIndex);
            x.erase(it);
            return CHIP_NO_ERROR;
        }

    case ConcreteDataAttributePath::ListOperation::ReplaceItem:
        if (aPath.mListIndex >= x.size())
        {
            return CHIP_ERROR_INVALID_LIST_LENGTH;
        }
        else
        {
            auto it = x.begin();
            std::advance(it, aPath.mListIndex);
            return aDecoder.Decode(*it);
        }

    case ConcreteDataAttributePath::ListOperation::ReplaceAll:
        x.clear();
        // fallthrough
    default:
        x.emplace_back();
        return aDecoder.Decode(x.back());
    }
}

/*
 * @brief
 *
 * Decodes an optional value (struct field, command field, event field).
 */
template <typename X>
CHIP_ERROR Decode(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder, Optional<X> & x)
{
    // If we are calling this, it means we found the right tag, so just decode
    // the item.
    return Decode(aPath, aDecoder, x.HasValue() ? x.Value() : x.Emplace());
}

/*
 * @brief
 *
 * Decodes a nullable value.
 */
template <typename X>
CHIP_ERROR Decode(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder, Nullable<X> & x)
{
    if (aDecoder.WillDecodeNull())
    {
        x.SetNull();
        return CHIP_NO_ERROR;
    }

    // We have a value; decode it.
    ReturnErrorOnFailure(Decode(aPath, aDecoder, x.SetNonNull()));
    if (!x.HasValidValue())
    {
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);
    }
    return CHIP_NO_ERROR;
}

} // namespace DataModel
} // namespace app
} // namespace chip
