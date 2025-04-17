#pragma once
#include <tuple>

namespace chip {
namespace app {
namespace Meta {

namespace detail {

template <class... T>
using LastElement = std::tuple_element_t<sizeof...(T) - 1, std::tuple<T...>>;

template <class S, template <class...> class T>
struct IsMetatypeImpl : std::false_type
{
};
template <template <class...> class T, class... S>
struct IsMetatypeImpl<T<S...>, T> : std::true_type
{
};

template <class S, template <class...> class T>
constexpr bool IsMetatype = IsMetatypeImpl<S, T>::value;

template <std::size_t Index, std::size_t... Is, class... Ts>
auto RestTuple(const std::tuple<Ts...> & tuple, std::index_sequence<Is...>)
{
    return std::make_tuple(std::get<Index + Is>(tuple)...);
}

// Gives the rest tuple from the Index forward
template <std::size_t Index, class... Ts>
auto RestTuple(const std::tuple<Ts...> & tuple)
{
    return RestTuple<Index>(tuple, std::make_index_sequence<sizeof...(Ts) - Index>{});
}

template <class... T>
auto MaybeMakeTuple(std::tuple<T...> Some)
{
    return Some;
}
template <class T, class = std::enable_if_t<!detail::IsMetatype<T, std::tuple>, void>>
auto MaybeMakeTuple(T && Some)
{
    return std::make_tuple(std::forward<T>(Some));
}

template <class TDefault>
struct CaseDefault
{
    TDefault call;
};
template <class TDefault>
CaseDefault(TDefault) -> CaseDefault<TDefault>;

template <class TCase, class TCallable>
struct Case
{
    TCase test;
    TCallable call;
};
template <class TCase, class TCallable>
Case(TCase, TCallable) -> Case<TCase, TCallable>;

} // namespace detail

template <auto VTarget, class TCallable>
auto Case(TCallable callable)
{
    auto equals = [](auto k) { return k == VTarget; };
    if constexpr (std::is_invocable_v<TCallable, Object<VTarget>>)
    {
        return detail::Case{ equals, [=]() { return callable(Object<VTarget>{}); } };
    }
    else
    {
        return detail::Case{ equals, callable };
    }
}

template <auto... values>
auto Case(ObjectList<values...> list)
{
    return [](auto callable) { return std::make_tuple(Case<values>(callable)...); };
}

// Defines the default case
template <class T>
auto Default(T value)
{
    if constexpr (std::is_invocable_v<T>)
    {
        return detail::CaseDefault{ value };
    }
    else
    {
        return detail::CaseDefault{ [=]() { return value; } };
    }
}

template <class T, class... TCases>
auto Switch(T && value, TCases... cases)
{
    return Switch(std::forward<T>(value), std::tuple_cat(detail::MaybeMakeTuple(cases)...));
}

template <class T, class... TCases>
auto Switch(T && value, std::tuple<TCases...> cases)
{
    static_assert(detail::IsMetatype<detail::LastElement<TCases...>, detail::CaseDefault>, "Last element must be a default case");

    // Handling the cases 4 by 4 to diminish the template explosion
    if constexpr (sizeof...(TCases) == 1)
    {
        return std::get<0>(cases).call();
    }
    else if (std::get<0>(cases).test(value))
    {
        return std::get<0>(cases).call();
    }
    else if constexpr (sizeof...(TCases) == 2)
    {
        return std::get<1>(cases).call();
    }
    else if (std::get<1>(cases).test(value))
    {
        return std::get<1>(cases).call();
    }
    else if constexpr (sizeof...(TCases) == 3)
    {
        return std::get<2>(cases).call();
    }
    else if (std::get<2>(cases).test(value))
    {
        return std::get<2>(cases).call();
    }
    else if constexpr (sizeof...(TCases) == 4)
    {
        return std::get<3>(cases).call();
    }
    else if (std::get<3>(cases).test(value))
    {
        return std::get<3>(cases).call();
    }
    else
    {
        return Switch(value, detail::RestTuple<4>(cases));
    }
}

} // namespace Meta
} // namespace app
} // namespace chip
