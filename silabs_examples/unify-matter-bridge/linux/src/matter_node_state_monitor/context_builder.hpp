/******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/

#ifndef CONTEXT_BUILDER_HPP
#define CONTEXT_BUILDER_HPP
#include <vector>
#include <memory>
#include <variant>

/**
 * @brief Class that implements a builder pattern to dynamicly build a
 * contigious list. When `build()` is called, the list will be copied to an
 * unique vector so it can safely transfer ownership.
 *
 * The type that can be appended is generalized. e.g. this class can be reused
 * to build list of your type of choice.
 */
template<class T> class builder
{
  public:
  /**
 * Appends an item to the internal list of the builder. This list will later be
 * presented as an unique array.
 *
 * @param args  all arguments required to construct an object of type T
 */
  template<typename... Args> builder<T> &append(Args &&...args)
  {
    data.push_back(T {std::forward<Args>(args)...});
    return *this;
  }

  /**
 * Finalize the builder and call the on_done function. build() will copy the
 * gathered list items and present it a contigious unique array.
 */
  std::shared_ptr<const std::vector<T>> build() const
  {
    return std::make_shared<const std::vector<T>>(data);
  }

  private:
  std::vector<T> data;
};

/** SFINAE declarations to compile time figure out if a certain template
 * parameter is defined inside a variadic template list */
template<typename... T> struct TypeInList;

template<typename T, typename T2> struct TypeInList<T, T2> {
  static constexpr bool value = std::is_same<T, T2>();
};

template<typename T, typename T2, typename... other>
struct TypeInList<T, T2, other...> {
  static constexpr bool value
    = TypeInList<T, T2>::value ? true : TypeInList<T, other...>::value;
};

/**
 * @brief This context object is a small wrapper around a user defined type
 * `ContextType`. In this class you can register lists of other types that
 * require to have the same lifetime as the `ContextType`. This is useful for
 * when the `ContextType` has pointers to these other types as this class can
 * give guarantee that the lifetime of `ContextType` always outlives the
 * types it internally points to.
 *
 * Its not possible to register arbitrary types as associated lifetime objects.
 * only the types that are passed in as template arguments are able to be
 * registered. If you try to pass in another type, a static assert will fire
 * during compilation.
 *
 * This class works for generic types, for a more concrete usecase take a look
 * at matter_endpoint_builder
 *
 */
template<class ContextType, class... T> class lifetime_context
{
  public:
  /** the variant type which defines which types the vector can hold. */
  using ContextVariant = std::variant<std::shared_ptr<const std::vector<T>>...>;
  lifetime_context()   = default;

  lifetime_context &operator=(const lifetime_context &other) = delete;

  /** @brief dereference operator to easily access the wrapped type
   * `ContextType` */
  ContextType *operator*()
  {
    return &context;
  }

  const ContextType *operator*() const
  {
    return &context;
  }

  /** @brief dereference operator to easily access the wrapped type
   * `ContextType` */
  ContextType *operator->()
  {
    return &context;
  }

  /** @brief Function registers a list of elements of one of the defined types.
   * By passing this list the ownership is transferred to this context class.
   * This class gives a guarantee that the appended lists lifetime is at least
   * equally as long as the contained `ContextType`
   * @param lifetime a unique ptr to a list of elements 
   * */
  template<class U>
  void add_lifetime(std::shared_ptr<const std::vector<U>> &&lifetime)
  {
    static_assert(TypeInList<U, T...>::value,
                  "cannot add a type thats not declared as template type "
                  "for class lifetime_context");
    ContextVariant variant {std::move(lifetime)};
    lifetimes.push_back(std::move(variant));
  }

  private:
  ContextType context;
  std::vector<ContextVariant> lifetimes;
};

#endif