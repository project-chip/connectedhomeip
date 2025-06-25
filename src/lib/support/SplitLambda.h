#pragma once
#include <tuple>
#include <utility>

namespace chip {

namespace detail {
template <typename T>
struct SplitLambdaCallerImpl;

template <class TReturn, class TLambda, class... TArgs>
struct SplitLambdaCallerImpl<TReturn (TLambda::*)(TArgs...) const>
{
    static TReturn Call(TArgs... args, void * context) { return (*static_cast<TLambda *>(context))(std::forward<TArgs>(args)...); }
};

} // namespace detail

/// @brief Helper Object to use Lambdas through C-Like APIs where context is split from the callback.
///        This incurs no runtime code execution; just keeps everything typesafe.
/// @tparam TLambda
/// @example
///     /* FunctionAPI */
///     int api_function(void (*callback) (int api_value, void * context), void * context );
///
///     int local_function()
///     {
///         int local_api_value = -1;
///         int other_local_variable = 7;
///
///         SplitLambda on_api_update_my_vars = [&](int value){
///             local_api_value = value;
///             other_local_variable = value +3;
///         };
///
///         /*Call API */
///         return api_function(on_api_update_my_vars.Caller(), on_api_update_my_vars.Context());
///     }
template <class TLambda>
struct SplitLambda : detail::SplitLambdaCallerImpl<decltype(&TLambda::operator())>
{
    TLambda callable;

    SplitLambda(TLambda callable_) : callable(callable_) {}
    SplitLambda(SplitLambda &)  = delete; // Cannot be copied
    SplitLambda(SplitLambda &&) = delete; // Cannot be moved

    inline void * Context() { return static_cast<void *>(&callable); }
    inline auto Caller() { return &this->Call; }
};

template <class TLambda>
SplitLambda(TLambda callable_) -> SplitLambda<TLambda>;

} // namespace chip
