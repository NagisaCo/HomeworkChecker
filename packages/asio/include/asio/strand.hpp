﻿//
// strand.hpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2020 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_STRAND_HPP
#define ASIO_STRAND_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.hpp"
#include "asio/detail/strand_executor_service.hpp"
#include "asio/detail/type_traits.hpp"
#include "asio/execution/executor.hpp"
#include "asio/is_executor.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {

/// Provides serialised function invocation for any executor type.
template <typename Executor>
class strand
{
public:
  /// The type of the underlying executor.
  typedef Executor inner_executor_type;

  /// Default constructor.
  /**
   * This constructor is only valid if the underlying executor type is default
   * constructible.
   */
  strand()
    : executor_(),
      impl_(strand::create_implementation(executor_))
  {
  }

  /// Construct a strand for the specified executor.
  template <typename Executor1>
  explicit strand(const Executor1& e,
      typename enable_if<
        conditional<
          !is_same<Executor1, strand>::value,
          is_convertible<Executor1, Executor>,
          false_type
        >::type::value
      >::type* = 0)
    : executor_(e),
      impl_(strand::create_implementation(executor_))
  {
  }

  /// Copy constructor.
  strand(const strand& other) ASIO_NOEXCEPT
    : executor_(other.executor_),
      impl_(other.impl_)
  {
  }

  /// Converting constructor.
  /**
   * This constructor is only valid if the @c OtherExecutor type is convertible
   * to @c Executor.
   */
  template <class OtherExecutor>
  strand(
      const strand<OtherExecutor>& other) ASIO_NOEXCEPT
    : executor_(other.executor_),
      impl_(other.impl_)
  {
  }

  /// Assignment operator.
  strand& operator=(const strand& other) ASIO_NOEXCEPT
  {
    executor_ = other.executor_;
    impl_ = other.impl_;
    return *this;
  }

  /// Converting assignment operator.
  /**
   * This assignment operator is only valid if the @c OtherExecutor type is
   * convertible to @c Executor.
   */
  template <class OtherExecutor>
  strand& operator=(
      const strand<OtherExecutor>& other) ASIO_NOEXCEPT
  {
    executor_ = other.executor_;
    impl_ = other.impl_;
    return *this;
  }

#if defined(ASIO_HAS_MOVE) || defined(GENERATING_DOCUMENTATION)
  /// Move constructor.
  strand(strand&& other) ASIO_NOEXCEPT
    : executor_(ASIO_MOVE_CAST(Executor)(other.executor_)),
      impl_(ASIO_MOVE_CAST(implementation_type)(other.impl_))
  {
  }

  /// Converting move constructor.
  /**
   * This constructor is only valid if the @c OtherExecutor type is convertible
   * to @c Executor.
   */
  template <class OtherExecutor>
  strand(strand<OtherExecutor>&& other) ASIO_NOEXCEPT
    : executor_(ASIO_MOVE_CAST(OtherExecutor)(other.executor_)),
      impl_(ASIO_MOVE_CAST(implementation_type)(other.impl_))
  {
  }

  /// Move assignment operator.
  strand& operator=(strand&& other) ASIO_NOEXCEPT
  {
    executor_ = ASIO_MOVE_CAST(Executor)(other.executor_);
    impl_ = ASIO_MOVE_CAST(implementation_type)(other.impl_);
    return *this;
  }

  /// Converting move assignment operator.
  /**
   * This assignment operator is only valid if the @c OtherExecutor type is
   * convertible to @c Executor.
   */
  template <class OtherExecutor>
  strand& operator=(strand<OtherExecutor>&& other) ASIO_NOEXCEPT
  {
    executor_ = ASIO_MOVE_CAST(OtherExecutor)(other.executor_);
    impl_ = ASIO_MOVE_CAST(implementation_type)(other.impl_);
    return *this;
  }
#endif // defined(ASIO_HAS_MOVE) || defined(GENERATING_DOCUMENTATION)

  /// Destructor.
  ~strand() ASIO_NOEXCEPT
  {
  }

  /// Obtain the underlying executor.
  inner_executor_type get_inner_executor() const ASIO_NOEXCEPT
  {
    return executor_;
  }

  /// Forward a query to the underlying executor.
  /**
   * Do not call this function directly. It is intended for use with the
   * execution::execute customisation point.
   *
   * For example:
   * @code asio::strand<my_executor_type> ex = ...;
   * if (asio::query(ex, asio::execution::blocking)
   *       == asio::execution::blocking.never)
   *   ... @endcode
   */
  template <typename Property>
  typename enable_if<
    can_query<const Executor&, Property>::value,
    typename query_result<const Executor&, Property>::type
  >::type query(const Property& p) const
    ASIO_NOEXCEPT_IF((
      is_nothrow_query<const Executor&, Property>::value))
  {
    return asio::query(executor_, p);
  }

  /// Forward a requirement to the underlying executor.
  /**
   * Do not call this function directly. It is intended for use with the
   * asio::require customisation point.
   *
   * For example:
   * @code asio::strand<my_executor_type> ex1 = ...;
   * auto ex2 = asio::require(ex1,
   *     asio::execution::blocking.never); @endcode
   */
  template <typename Property>
  typename enable_if<
    can_require<const Executor&, Property>::value,
    strand<typename decay<
      typename require_result<const Executor&, Property>::type
    >::type>
  >::type require(const Property& p) const
    ASIO_NOEXCEPT_IF((
      is_nothrow_require<const Executor&, Property>::value))
  {
    return strand<typename decay<
      typename require_result<const Executor&, Property>::type
        >::type>(asio::require(executor_, p), impl_);
  }

  /// Forward a preference to the underlying executor.
  /**
   * Do not call this function directly. It is intended for use with the
   * asio::prefer customisation point.
   *
   * For example:
   * @code asio::strand<my_executor_type> ex1 = ...;
   * auto ex2 = asio::prefer(ex1,
   *     asio::execution::blocking.never); @endcode
   */
  template <typename Property>
  typename enable_if<
    can_prefer<const Executor&, Property>::value,
    strand<typename decay<
      typename prefer_result<const Executor&, Property>::type
    >::type>
  >::type prefer(const Property& p) const
    ASIO_NOEXCEPT_IF((
      is_nothrow_prefer<const Executor&, Property>::value))
  {
    return strand<typename decay<
      typename prefer_result<const Executor&, Property>::type
        >::type>(asio::prefer(executor_, p), impl_);
  }

#if !defined(ASIO_NO_TS_EXECUTORS)
  /// Obtain the underlying execution context.
  execution_context& context() const ASIO_NOEXCEPT
  {
    return executor_.context();
  }

  /// Inform the strand that it has some outstanding work to do.
  /**
   * The strand delegates this call to its underlying executor.
   */
  void on_work_started() const ASIO_NOEXCEPT
  {
    executor_.on_work_started();
  }

  /// Inform the strand that some work is no longer outstanding.
  /**
   * The strand delegates this call to its underlying executor.
   */
  void on_work_finished() const ASIO_NOEXCEPT
  {
    executor_.on_work_finished();
  }
#endif // !defined(ASIO_NO_TS_EXECUTORS)

  /// Request the strand to invoke the given function object.
  /**
   * Do not call this function directly. It is intended for use with the
   * execution::execute customisation point.
   *
   * For example:
   * @code asio::strand<my_executor_type> ex = ...;
   * execution::execute(ex, my_function_object); @endcode
   *
   * This function is used to ask the strand to execute the given function
   * object on its underlying executor. The function object will be executed
   * according to the properties of the underlying executor.
   *
   * @param f The function object to be called. The executor will make
   * a copy of the handler object as required. The function signature of the
   * function object must be: @code void function(); @endcode
   */
  template <typename Function>
  typename enable_if<
    execution::can_execute<const Executor&, Function>::value
  >::type execute(ASIO_MOVE_ARG(Function) f) const
  {
    detail::strand_executor_service::execute(impl_,
        executor_, ASIO_MOVE_CAST(Function)(f));
  }

#if !defined(ASIO_NO_TS_EXECUTORS)
  /// Request the strand to invoke the given function object.
  /**
   * This function is used to ask the strand to execute the given function
   * object on its underlying executor. The function object will be executed
   * inside this function if the strand is not otherwise busy and if the
   * underlying executor's @c dispatch() function is also able to execute the
   * function before returning.
   *
   * @param f The function object to be called. The executor will make
   * a copy of the handler object as required. The function signature of the
   * function object must be: @code void function(); @endcode
   *
   * @param a An allocator that may be used by the executor to allocate the
   * internal storage needed for function invocation.
   */
  template <typename Function, typename Allocator>
  void dispatch(ASIO_MOVE_ARG(Function) f, const Allocator& a) const
  {
    detail::strand_executor_service::dispatch(impl_,
        executor_, ASIO_MOVE_CAST(Function)(f), a);
  }

  /// Request the strand to invoke the given function object.
  /**
   * This function is used to ask the executor to execute the given function
   * object. The function object will never be executed inside this function.
   * Instead, it will be scheduled by the underlying executor's defer function.
   *
   * @param f The function object to be called. The executor will make
   * a copy of the handler object as required. The function signature of the
   * function object must be: @code void function(); @endcode
   *
   * @param a An allocator that may be used by the executor to allocate the
   * internal storage needed for function invocation.
   */
  template <typename Function, typename Allocator>
  void post(ASIO_MOVE_ARG(Function) f, const Allocator& a) const
  {
    detail::strand_executor_service::post(impl_,
        executor_, ASIO_MOVE_CAST(Function)(f), a);
  }

  /// Request the strand to invoke the given function object.
  /**
   * This function is used to ask the executor to execute the given function
   * object. The function object will never be executed inside this function.
   * Instead, it will be scheduled by the underlying executor's defer function.
   *
   * @param f The function object to be called. The executor will make
   * a copy of the handler object as required. The function signature of the
   * function object must be: @code void function(); @endcode
   *
   * @param a An allocator that may be used by the executor to allocate the
   * internal storage needed for function invocation.
   */
  template <typename Function, typename Allocator>
  void defer(ASIO_MOVE_ARG(Function) f, const Allocator& a) const
  {
    detail::strand_executor_service::defer(impl_,
        executor_, ASIO_MOVE_CAST(Function)(f), a);
  }
#endif // !defined(ASIO_NO_TS_EXECUTORS)

  /// Determine whether the strand is running in the current thread.
  /**
   * @return @c true if the current thread is executing a function that was
   * submitted to the strand using post(), dispatch() or defer(). Otherwise
   * returns @c false.
   */
  bool running_in_this_thread() const ASIO_NOEXCEPT
  {
    return detail::strand_executor_service::running_in_this_thread(impl_);
  }

  /// Compare two strands for equality.
  /**
   * Two strands are equal if they refer to the same ordered, non-concurrent
   * state.
   */
  friend bool operator==(const strand& a, const strand& b) ASIO_NOEXCEPT
  {
    return a.impl_ == b.impl_;
  }

  /// Compare two strands for inequality.
  /**
   * Two strands are equal if they refer to the same ordered, non-concurrent
   * state.
   */
  friend bool operator!=(const strand& a, const strand& b) ASIO_NOEXCEPT
  {
    return a.impl_ != b.impl_;
  }

#if defined(GENERATING_DOCUMENTATION)
private:
#endif // defined(GENERATING_DOCUMENTATION)
  typedef detail::strand_executor_service::implementation_type
    implementation_type;

  template <typename InnerExecutor>
  static implementation_type create_implementation(const InnerExecutor& ex,
      typename enable_if<
        can_query<InnerExecutor, execution::context_t>::value
      >::type* = 0)
  {
    return use_service<detail::strand_executor_service>(
        asio::query(ex, execution::context)).create_implementation();
  }

  template <typename InnerExecutor>
  static implementation_type create_implementation(const InnerExecutor& ex,
      typename enable_if<
        !can_query<InnerExecutor, execution::context_t>::value
      >::type* = 0)
  {
    return use_service<detail::strand_executor_service>(
        ex.context()).create_implementation();
  }

  strand(const Executor& ex, const implementation_type& impl)
    : executor_(ex),
      impl_(impl)
  {
  }

  Executor executor_;
  implementation_type impl_;
};

/** @defgroup make_strand asio::make_strand
 *
 * @brief The asio::make_strand function creates a @ref strand object for
 * an executor or execution context.
 */
/*@{*/

/// Create a @ref strand object for an executor.
template <typename Executor>
inline strand<Executor> make_strand(const Executor& ex,
    typename enable_if<
      is_executor<Executor>::value || execution::is_executor<Executor>::value
    >::type* = 0)
{
  return strand<Executor>(ex);
}

/// Create a @ref strand object for an execution context.
template <typename ExecutionContext>
inline strand<typename ExecutionContext::executor_type>
make_strand(ExecutionContext& ctx,
    typename enable_if<
      is_convertible<ExecutionContext&, execution_context&>::value
    >::type* = 0)
{
  return strand<typename ExecutionContext::executor_type>(ctx.get_executor());
}

/*@}*/

#if !defined(GENERATING_DOCUMENTATION)

namespace traits {

#if !defined(ASIO_HAS_DEDUCED_EQUALITY_COMPARABLE_TRAIT)

template <typename Executor>
struct equality_comparable<strand<Executor> >
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept = true);
};

#endif // !defined(ASIO_HAS_DEDUCED_EQUALITY_COMPARABLE_TRAIT)

#if !defined(ASIO_HAS_DEDUCED_EXECUTE_MEMBER_TRAIT)

template <typename Executor, typename Function>
struct execute_member<strand<Executor>, Function,
    typename enable_if<
      execution::can_execute<const Executor&, Function>::value
    >::type>
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept = false);
  typedef void result_type;
};

#endif // !defined(ASIO_HAS_DEDUCED_EXECUTE_MEMBER_TRAIT)

#if !defined(ASIO_HAS_DEDUCED_QUERY_MEMBER_TRAIT)

template <typename Executor, typename Property>
struct query_member<strand<Executor>, Property,
    typename enable_if<
      can_query<const Executor&, Property>::value
    >::type>
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept =
      (is_nothrow_query<Executor, Property>::value));
  typedef typename query_result<Executor, Property>::type result_type;
};

#endif // !defined(ASIO_HAS_DEDUCED_QUERY_MEMBER_TRAIT)

#if !defined(ASIO_HAS_DEDUCED_REQUIRE_MEMBER_TRAIT)

template <typename Executor, typename Property>
struct require_member<strand<Executor>, Property,
    typename enable_if<
      can_require<const Executor&, Property>::value
    >::type>
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept =
      (is_nothrow_require<Executor, Property>::value));
  typedef strand<typename decay<
    typename require_result<Executor, Property>::type
      >::type> result_type;
};

#endif // !defined(ASIO_HAS_DEDUCED_REQUIRE_MEMBER_TRAIT)

#if !defined(ASIO_HAS_DEDUCED_PREFER_MEMBER_TRAIT)

template <typename Executor, typename Property>
struct prefer_member<strand<Executor>, Property,
    typename enable_if<
      can_prefer<const Executor&, Property>::value
    >::type>
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept =
      (is_nothrow_prefer<Executor, Property>::value));
  typedef strand<typename decay<
    typename prefer_result<Executor, Property>::type
      >::type> result_type;
};

#endif // !defined(ASIO_HAS_DEDUCED_PREFER_MEMBER_TRAIT)

} // namespace traits

#endif // !defined(GENERATING_DOCUMENTATION)

} // namespace asio

#include "asio/detail/pop_options.hpp"

// If both io_context.hpp and strand.hpp have been included, automatically
// include the header file needed for the io_context::strand class.
#if !defined(ASIO_NO_EXTENSIONS)
# if defined(ASIO_IO_CONTEXT_HPP)
#  include "asio/io_context_strand.hpp"
# endif // defined(ASIO_IO_CONTEXT_HPP)
#endif // !defined(ASIO_NO_EXTENSIONS)

#endif // ASIO_STRAND_HPP
