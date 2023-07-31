# Header `format_scan`

## Free functions

### `<class ...Args> bool sscan(const char *test, size_t test_len, const char *format, Args &...args)`

Checks the input string (test) compares the same as the format string, and writes down matched values to variable references.

Returns true if the input string matches the format string, false otherwise.

### `<class ...Args> bool sscan(::etl::string_view test, const char *format, Args &...args)`

Overload for the case where the size can be obtained automatically.

### `<class ...Args> size_t sscan_prefix(const char *test, size_t test_len, const char *format, Args &...args)`

Checks the format string is a prefix of the input string (test), and writes down matched values to variable references.

Returns a truthy value if the test strings matches and 0 otherwise. The value is the number of consumed characters.

### `<class ...Args> size_t sscan_prefix(::etl::string_view test, const char *format, Args &...args)`

Overload for the case where the size can be obtained automatically.

<hr />

The `sscan` family of functions behaves as the reverse of `sformat`. If the format string contains `{}`, then it is used as a placeholder for matching the test string against the variable reference later in the argument list. The call will extract the matched value in the string and write it to the variable.

```cpp
#include <troll_util/format_scan.hpp>
using namespace troll;

etl::string<20> test_string = "set 16 S!";
int i = 0;
char j = 0;
bool success = sscan(test_string, "set {} {}!", i, j);
```

The test string here matches the format string, so the call returns true, and variables i and j are `16` and `'S'`, respectively.

The prefix variant will not compare the whole string, but only make sure that a _prefix_ of the test string matches the format string. It can be used to ignore additional inputs, or in the case of custom types:

```cpp
struct point {
  int x, y;
};

namespace troll {
  template<>
  struct from_stringer<point> {
    size_t operator()(etl::string_view s, point &p) const {
      // at this point there may be extra stuff after parsing point
      return sscan_prefix(s, "(x={}, y={})", p.x, p.y);
    }
  };
}

// read the object from test string
etl::string<20> test_string = "set 16 (x=1, y=-1)!";
int i = 0;
point p;
bool success = sscan(test_string, "set {} {}!", i, p);
// now: i = 16, p = point{1, -1}
```

User needs to specialize the `from_stringer` class template to provide the parsing functionality for the custom type. The implementation receives the remaining string to parse as the first argument and the object to fill in as the second argument, and returns the number of characters consumed to match this object.

There are limitations of the scanning, though, due to its simple implementation. For example, matching string `"strings a"` against the format `"{}s a"` will not work by default if the `{}` corresponds to a string type. Here the prefix `"strings"` is eagerly parsed to match the placeholder (where whitespaces are treated as delimiters by default in this case), therefore the remaining `a` cannot match. To parse more complicated expressions like this, the user will need to implement parsing on their own, or use a proper parsing library such as regex.
