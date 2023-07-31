/**
 * -- troll --
 * 
 * Copyright (c) 2023 dearoneesama
 * 
 * This software is licensed under MIT License.
 */

#pragma once

#include <etl/to_arithmetic.h>
#include "format.hpp"

namespace troll {

  constexpr bool is_digit(char c) noexcept {
    return c >= '0' && c <= '9';
  }

  constexpr bool is_white_space(char c) noexcept {
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
  }

  constexpr bool is_non_white_space(char c) noexcept {
    return !is_white_space(c);
  }

  template<class F>
  constexpr size_t eat_while(const char *test, size_t test_len, F &&f) noexcept {
    size_t i = 0;
    while (test_len-- && test[i]) {
      if (!f(test[i])) {
        break;
      }
      ++i;
    }
    return i;
  }

  static constexpr auto sscan_eats_white_space = true;

  struct unsupported_from_string_type {};

  template<class T>
  struct from_stringer {
    using TT = std::conditional_t<std::is_pointer_v<T>, T, T &>;
    unsupported_from_string_type operator()(::etl::string_view, TT) const;
  };

  struct sscan_impl_ret {
    bool success;
    size_t test_remain {};
  };

  template<bool Prefix>
  constexpr inline sscan_impl_ret sscan_impl(const char *test, size_t test_len, const char *format) noexcept {
    // strncmp
    while (*format && test_len) {
      if constexpr (sscan_eats_white_space) {
        if (is_white_space(*format) || is_white_space(*test)) {
          size_t len = eat_while(test, test_len, is_white_space);
          test += len;
          test_len -= len;
          format += eat_while(format, -1, is_white_space);
          continue;
        }
      }
      if (*format++ != *test++) {
        return {false};
      }
      --test_len;
    }

    if constexpr (sscan_eats_white_space) {
      format += eat_while(format, -1, is_white_space);
      test_len -= eat_while(test, test_len, is_white_space);
    }

    if constexpr (Prefix) {
      return {!*format, test_len};
    } else {
      return {!*format && !test_len};
    }
  }

  template<bool Prefix, class Arg0, class ...Args>
  constexpr inline sscan_impl_ret sscan_impl(const char *test, size_t test_len, const char *format, Arg0 &arg0, Args &...args) noexcept {
    while (*format && test_len) {
      if constexpr (sscan_eats_white_space) {
        if (is_white_space(*format) || is_white_space(*test)) {
          size_t len = eat_while(test, test_len, is_white_space);
          test += len;
          test_len -= len;
          format += eat_while(format, -1, is_white_space);
          continue;
        }
      }

      if (*format == '{' && format[1] == '}') {
        using Decay = std::decay_t<Arg0>;
        constexpr bool custom = !std::is_same_v<decltype(from_stringer<Decay>{}(std::declval<::etl::string_view>(), arg0)), unsupported_from_string_type>;
        if constexpr (custom) {
          // custom type
          if (size_t i = from_stringer<Decay>{}(::etl::string_view(test, test_len), arg0)) {
            return sscan_impl<Prefix>(test + i, test_len - i, format + 2, args...);
          } else {
            return {false};
          }
        } else if constexpr (std::is_integral_v<Decay> && !std::is_same_v<Decay, char> && !std::is_same_v<Decay, unsigned char>) {
          // int, uint, ...
          size_t i{};
          if (std::is_signed_v<Decay> && test_len && *test == '-') {
            i = 1 + eat_while(test + 1, test_len - 1, is_digit);
          } else {
            i = eat_while(test, test_len, is_digit);
          }
          auto result = ::etl::to_arithmetic<Decay>(::etl::string_view(test, i));
          if (result) {
            arg0 = result.value();
            return sscan_impl<Prefix>(test + i, test_len - i, format + 2, args...);
          } else {
            return {false};
          }
        } else if constexpr (std::is_same_v<Decay, char> || std::is_same_v<Decay, unsigned char>) {
          // char
          if (test_len) {
            arg0 = *test++;
            return sscan_impl<Prefix>(test + 1, test_len - 1, format + 2, args...);
          } else {
            return {false};
          }
        } else if constexpr (std::is_pointer_v<Decay> && std::is_same_v<std::remove_pointer_t<Decay>, char>) {
          // char *
          size_t i = eat_while(test, test_len, is_non_white_space);
          if (i) {
            // if arg0 is array reference, then its size is known
            using NoRef = std::remove_reference_t<Arg0>;
            size_t safe = i;
            if constexpr (std::is_array_v<NoRef>) {
              safe = safe < (std::extent_v<NoRef> - 1) ? safe : (std::extent_v<NoRef> - 1);
              arg0[safe] = '\0';
            }
            for (size_t n = 0; n < safe; ++n) {
              arg0[n] = test[n];
            }
            return sscan_impl<Prefix>(test + i, test_len - i, format + 2, args...);
          } else {
            return {false};
          }
        } else if constexpr (is_etl_string<Decay>::value) {
          // ::etl::string
          size_t i = eat_while(test, test_len, is_non_white_space);
          if (i) {
            arg0.assign(test, i);
            return sscan_impl<Prefix>(test + i, test_len - i, format + 2, args...);
          } else {
            return {false};
          }
        } else if constexpr (std::is_floating_point_v<Decay>) {
          // note: floats can only be parsed when it is not surrounded by other nws characters
          size_t i = eat_while(test, test_len, is_non_white_space);
          if (i) {
            auto result = ::etl::to_arithmetic<Decay>(::etl::string_view(test, i));
            if (result) {
              arg0 = result.value();
              return sscan_impl<Prefix>(test + i, test_len - i, format + 2, args...);
            } else {
              return {false};
            }
          } else {
            return {false};
          }
        } else {
          static_assert(custom, "unsupported type");
        }
      }
      if (*format++ != *test++) {
        return {false};
      }
      --test_len;
    }

    if constexpr (sscan_eats_white_space) {
      format += eat_while(format, -1, is_white_space);
      test_len -= eat_while(test, test_len, is_white_space);
    }

    if constexpr (Prefix) {
      return {!*format, test_len};
    } else {
      return {!*format && !test_len};
    }
  }

  /**
   * Checks the input string (test) compares the same as the format string, and writes down
   * matched values to variable references.
   * 
   * Returns true if the input string matches the format string, false otherwise.
   */
  template<class ...Args>
  constexpr inline bool sscan(const char *test, size_t test_len, const char *format, Args &...args) noexcept {
    return sscan_impl<false>(test, test_len, format, args...).success;
  }

  // this overload is not useful in practice
  /*template<size_t N, class ...Args>
  constexpr inline bool sscan(const char (&test)[N], const char *format, Args &...args) noexcept {
    return sscan(test, N - 1, format, args...);
  }*/

  /**
   * Overload for the case where the size can be obtained automatically.
  */
  template<class ...Args>
  constexpr inline bool sscan(::etl::string_view test, const char *format, Args &...args) noexcept {
    return sscan(test.data(), test.size(), format, args...);
  }

  /**
   * Checks the format string is a prefix of the input string (test), and writes down matched
   * values to variable references.
   * 
   * Returns a truthy value if the test strings matches and 0 otherwise. The value is the number
   * of consumed characters.
   */
  template<class ...Args>
  constexpr inline size_t sscan_prefix(const char *test, size_t test_len, const char *format, Args &...args) noexcept {
    auto result = sscan_impl<true>(test, test_len, format, args...);
    return result.success ? test_len - result.test_remain : 0;
  }

  /**
   * Overload for the case where the size can be obtained automatically.
  */
  template<class ...Args>
  constexpr inline size_t sscan_prefix(::etl::string_view test, const char *format, Args &...args) noexcept {
    return sscan_prefix(test.data(), test.size(), format, args...);
  }

}  // namespace troll
