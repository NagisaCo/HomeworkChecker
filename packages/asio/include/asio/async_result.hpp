﻿//
// async_result.hpp
// ~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2020 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_ASYNC_RESULT_HPP
#define ASIO_ASYNC_RESULT_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.hpp"
#include "asio/detail/type_traits.hpp"
#include "asio/detail/variadic_templates.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {

#if defined(ASIO_HAS_CONCEPTS) \
  && defined(ASIO_HAS_VARIADIC_TEMPLATES) \
  && defined(ASIO_HAS_DECLTYPE)

namespace detail {

template <typename T>
struct is_completion_signature : false_type
{
};

template <typename R, typename... Args>
struct is_completion_signature<R(Args...)> : true_type
{
};

template <typename T, typename... Args>
ASIO_CONCEPT callable_with = requires(T t, Args&&... args)
{
  t(static_cast<Args&&>(args)...);
};

template <typename T, typename Signature>
struct is_completion_handler_for : false_type
{
};

template <typename T, typename R, typename... Args>
struct is_completion_handler_for<T, R(Args...)>
  : integral_constant<bool, (callable_with<T, Args...>)>
{
};

} // namespace detail

template <typename T>
ASIO_CONCEPT completion_signature =
  detail::is_completion_signature<T>::value;

#define ASIO_COMPLETION_SIGNATURE \
  ::asio::completion_signature

template <typename T, typename Signature>
ASIO_CONCEPT completion_handler_for =
  detail::is_completion_signature<Signature>::value
    && detail::is_completion_handler_for<T, Signature>::value;

#define ASIO_COMPLETION_HANDLER_FOR(s) \
  ::asio::completion_handler_for<s>

#else // defined(ASIO_HAS_CONCEPTS)
      //   && defined(ASIO_HAS_VARIADIC_TEMPLATES)
      //   && defined(ASIO_HAS_DECLTYPE)

#define ASIO_COMPLETION_SIGNATURE typename
#define ASIO_COMPLETION_HANDLER_FOR(s) typename

#endif // defined(ASIO_HAS_CONCEPTS)
       //   && defined(ASIO_HAS_VARIADIC_TEMPLATES)
       //   && defined(ASIO_HAS_DECLTYPE)

/// An interface for customising the behaviour of an initiating function.
/**
 * The async_result traits class is used for determining:
 *
 * @li the concrete completion handler type to be called at the end of the
 * asynchronous operation;
 *
 * @li the initiating function return type; and
 *
 * @li how the return value of the initiating function is obtained.
 *
 * The trait allows the handler and return types to be determined at the point
 * where the specific completion handler signature is known.
 *
 * This template may be specialised for user-defined completion token types.
 * The primary template assumes that the CompletionToken is the completion
 * handler.
 */
template <typename CompletionToken, ASIO_COMPLETION_SIGNATURE Signature>
class async_result
{
public:
  /// The concrete completion handler type for the specific signature.
  typedef CompletionToken completion_handler_type;

  /// The return type of the initiating function.
  typedef void return_type;

  /// Construct an async result from a given handler.
  /**
   * When using a specalised async_result, the constructor has an opportunity
   * to initialise some state associated with the completion handler, which is
   * then returned from the initiating function.
   */
  explicit async_result(completion_handler_type& h)
  {
    (void)h;
  }

  /// Obtain the value to be returned from the initiating function.
  return_type get()
  {
  }

#if defined(GENERATING_DOCUMENTATION)

  /// Initiate the asynchronous operation that will produce the result, and
  /// obtain the value to be returned from the initiating function.
  template <typename Initiation, typename RawCompletionToken, typename... Args>
  static return_type initiate(
      ASIO_MOVE_ARG(Initiation) initiation,
      ASIO_MOVE_ARG(RawCompletionToken) token,
      ASIO_MOVE_ARG(Args)... args);

#elif defined(ASIO_HAS_VARIADIC_TEMPLATES)

  template <typename Initiation,
      ASIO_COMPLETION_HANDLER_FOR(Signature) RawCompletionToken,
      typename... Args>
  static return_type initiate(
      ASIO_MOVE_ARG(Initiation) initiation,
      ASIO_MOVE_ARG(RawCompletionToken) token,
      ASIO_MOVE_ARG(Args)... args)
  {
    ASIO_MOVE_CAST(Initiation)(initiation)(
        ASIO_MOVE_CAST(RawCompletionToken)(token),
        ASIO_MOVE_CAST(Args)(args)...);
  }

#else // defined(ASIO_HAS_VARIADIC_TEMPLATES)

  template <typename Initiation,
      ASIO_COMPLETION_HANDLER_FOR(Signature) RawCompletionToken>
  static return_type initiate(
      ASIO_MOVE_ARG(Initiation) initiation,
      ASIO_MOVE_ARG(RawCompletionToken) token)
  {
    ASIO_MOVE_CAST(Initiation)(initiation)(
        ASIO_MOVE_CAST(RawCompletionToken)(token));
  }

#define ASIO_PRIVATE_INITIATE_DEF(n) \
  template <typename Initiation, \
      ASIO_COMPLETION_HANDLER_FOR(Signature) RawCompletionToken, \
      ASIO_VARIADIC_TPARAMS(n)> \
  static return_type initiate( \
      ASIO_MOVE_ARG(Initiation) initiation, \
      ASIO_MOVE_ARG(RawCompletionToken) token, \
      ASIO_VARIADIC_MOVE_PARAMS(n)) \
  { \
    ASIO_MOVE_CAST(Initiation)(initiation)( \
        ASIO_MOVE_CAST(RawCompletionToken)(token), \
        ASIO_VARIADIC_MOVE_ARGS(n)); \
  } \
  /**/
  ASIO_VARIADIC_GENERATE(ASIO_PRIVATE_INITIATE_DEF)
#undef ASIO_PRIVATE_INITIATE_DEF

#endif // defined(ASIO_HAS_VARIADIC_TEMPLATES)

private:
  async_result(const async_result&) ASIO_DELETED;
  async_result& operator=(const async_result&) ASIO_DELETED;
};

#if !defined(GENERATING_DOCUMENTATION)

template <ASIO_COMPLETION_SIGNATURE Signature>
class async_result<void, Signature>
{
  // Empty.
};

#endif // !defined(GENERATING_DOCUMENTATION)

/// Helper template to deduce the handler type from a CompletionToken, capture
/// a local copy of the handler, and then create an async_result for the
/// handler.
template <typename CompletionToken, ASIO_COMPLETION_SIGNATURE Signature>
struct async_completion
{
  /// The real handler type to be used for the asynchronous operation.
  typedef typename asio::async_result<
    typename decay<CompletionToken>::type,
      Signature>::completion_handler_type completion_handler_type;

#if defined(ASIO_HAS_MOVE) || defined(GENERATING_DOCUMENTATION)
  /// Constructor.
  /**
   * The constructor creates the concrete completion handler and makes the link
   * between the handler and the asynchronous result.
   */
  explicit async_completion(CompletionToken& token)
    : completion_handler(static_cast<typename conditional<
        is_same<CompletionToken, completion_handler_type>::value,
        completion_handler_type&, CompletionToken&&>::type>(token)),
      result(completion_handler)
  {
  }
#else // defined(ASIO_HAS_MOVE) || defined(GENERATING_DOCUMENTATION)
  explicit async_completion(typename decay<CompletionToken>::type& token)
    : completion_handler(token),
      result(completion_handler)
  {
  }

  explicit async_completion(const typename decay<CompletionToken>::type& token)
    : completion_handler(token),
      result(completion_handler)
  {
  }
#endif // defined(ASIO_HAS_MOVE) || defined(GENERATING_DOCUMENTATION)

  /// A copy of, or reference to, a real handler object.
#if defined(ASIO_HAS_MOVE) || defined(GENERATING_DOCUMENTATION)
  typename conditional<
    is_same<CompletionToken, completion_handler_type>::value,
    completion_handler_type&, completion_handler_type>::type completion_handler;
#else // defined(ASIO_HAS_MOVE) || defined(GENERATING_DOCUMENTATION)
  completion_handler_type completion_handler;
#endif // defined(ASIO_HAS_MOVE) || defined(GENERATING_DOCUMENTATION)

  /// The result of the asynchronous operation's initiating function.
  async_result<typename decay<CompletionToken>::type, Signature> result;
};

namespace detail {

template <typename CompletionToken, typename Signature>
struct async_result_helper
  : async_result<typename decay<CompletionToken>::type, Signature>
{
};

struct async_result_memfns_base
{
  void initiate();
};

template <typename T>
struct async_result_memfns_derived
  : T, async_result_memfns_base
{
};

template <typename T, T>
struct async_result_memfns_check
{
};

template <typename>
char (&async_result_initiate_memfn_helper(...))[2];

template <typename T>
char async_result_initiate_memfn_helper(
    async_result_memfns_check<
      void (async_result_memfns_base::*)(),
      &async_result_memfns_derived<T>::initiate>*);

template <typename CompletionToken, typename Signature>
struct async_result_has_initiate_memfn
  : integral_constant<bool, sizeof(async_result_initiate_memfn_helper<
      async_result<typename decay<CompletionToken>::type, Signature>
    >(0)) != 1>
{
};

} // namespace detail

#if defined(GENERATING_DOCUMENTATION)
# define ASIO_INITFN_RESULT_TYPE(ct, sig) \
  void_or_deduced
#elif defined(_MSC_VER) && (_MSC_VER < 1500)
# define ASIO_INITFN_RESULT_TYPE(ct, sig) \
  typename ::asio::detail::async_result_helper< \
    ct, sig>::return_type
#define ASIO_HANDLER_TYPE(ct, sig) \
  typename ::asio::detail::async_result_helper< \
    ct, sig>::completion_handler_type
#else
# define ASIO_INITFN_RESULT_TYPE(ct, sig) \
  typename ::asio::async_result< \
    typename ::asio::decay<ct>::type, sig>::return_type
#define ASIO_HANDLER_TYPE(ct, sig) \
  typename ::asio::async_result< \
    typename ::asio::decay<ct>::type, sig>::completion_handler_type
#endif

#if defined(GENERATING_DOCUMENTATION)
# define ASIO_INITFN_AUTO_RESULT_TYPE(ct, sig) \
  auto
#elif defined(ASIO_HAS_RETURN_TYPE_DEDUCTION)
# define ASIO_INITFN_AUTO_RESULT_TYPE(ct, sig) \
  auto
#else
# define ASIO_INITFN_AUTO_RESULT_TYPE(ct, sig) \
  ASIO_INITFN_RESULT_TYPE(ct, sig)
#endif

#if defined(GENERATING_DOCUMENTATION)
# define ASIO_INITFN_DEDUCED_RESULT_TYPE(ct, sig, expr) \
  void_or_deduced
#elif defined(ASIO_HAS_DECLTYPE)
# define ASIO_INITFN_DEDUCED_RESULT_TYPE(ct, sig, expr) \
  decltype expr
#else
# define ASIO_INITFN_DEDUCED_RESULT_TYPE(ct, sig, expr) \
  ASIO_INITFN_RESULT_TYPE(ct, sig)
#endif

#if defined(GENERATING_DOCUMENTATION)

template <typename CompletionToken,
    completion_signature Signature,
    typename Initiation, typename... Args>
void_or_deduced async_initiate(
    ASIO_MOVE_ARG(Initiation) initiation,
    ASIO_NONDEDUCED_MOVE_ARG(CompletionToken),
    ASIO_MOVE_ARG(Args)... args);

#elif defined(ASIO_HAS_VARIADIC_TEMPLATES)

template <typename CompletionToken,
    ASIO_COMPLETION_SIGNATURE Signature,
    typename Initiation, typename... Args>
inline typename enable_if<
    detail::async_result_has_initiate_memfn<CompletionToken, Signature>::value,
    ASIO_INITFN_DEDUCED_RESULT_TYPE(CompletionToken, Signature,
      (async_result<typename decay<CompletionToken>::type,
        Signature>::initiate(declval<ASIO_MOVE_ARG(Initiation)>(),
          declval<ASIO_MOVE_ARG(CompletionToken)>(),
          declval<ASIO_MOVE_ARG(Args)>()...)))>::type
async_initiate(ASIO_MOVE_ARG(Initiation) initiation,
    ASIO_NONDEDUCED_MOVE_ARG(CompletionToken) token,
    ASIO_MOVE_ARG(Args)... args)
{
  return async_result<typename decay<CompletionToken>::type,
    Signature>::initiate(ASIO_MOVE_CAST(Initiation)(initiation),
      ASIO_MOVE_CAST(CompletionToken)(token),
      ASIO_MOVE_CAST(Args)(args)...);
}

template <typename CompletionToken,
    ASIO_COMPLETION_SIGNATURE Signature,
    typename Initiation, typename... Args>
inline typename enable_if<
    !detail::async_result_has_initiate_memfn<CompletionToken, Signature>::value,
    ASIO_INITFN_RESULT_TYPE(CompletionToken, Signature)>::type
async_initiate(ASIO_MOVE_ARG(Initiation) initiation,
    ASIO_NONDEDUCED_MOVE_ARG(CompletionToken) token,
    ASIO_MOVE_ARG(Args)... args)
{
  async_completion<CompletionToken, Signature> completion(token);

  ASIO_MOVE_CAST(Initiation)(initiation)(
      ASIO_MOVE_CAST(ASIO_HANDLER_TYPE(CompletionToken,
        Signature))(completion.completion_handler),
      ASIO_MOVE_CAST(Args)(args)...);

  return completion.result.get();
}

#else // defined(ASIO_HAS_VARIADIC_TEMPLATES)

template <typename CompletionToken,
    ASIO_COMPLETION_SIGNATURE Signature,
    typename Initiation>
inline typename enable_if<
    detail::async_result_has_initiate_memfn<CompletionToken, Signature>::value,
    ASIO_INITFN_DEDUCED_RESULT_TYPE(CompletionToken, Signature,
      (async_result<typename decay<CompletionToken>::type,
        Signature>::initiate(declval<ASIO_MOVE_ARG(Initiation)>(),
          declval<ASIO_MOVE_ARG(CompletionToken)>())))>::type
async_initiate(ASIO_MOVE_ARG(Initiation) initiation,
    ASIO_NONDEDUCED_MOVE_ARG(CompletionToken) token)
{
  return async_result<typename decay<CompletionToken>::type,
    Signature>::initiate(ASIO_MOVE_CAST(Initiation)(initiation),
      ASIO_MOVE_CAST(CompletionToken)(token));
}

template <typename CompletionToken,
    ASIO_COMPLETION_SIGNATURE Signature,
    typename Initiation>
inline typename enable_if<
    !detail::async_result_has_initiate_memfn<CompletionToken, Signature>::value,
    ASIO_INITFN_RESULT_TYPE(CompletionToken, Signature)>::type
async_initiate(ASIO_MOVE_ARG(Initiation) initiation,
    ASIO_NONDEDUCED_MOVE_ARG(CompletionToken) token)
{
  async_completion<CompletionToken, Signature> completion(token);

  ASIO_MOVE_CAST(Initiation)(initiation)(
      ASIO_MOVE_CAST(ASIO_HANDLER_TYPE(CompletionToken,
        Signature))(completion.completion_handler));

  return completion.result.get();
}

#define ASIO_PRIVATE_INITIATE_DEF(n) \
  template <typename CompletionToken, \
      ASIO_COMPLETION_SIGNATURE Signature, \
      typename Initiation, ASIO_VARIADIC_TPARAMS(n)> \
  inline typename enable_if< \
      detail::async_result_has_initiate_memfn< \
        CompletionToken, Signature>::value, \
      ASIO_INITFN_DEDUCED_RESULT_TYPE(CompletionToken, Signature, \
        (async_result<typename decay<CompletionToken>::type, \
          Signature>::initiate(declval<ASIO_MOVE_ARG(Initiation)>(), \
            declval<ASIO_MOVE_ARG(CompletionToken)>(), \
            ASIO_VARIADIC_MOVE_DECLVAL(n))))>::type \
  async_initiate(ASIO_MOVE_ARG(Initiation) initiation, \
      ASIO_NONDEDUCED_MOVE_ARG(CompletionToken) token, \
      ASIO_VARIADIC_MOVE_PARAMS(n)) \
  { \
    return async_result<typename decay<CompletionToken>::type, \
      Signature>::initiate(ASIO_MOVE_CAST(Initiation)(initiation), \
        ASIO_MOVE_CAST(CompletionToken)(token), \
        ASIO_VARIADIC_MOVE_ARGS(n)); \
  } \
  \
  template <typename CompletionToken, \
      ASIO_COMPLETION_SIGNATURE Signature, \
      typename Initiation, ASIO_VARIADIC_TPARAMS(n)> \
  inline typename enable_if< \
      !detail::async_result_has_initiate_memfn< \
        CompletionToken, Signature>::value, \
      ASIO_INITFN_RESULT_TYPE(CompletionToken, Signature)>::type \
  async_initiate(ASIO_MOVE_ARG(Initiation) initiation, \
      ASIO_NONDEDUCED_MOVE_ARG(CompletionToken) token, \
      ASIO_VARIADIC_MOVE_PARAMS(n)) \
  { \
    async_completion<CompletionToken, Signature> completion(token); \
  \
    ASIO_MOVE_CAST(Initiation)(initiation)( \
        ASIO_MOVE_CAST(ASIO_HANDLER_TYPE(CompletionToken, \
          Signature))(completion.completion_handler), \
        ASIO_VARIADIC_MOVE_ARGS(n)); \
  \
    return completion.result.get(); \
  } \
  /**/
  ASIO_VARIADIC_GENERATE(ASIO_PRIVATE_INITIATE_DEF)
#undef ASIO_PRIVATE_INITIATE_DEF

#endif // defined(ASIO_HAS_VARIADIC_TEMPLATES)

#if defined(ASIO_HAS_CONCEPTS) \
  && defined(ASIO_HAS_VARIADIC_TEMPLATES) \
  && defined(ASIO_HAS_DECLTYPE)

namespace detail {

template <typename Signature>
struct initiation_archetype
{
  template <completion_handler_for<Signature> CompletionHandler>
  void operator()(CompletionHandler&&) const
  {
  }
};

} // namespace detail

template <typename T, typename Signature>
ASIO_CONCEPT completion_token_for =
  detail::is_completion_signature<Signature>::value
  &&
  requires(T&& t)
  {
    async_initiate<T, Signature>(detail::initiation_archetype<Signature>{}, t);
  };

#define ASIO_COMPLETION_TOKEN_FOR(s) \
  ::asio::completion_token_for<s>

#else // defined(ASIO_HAS_CONCEPTS)
      //   && defined(ASIO_HAS_VARIADIC_TEMPLATES)
      //   && defined(ASIO_HAS_DECLTYPE)

#define ASIO_COMPLETION_TOKEN_FOR(s) typename

#endif // defined(ASIO_HAS_CONCEPTS)
       //   && defined(ASIO_HAS_VARIADIC_TEMPLATES)
       //   && defined(ASIO_HAS_DECLTYPE)

namespace detail {

template <typename T, typename = void>
struct default_completion_token_impl
{
  typedef void type;
};

template <typename T>
struct default_completion_token_impl<T,
  typename void_type<typename T::default_completion_token_type>::type>
{
  typedef typename T::default_completion_token_type type;
};

} // namespace detail

#if defined(GENERATING_DOCUMENTATION)

/// Traits type used to determine the default completion token type associated
/// with a type (such as an executor).
/**
 * A program may specialise this traits type if the @c T template parameter in
 * the specialisation is a user-defined type.
 *
 * Specialisations of this trait may provide a nested typedef @c type, which is
 * a default-constructible completion token type.
 */
template <typename T>
struct default_completion_token
{
  /// If @c T has a nested type @c default_completion_token_type,
  /// <tt>T::default_completion_token_type</tt>. Otherwise the typedef @c type
  /// is not defined.
  typedef see_below type;
};
#else
template <typename T>
struct default_completion_token
  : detail::default_completion_token_impl<T>
{
};
#endif

#if defined(ASIO_HAS_ALIAS_TEMPLATES)

template <typename T>
using default_completion_token_t = typename default_completion_token<T>::type;

#endif // defined(ASIO_HAS_ALIAS_TEMPLATES)

#if defined(ASIO_HAS_DEFAULT_FUNCTION_TEMPLATE_ARGUMENTS)

#define ASIO_DEFAULT_COMPLETION_TOKEN_TYPE(e) \
  = typename ::asio::default_completion_token<e>::type
#define ASIO_DEFAULT_COMPLETION_TOKEN(e) \
  = typename ::asio::default_completion_token<e>::type()

#else // defined(ASIO_HAS_DEFAULT_FUNCTION_TEMPLATE_ARGUMENTS)

#define ASIO_DEFAULT_COMPLETION_TOKEN_TYPE(e)
#define ASIO_DEFAULT_COMPLETION_TOKEN(e)

#endif // defined(ASIO_HAS_DEFAULT_FUNCTION_TEMPLATE_ARGUMENTS)

} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_ASYNC_RESULT_HPP
