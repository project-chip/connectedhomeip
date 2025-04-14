#pragma once
#include <tuple>
#include <utility>

namespace chip {

namespace detail {
template <typename T>
struct SplittedLambdaCallerImpl;

template <class TReturn, class TLambda, class... TArgs>
struct SplittedLambdaCallerImpl<TReturn (TLambda::*)(TArgs...) const>
{
    using CallSignature = TReturn(TArgs... args, void * context);
    template <std::size_t I>
    using TupleElement = std::tuple_element<I, std::tuple<CallSignature *, void *>>;

    static TReturn Call(TArgs... args, void * context) { return (*static_cast<TLambda *>(context))(std::forward<TArgs>(args)...); }
};

} // namespace detail

template <class TLambda>
struct SplittedLambda : detail::SplittedLambdaCallerImpl<decltype(&TLambda::operator())>
{
    TLambda callable;

    SplittedLambda(TLambda callable) : callable(callable) {}
    SplittedLambda(SplittedLambda &)  = delete; // Cannot be copied
    SplittedLambda(SplittedLambda &&) = delete; // Cannot be moved

    inline void * Context() { return static_cast<void *>(&callable); }
    inline auto Caller() { return &this->Call; }
};

template <class T>
SplittedLambda<T> SplitLambda(T callable)
{
    return SplittedLambda<T>{ callable };
};

} // namespace chip
