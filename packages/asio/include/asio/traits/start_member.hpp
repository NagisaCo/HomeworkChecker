﻿//
// traits/start_member.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2020 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_TRAITS_START_MEMBER_HPP
#define ASIO_TRAITS_START_MEMBER_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.hpp"
#include "asio/detail/type_traits.hpp"

#if defined(ASIO_HAS_DECLTYPE) \
  && defined(ASIO_HAS_NOEXCEPT) \
  && defined(ASIO_HAS_WORKING_EXPRESSION_SFINAE)
# define ASIO_HAS_DEDUCED_START_MEMBER_TRAIT 1
#endif // defined(ASIO_HAS_DECLTYPE)
       //   && defined(ASIO_HAS_NOEXCEPT)
       //   && defined(ASIO_HAS_WORKING_EXPRESSION_SFINAE)

#include "asio/detail/push_options.hpp"

namespace asio {
namespace traits {

template <typename T, typename = void>
struct start_member_default;

template <typename T, typename = void>
struct start_member;

} // namespace traits
namespace detail {

struct no_start_member
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = false);
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept = false);
};

#if defined(ASIO_HAS_DEDUCED_START_MEMBER_TRAIT)

template <typename T, typename = void>
struct start_member_trait : no_start_member
{
};

template <typename T>
struct start_member_trait<T,
  typename void_type<
    decltype(declval<T>().start())
  >::type>
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = true);

  using result_type = decltype(declval<T>().start());

  ASIO_STATIC_CONSTEXPR(bool,
    is_noexcept = noexcept(declval<T>().start()));
};

#else // defined(ASIO_HAS_DEDUCED_START_MEMBER_TRAIT)

template <typename T, typename = void>
struct start_member_trait :
  conditional<
    is_same<T, typename remove_reference<T>::type>::value,
    typename conditional<
      is_same<T, typename add_const<T>::type>::value,
      no_start_member,
      traits::start_member<typename add_const<T>::type>
    >::type,
    traits::start_member<typename remove_reference<T>::type>
  >::type
{
};

#endif // defined(ASIO_HAS_DEDUCED_START_MEMBER_TRAIT)

} // namespace detail
namespace traits {

template <typename T, typename>
struct start_member_default :
  detail::start_member_trait<T>
{
};

template <typename T, typename>
struct start_member :
  start_member_default<T>
{
};

} // namespace traits
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_TRAITS_START_MEMBER_HPP
