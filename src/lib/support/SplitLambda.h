#pragma once
#include <tuple>
#include <utility>

namespace chip {

namespace detail {
template <typename T>
struct SplitLambdaCallerImpl;

// Detail -- Specialization to extract argument and return types from the callable type.
template <class TReturn, class TCallable, class... TArgs>
struct SplitLambdaCallerImpl<TReturn (TCallable::*)(TArgs...) const>
{

    // This call function knows the arguments and provides the signature required for the C-like callbacks
    static TReturn Call(TArgs... args, void * context)
    {
        return (*static_cast<TCallable *>(context))(std::forward<TArgs>(args)...);
    }
};
} // namespace detail

/// @brief Helper Object to use Lambdas through C-Like APIs where context is split from the callback.
///        This incurs no runtime code execution; just keeps everything typesafe.
/// @tparam TCallable
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
template <class TCallable>
struct SplitLambda : detail::SplitLambdaCallerImpl<decltype(&TCallable::operator())>
{
    TCallable callable;

    SplitLambda(TCallable callable_) : callable(callable_) {}
    SplitLambda(SplitLambda &)  = delete; // Cannot be copied
    SplitLambda(SplitLambda &&) = delete; // Cannot be moved

    inline void * Context() { return static_cast<void *>(&callable); }
    inline auto Caller() { return &this->Call; }
};

template <class TCallable>
SplitLambda(TCallable callable_) -> SplitLambda<TCallable>;

} // namespace chip
