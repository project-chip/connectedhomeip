/**
 *
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
 *
 */

/**
 * \file
 * \brief Contains an implementation of C++17 optional (n3793).
 *
 * \sa https://github.com/akrzemi1/Optional
 * \sa http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2014/n3848.html
 */

#ifndef NETWORK_OPTIONAL_INC
#define NETWORK_OPTIONAL_INC

#include <algorithm>
#include <cstdlib>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>

#if !defined(DOXYGEN_SHOULD_SKIP_THIS)
#ifdef NDEBUG
#define NETWORK_ASSERTED_EXPRESSION(CHECK, EXPR) (EXPR)
#else
#define NETWORK_ASSERTED_EXPRESSION(CHECK, EXPR) ((CHECK) ? (EXPR) : (fail(#CHECK, __FILE__, __LINE__), (EXPR)))
inline void fail(const char *, const char *, unsigned) {}
#endif // NDEBUG
#endif // !defined(DOXYGEN_SHOULD_SKIP_THIS)

namespace network {
/**
 * \ingroup optional
 * \class nullopt_t optional.hpp network/uri.hpp
 * \brief Disengaged state indicator.
 * \sa optional
 */
struct nullopt_t
{
#if !defined(DOXYGEN_SHOULD_SKIP_THIS)
    struct init
    {
    };
    constexpr nullopt_t(init) {}
#endif // !defined(DOXYGEN_SHOULD_SKIP_THIS)
};

/**
 * \ingroup optional
 * \brief Used to indicate a *disengaged* state for optional objects.
 */
constexpr nullopt_t nullopt{ nullopt_t::init{} };

/**
 * \ingroup optional
 * \class bad_optional_access optional.hpp network/uri.hpp
 * \brief Exception thrown when the value member function is called when the
 *        optional object is disengaged.
 */
class bad_optional_access : public std::logic_error
{
public:
    /**
     * \brief Constructor.
     * \param what_arg The error message.
     */
    explicit bad_optional_access(const std::string & what_arg) : std::logic_error(what_arg) {}

    /**
     * \brief Constructor.
     * \param what_arg The error message.
     */
    explicit bad_optional_access(const char * what_arg) : std::logic_error(what_arg) {}
};

#if !defined(DOXYGEN_SHOULD_SKIP_THIS)
namespace details {
struct dummy_t
{
};

template <class T>
union trivially_destructible_optional_storage
{
    static_assert(std::is_trivially_destructible<T>::value, "");

    dummy_t dummy_;
    T value_;

    constexpr trivially_destructible_optional_storage() : dummy_{} {}

    constexpr trivially_destructible_optional_storage(const T & v) : value_{ v } {}

    ~trivially_destructible_optional_storage() = default;
};

template <class T>
union optional_storage
{
    dummy_t dummy_;
    T value_;

    constexpr optional_storage() : dummy_{} {}

    constexpr optional_storage(const T & v) : value_{ v } {}

    ~optional_storage() {}
};

template <class T>
class trivially_destructible_optional_base
{
public:
    typedef T value_type;

    constexpr trivially_destructible_optional_base() noexcept : init_(false), storage_{} {}

    constexpr trivially_destructible_optional_base(const T & v) : init_(true), storage_{ v } {}

    constexpr trivially_destructible_optional_base(T && v) : init_(true), storage_{ std::move(v) } {}

    ~trivially_destructible_optional_base() = default;

protected:
    bool init_;
    optional_storage<T> storage_;
};

template <class T>
class optional_base
{
public:
    typedef T value_type;

    constexpr optional_base() noexcept : init_(false), storage_{} {}

    constexpr optional_base(const T & v) : init_(true), storage_{ v } {}

    constexpr optional_base(T && v) : init_(true), storage_{ std::move(v) } {}

    ~optional_base()
    {
        if (init_)
        {
            storage_.value_.T::~T();
        }
    }

protected:
    bool init_;
    optional_storage<T> storage_;
};
} // namespace details
#endif // !defined(DOXYGEN_SHOULD_SKIP_THIS)

/**
 * \ingroup optional
 * \class optional optional.hpp network/uri.hpp
 * \brief An implementation of C++17 optional (n3793)
 */
#if !defined(DOXYGEN_SHOULD_SKIP_THIS)
template <class T>
using optional_base = typename std::conditional<std::is_trivially_destructible<T>::value,
                                                details::trivially_destructible_optional_base<T>, details::optional_base<T>>::type;
#endif // !defined(DOXYGEN_SHOULD_SKIP_THIS)

template <class T>
#if !defined(DOXYGEN_SHOULD_SKIP_THIS)
class optional : optional_base<T>
{
#else
class optional
{
#endif // !defined(DOXYGEN_SHOULD_SKIP_THIS)
    typedef optional_base<T> base_type;

public:
    /**
     * \brief Optional value type.
     */
    typedef T value_type;

    /**
     * \brief Constructor.
     * \post *disengaged*.
     */
    constexpr optional() : optional_base<T>() {}

    /**
     * \brief Constructor.
     * \post *disengaged*.
     */
    constexpr optional(nullopt_t) noexcept : optional_base<T>() {}

    /**
     * \brief Copy constructor.
     * \param other The other optional object.
     */
    optional(const optional & other) : optional_base<T>()
    {
        if (other)
        {
            ::new (static_cast<void *>(ptr())) T(*other);
            base_type::init_ = true;
        }
    }

    /**
     * \brief Move constructor.
     * \param other The other optional object.
     */
    optional(optional && other) noexcept
    {
        if (other)
        {
            ::new (static_cast<void *>(ptr())) T(std::move(other.storage_.value_));
            base_type::init_ = true;
        }
    }

    /**
     * \brief Constructor.
     * \param value The value with which to initialize the optional object.
     * \post *engaged*
     */
    constexpr optional(const T & value) : optional_base<T>(value) {}

    /**
     * \brief Constructor.
     * \param value The value with which to initialize the optional object.
     * \post *engaged*
     */
    constexpr optional(T && value) : optional_base<T>(std::move(value)) {}

    /**
     * \brief Assignment operator.
     * \post *disengaged*.
     * \returns \c *this.
     */
    optional & operator=(nullopt_t) noexcept
    {
        if (base_type::init_)
        {
            ptr()->T::~T();
        }
        base_type::init_ = false;
        return *this;
    }

    /**
     * \brief Copy assignment operator.
     * \param other The other optional object.
     * \returns \c *this.
     */
    optional & operator=(const optional & other)
    {
        if (bool(*this) && !other)
        {
            ptr()->T::~T();
            base_type::init_ = false;
        }
        else if (!(*this) && bool(other))
        {
            ::new (static_cast<void *>(ptr())) T(*other);
            base_type::init_ = true;
        }
        else if (bool(*this) && bool(other))
        {
            base_type::storage_.value_ = *other;
        }
        return *this;
    }

    /**
     * \brief Move assignment operator.
     * \param other The other optional object.
     * \returns \c *this.
     */
    optional & operator=(optional && other) noexcept
    {
        if (bool(*this) && !other)
        {
            ptr()->T::~T();
            base_type::init_ = false;
        }
        else if (!(*this) && bool(other))
        {
            ::new (static_cast<void *>(ptr())) T(std::move(*other));
            base_type::init_ = true;
        }
        else if (bool(*this) && bool(other))
        {
            base_type::storage_.value_ = std::move(*other);
        }
        return *this;
    }

    /**
     * \brief Destructor.
     */
    ~optional() = default;

    /**
     * \brief Swap function.
     * \param other The other optional object.
     */
    void swap(optional & other) noexcept
    {
        if (bool(*this) && !other)
        {
            ::new (static_cast<void *>(other.ptr())) T(std::move(**this));
            ptr()->T::~T();
            std::swap(base_type::init_, other.base_type::init_);
        }
        else if (!(*this) && bool(other))
        {
            ::new (static_cast<void *>(ptr())) T(std::move(*other));
            other.ptr()->T::~T();
            std::swap(base_type::init_, other.init_);
        }
        else if (bool(*this) && bool(other))
        {
            std::swap(**this, *other);
        }
    }

    /**
     * \brief Observer.
     * \pre *engaged*
     * \returns The underlying optional value.
     */
    constexpr T const * operator->() const { return NETWORK_ASSERTED_EXPRESSION(bool(*this), ptr()); }

    /**
     * \brief Observer.
     * \pre *engaged*
     * \returns The underlying optional value.
     */
    T * operator->() { return NETWORK_ASSERTED_EXPRESSION(bool(*this), ptr()); }

    /**
     * \brief Observer.
     * \pre *engaged*
     * \returns The underlying optional value.
     */
    constexpr T const & operator*() const { return NETWORK_ASSERTED_EXPRESSION(bool(*this), base_type::storage_.value_); }

    /**
     * \brief Observer.
     * \pre *engaged*
     * \returns The underlying optional value.
     */
    T & operator*() { return NETWORK_ASSERTED_EXPRESSION(bool(*this), base_type::storage_.value_); }

    /**
     * \brief Operator bool overloads.
     * \returns \c true if the optional is *engaged*, \c false if it is
     * *disengaged*.
     */
    constexpr explicit operator bool() const noexcept { return base_type::init_; }

    /**
     * \returns The underlying optional value, if \c bool(*this).
     * \throws A bad_optional_access if \c !*this.
     */
    constexpr T const & value() const
    {
        return *this ? base_type::storage_.value_
                     : (abort(), base_type::storage_.value_);
    }
    /**
     * \returns The underlying optional value, if \c bool(*this).
     * \throws A bad_optional_access if \c !*this.
     */
    T & value() { return *this ? base_type::storage_.value_ : (abort(), base_type::storage_.value_); }

    /**
     * \returns <tt>bool(*this) ? **this :
     * static_cast<T>(std::forward<U>(v))</tt>. \pre \c
     * <tt>std::is_copy_constructible<T>::value</tt> is \c true and
     * <tt>std::is_convertible<U&&, T>::value</tt> is \c true.
     */
    template <class U>
    T value_or(U && other) const &
    {
        static_assert(std::is_copy_constructible<value_type>::value, "Must be copy constructible.");
        static_assert(std::is_convertible<U, value_type>::value, "U must be convertible to T.");
        return bool(*this) ? **this : static_cast<T>(std::forward<U>(other));
    }

    /**
     * \returns <tt>bool(*this) ? std::move(**this) :
     * static_cast<T>(std::forward<U>(v))</tt>. \pre
     * <tt>std::is_move_constructible<T>::value</tt> is \c true and
     * <tt>std::is_convertible<U&&, T>::value</tt> is \c true.
     */
    template <class U>
    T value_or(U && other) &&
    {
        static_assert(std::is_copy_constructible<value_type>::value, "Must be copy constructible.");
        static_assert(std::is_convertible<U, value_type>::value, "U must be convertible to T.");
        return bool(*this) ? std::move(**this) : static_cast<T>(std::forward<U>(other));
    }

private:
    T * ptr() { return std::addressof(base_type::storage_.value_); }
};

/**
 * \brief Equality operator.
 */
template <class T>
bool operator==(const optional<T> & lhs, const optional<T> & rhs)
{
    if (bool(lhs) != bool(rhs))
    {
        return false;
    }
    else if (!bool(lhs))
    {
        return true;
    }
    else
    {
        return *lhs == *rhs;
    }
}

/**
 * \brief Inequality operator.
 */
template <class T>
bool operator!=(const optional<T> & lhs, const optional<T> & rhs)
{
    return !(lhs == rhs);
}

/**
 * \brief Comparison operator.
 */
template <class T>
bool operator<(const optional<T> & lhs, const optional<T> & rhs)
{
    if (!rhs)
    {
        return false;
    }
    else if (!lhs)
    {
        return true;
    }
    else
    {
        return *lhs < *rhs;
    }
}

/**
 * \brief Comparison operator.
 * \returns <tt>rhs < lhs</tt>.
 */
template <class T>
bool operator>(const optional<T> & lhs, const optional<T> & rhs)
{
    return rhs < lhs;
}

/**
 * \brief Comparison operator.
 * \returns <tt>!(rhs < lhs)</tt>.
 */
template <class T>
bool operator<=(const optional<T> & lhs, const optional<T> & rhs)
{
    return !(rhs < lhs);
}

/**
 * \brief Comparison operator.
 * \returns <tt>!(rhs > lhs)</tt>.
 */
template <class T>
bool operator>=(const optional<T> & lhs, const optional<T> & rhs)
{
    return !(lhs < rhs);
}

/**
 * \brief Equality operator.
 * \returns \c !x.
 */
template <class T>
bool operator==(const optional<T> & x, nullopt_t) noexcept
{
    return !x;
}

/**
 * \brief Equality operator.
 * \returns \c !x.
 */
template <class T>
bool operator==(nullopt_t, const optional<T> & x) noexcept
{
    return !x;
}

/**
 * \brief Inequality operator.
 * \returns \c bool(x).
 */
template <class T>
bool operator!=(const optional<T> & x, nullopt_t) noexcept
{
    return bool(x);
}

/**
 * \brief Inequality operator.
 * \returns \c bool(x).
 */
template <class T>
bool operator!=(nullopt_t, const optional<T> & x) noexcept
{
    return bool(x);
}

/**
 * \brief Comparison operator.
 * \returns \c false.
 */
template <class T>
bool operator<(const optional<T> & x, nullopt_t) noexcept
{
    return false;
}

/**
 * \brief Comparison operator.
 * \returns \c bool(x).
 */
template <class T>
bool operator<(nullopt_t, const optional<T> & x) noexcept
{
    return bool(x);
}

/**
 * \brief Comparison operator.
 * \returns \c !x.
 */
template <class T>
bool operator<=(const optional<T> & x, nullopt_t) noexcept
{
    return !x;
}

/**
 * \brief Comparison operator.
 * \returns \c true.
 */
template <class T>
bool operator<=(nullopt_t, const optional<T> & x) noexcept
{
    return true;
}

/**
 * \brief Comparison operator.
 * \returns \c bool(x).
 */
template <class T>
bool operator>(const optional<T> & x, nullopt_t) noexcept
{
    return bool(x);
}

/**
 * \brief Comparison operator.
 * \returns \c false.
 */
template <class T>
bool operator>(nullopt_t, const optional<T> & x) noexcept
{
    return false;
}

/**
 * \brief Comparison operator.
 * \returns \c true.
 */
template <class T>
bool operator>=(const optional<T> & x, nullopt_t) noexcept
{
    return true;
}

/**
 * \brief Comparison operator.
 * \returns \c !x.
 */
template <class T>
bool operator>=(nullopt_t, const optional<T> & x) noexcept
{
    return !x;
}

/**
 * \brief Equality operator.
 * \returns <tt>bool(x) ? *x == v : false</tt>.
 */
template <class T>
bool operator==(const optional<T> & x, const T & v)
{
    return bool(x) ? *x == v : false;
}

/**
 * \brief Equality operator.
 * \returns <tt>bool(x) ? v == *x : false</tt>.
 */
template <class T>
bool operator==(const T & v, const optional<T> & x)
{
    return bool(x) ? v == *x : false;
}

/**
 * \brief Inequality operator.
 * \returns <tt>bool(x) ? !(*x == v) : true</tt>.
 */
template <class T>
bool operator!=(const optional<T> & x, const T & v)
{
    return bool(x) ? !(*x == v) : true;
}

/**
 * \brief Inequality operator.
 * \returns <tt>bool(x) ? !(v == *x) : true</tt>.
 */
template <class T>
bool operator!=(const T & v, const optional<T> & x)
{
    return bool(x) ? !(v == *x) : true;
}

/**
 * \brief Comparison operator.
 * \returns <tt>bool(x) ? *x < v : true</tt>.
 */
template <class T>
bool operator<(const optional<T> & x, const T & v)
{
    return bool(x) ? *x < v : true;
}

/**
 * \brief Comparison operator.
 * \returns <tt>bool(x) ? v < *x : false</tt>.
 */
template <class T>
bool operator<(const T & v, const optional<T> & x)
{
    return bool(x) ? v < *x : false;
}

/**
 * \brief Comparison operator.
 * \returns <tt>bool(x) ? *x < v : true</tt>.
 */
template <class T>
bool operator>(const optional<T> & x, const T & v)
{
    return bool(x) ? *x < v : true;
}

/**
 * \brief Comparison operator.
 * \returns <tt>bool(x) ? v < *x : false</tt>.
 */
template <class T>
bool operator>(const T & v, const optional<T> & x)
{
    return bool(x) ? v < *x : false;
}

/**
 * \brief Comparison operator.
 * \returns <tt>!(x < v)</tt>.
 */
template <class T>
bool operator>=(const optional<T> & x, const T & v)
{
    return !(x < v);
}

/**
 * \brief Comparison operator.
 * \returns <tt>!(v < x)</tt>.
 */
template <class T>
bool operator>=(const T & v, const optional<T> & x)
{
    return !(v < x);
}

/**
 * \brief Comparison operator.
 * \returns <tt>!(x > v)</tt>.
 */
template <class T>
bool operator<=(const optional<T> & x, const T & v)
{
    return !(x > v);
}

/**
 * \brief Comparison operator.
 * \returns <tt>!(v > x)</tt>.
 */
template <class T>
bool operator<=(const T & v, const optional<T> & x)
{
    return !(v > x);
}

/**
 * \ingroup optional
 * \brief Swap function.
 * \param lhs The first optional object.
 * \param rhs The second optional object.
 *
 * Calls:
 * \code{.cpp}
 * lhs.swap(rhs);
 * \endcode
 */
template <class T>
inline void swap(optional<T> & lhs, optional<T> & rhs) noexcept(noexcept(lhs.swap(rhs)))
{
    return lhs.swap(rhs);
}

/**
 * \ingroup optional
 * \brief A helper function to contruct an optional object.
 * \returns <tt>optional<typename
 * std::decay<T>::type>(std::forward(value))</tt>.
 */
template <class T>
inline constexpr optional<typename std::decay<T>::type> make_optional(T && value)
{
    return optional<typename std::decay<T>::type>(std::forward(value));
}
} // namespace network

#endif // NETWORK_OPTIONAL_INC
