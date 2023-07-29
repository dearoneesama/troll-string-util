/**
 * -- troll --
 * 
 * Copyright (c) 2023 dearoneesama
 * 
 * This software is licensed under MIT License.
 */

#include <catch2/catch_test_macros.hpp>
#include <etl/string_view.h>

#include <troll_util/format_scan.hpp>

namespace {
  struct test_type {
    int x;
    char s[10];
  };
}

template<>
struct troll::from_stringer<test_type> {
  size_t operator()(etl::string_view s, test_type &td) const {
    // defect: s={}) not supported
    return sscan_prefix(s, "td( x={}, s={} )", td.x, td.s);
  }
};

TEST_CASE("sscan usage", "[sscan]") {
  REQUIRE(troll::sscan("abcde", "abcde"));
  REQUIRE(!troll::sscan("xbcde", "abcde"));
  REQUIRE(!troll::sscan("abcdx", "abcde"));
  REQUIRE(!troll::sscan("abcdef", "abcde"));
  REQUIRE(!troll::sscan("abcdef", "abcdefg"));
  REQUIRE(!troll::sscan("", "abcde"));

  SECTION("only one {}") {
    int i = 0;
    REQUIRE(troll::sscan("129", "{}", i));
    REQUIRE(i == 129);
    REQUIRE(troll::sscan("-9", "{}", i));
    REQUIRE(i == -9);
    REQUIRE(troll::sscan("0", "{}", i));
    REQUIRE(i == 0);

    char c = 0;
    REQUIRE(troll::sscan("c", "{}", c));
    REQUIRE(c == 'c');

    char s[5];
    REQUIRE(troll::sscan("abcd", "{}", s));
    REQUIRE(etl::string_view{s} == "abcd");

    etl::string<5> es;
    REQUIRE(troll::sscan("abcd", "{}", es));
    REQUIRE(es == "abcd");

    float f = 0;
    REQUIRE(troll::sscan("1.23", "{}", f));
    REQUIRE(f == 1.23f);
  }

  SECTION("matching") {
    unsigned a = 0, b = 0;
    REQUIRE(troll::sscan("tr 123 456", "tr {} {}", a, b));
    REQUIRE(a == 123);
    REQUIRE(b == 456);

    REQUIRE(!troll::sscan("s 123 456", "tr {} {}", a, b));
    REQUIRE(!troll::sscan("tt 123 456", "tr {} {}", a, b));
    REQUIRE(!troll::sscan("tr 123 456", "tr {} {} {}", a, b));

    int i = 0, j = 0;
    char buf[10];
    char (*sub)[5] = reinterpret_cast<char(*)[5]>(reinterpret_cast<char *>(&buf));
    REQUIRE(troll::sscan("tr 123 aaaabbbcc 176 end", "tr {} {} {} end", i, *sub, j));
    REQUIRE(i == 123);
    REQUIRE(etl::string_view{buf} == "aaaa");
    REQUIRE(j == 176);

    REQUIRE(!troll::sscan("tr 123 17 aaaabbbcc 176 end", "tr {} {} {} end", i, *sub, j));
    REQUIRE(!troll::sscan("tr 123 17 aaaabbbcc 176 end", "tr {} {} {} x {} {} end", i, *sub, j));
  }

  if constexpr (troll::sscan_eats_white_space) {
    REQUIRE(troll::sscan("ab cde", "ab  cde"));
    REQUIRE(troll::sscan("ab cde  ", "ab  cde"));
    REQUIRE(troll::sscan("ab cde ", "ab  cde   "));
    REQUIRE(troll::sscan("ab cde  ", " ab  cde"));
    REQUIRE(!troll::sscan("ab cde  xz ", "ab  cde"));
    REQUIRE(!troll::sscan("ab cde  xz ", "ab  cde  xz yui"));

    SECTION("matching") {
      int i = 0;
      float f = 0;
      REQUIRE(troll::sscan(" tr -123  456 ", "tr {} {}", i, f));
      REQUIRE(i == -123);
      REQUIRE(f == 456.0f);

      REQUIRE(!troll::sscan(" tr -123  456 extra", "tr {} {}", i, f));
      REQUIRE(!troll::sscan(" tr -123  456 ", "tr {} {} extra", i, f));
    }
  }

  SECTION("matching prefix") {
    REQUIRE(troll::sscan_prefix("tr 123_456after", "tr 123") == 6);

    unsigned a = 0, b = 0;
    REQUIRE(troll::sscan_prefix("tr 123 456after", "tr {} {}", a, b) == 10);
    REQUIRE(b == 456);
    REQUIRE(troll::sscan_prefix("tr 123 456 after", "tr 123 {} af", b) == 13);
    REQUIRE(!troll::sscan_prefix("tr 123 456 after", "tr 123 {}x af", b));
  }

  SECTION("custom type") {
    test_type td1, td2;
    REQUIRE(troll::sscan("td( x=16, s=abcde ) and td( x=-9, s=96A )", "{} and {}", td1, td2));
    REQUIRE(td1.x == 16);
    REQUIRE(etl::string_view{td1.s} == "abcde");
    REQUIRE(td2.x == -9);
    REQUIRE(etl::string_view{td2.s} == "96A");

    REQUIRE(troll::sscan("start td( x=16, s=abcde ) td( x=-9, s=96A ) done", "start {} {} done", td1, td2));
  }
}
