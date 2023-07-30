# Header `format.hpp`

## Free functions

### `<class ...Args> size_t snformat(char *dest, size_t destlen, const char *format, const Args &...args)`

Formats the string into the buffer and returns the length of the result string.

This function _will_ output the nul terminator (`\0`), however it is not included in the return value.

This function does not overflow the buffer.

### `<size_t N, class ...Args> size_t snformat(char (&dest)[N], const char *format, const Args &...args)`

The overload is for the case where the buffer size can be automatically deduced if the destination is an array.

### `<size_t N, class ...Args> ::etl::string<N> sformat(const char *format, const Args &...args)`

Formats the string into a new string instance with specified capacity template parameter. Same as [`::etl::string`](https://www.etlcpp.com/string.html), the N will not include the nul terminator.

### `<class ...Args> size_t sformat(::etl::istring &dest, const char *format, const Args &...args)`

Formats the string into an existing string instance and returns the result length, which excludes the nul terminator. The string itself's capacity is used.

<hr />

The string formatting utility helps create more structured outputs without cumbersome manual calculating work when library utilities such as `snprintf` and `std::ostringstream` are not available. Similarly to C++/20's format, it treats occurrences of `{}` in the format string as placeholders for the arguments which are provided later in the function call, for example

```cpp
#include <format.hpp>
using namespace troll;

char buffer[50];
snformat(buffer, "Hello {}!", "World");
puts(buffer);
// Hello World!
```

For this use case, one can condense such string creation into one line by using etl's string type:

```cpp
int number_of_worlds = 16;
::etl::string<50> s = sformat<50>("Hello {} {}s!", number_of_worlds, "world");
puts(s.c_str());
// Hello 16 worlds!
```

The template parameter will correspond to the maximum allowable string size for the buffer.

To print custom types, one will need to specialize the `to_stringer` class template with the formatting functionality, otherwise compilation error may occur.

```cpp
struct point {
  int x, y;
};

namespace troll {
  template<>
  struct to_stringer<point> {
    void operator()(const point &p, ::etl::istring &s) const {
      sformat(s, "(x={}, y={})", p.x, p.y);
    }
  };
}
```

The implementation receives the object reference as the first parameter, and the second parameter is the buffer to write characters to. After this, the point type can be formatted:

```cpp
point p1 {16, 1}, p2 {-10, 0};
auto s = sformat<50>("{} + {} = {}", p1, p2, point{p1.x + p2.x, p1.y + p2.y});
puts(s.c_str());
// (x=16, y=1) + (x=-10, y=0) = (x=6, y=1)
```

<hr />

### `void pad(char *dest, size_t dest_pad_len, const char *src, size_t srclen, padding p, char padchar = ' ')`

Pads the string to the specified length. The function writes to every character in the dest buffer with the source string and pad characters given the dest_pad_len. However, it does not output the terminating nul.

This function operates on the byte level, which means ANSI codes (colors) are not supported.

### `<size_t DestPadLen, size_t SrcLen> void pad(char (&dest)[DestPadLen], const char (&src)[SrcLen], padding p, char padchar = ' ')`

The padding length is deduced from the destination array size. which is DestPadLen - 1. This overload writes to every character in the dest buffer except the last one, which is always set to nul.

### `<size_t DestPadLen> ::etl::string<DestPadLen> pad(::etl::string_view src, padding p, char padchar = ' ')`

Pads the source string with DestPadLen characters and returns a new string instance.

### `void pad(::etl::istring &dest, size_t dest_pad_len, ::etl::string_view src, padding p, char padchar = ' ')`

Pads the source string with dest_pad_len characters, or the destination capacity if the former is larger, into an existing string.

<hr />

It is recommended to use the string variants as they are simpler and one does not need to worry about nul terminators.

```cpp
::etl::string<4> my_string = "test";
auto padded = pad<10>(my_string, padding::middle);
```

This would result in a string of length 10 (without the nul),

```
"   test   "
```
with padding middle (other options are left and right) and the default padding character (space).

<hr />

## `enum class ansi_font`

Members: none, bold, dim, italic, underline, blink, reverse, hidden, strikethrough.

## `enum class ansi_color`

Members: none, black, red, green, yellow, blue, magenta, cyan, white.

## `<ansi_font Font = ansi_font::none, ansi_color FgColor = ansi_color::none, ansi_color BgColor = ansi_color::none> class static_ansi_style_options`

A compile-time object that holds ANSI style options and handles the work for escape strings.

### `ansi_font font`

The `ansi_front` enum class creates the style of the text.

### `ansi_color fg_color`

The `ansi_color` enum class creates the style of the foreground.

### `ansi_color bg_color`

The `ansi_color` enum class creates the style of the background.

### `size_t enabler_str_size`
### `::etl::string_view enabler_str()`

The string of ANSI escape characters used to start the style.

### `size_t disabler_str_size`
### `::etl::string_view disabler_str()`

The string of ANSI escape characters used to remove styles.

### `size_t wrapper_str_size`

The number of extra characters needed to wrap any string with the style.

<hr />

To create a string with styles which can be recognized by the terminal, one would first create this style object, then use the enabler and disabler strings provided by the object to wrap any other strings to be styled.

```cpp
::etl::string<4> my_string = "test";

// first, create a style
using style = static_ansi_style_options<
  ansi_font::bold,  // font
  ansi_color::red,  // foreground
  ansi_color::yellow  // background
>;

auto padded = pad<10>(my_string, padding::middle, '-');
// style codes take up extra buffer size, so plus
auto styled = sformat<10 + style::wrapper_str_size>(
  "{}{}{}", style::enabler_str(), padded, style::disabler_str()
);
puts(styled.c_str());
```

In this example, a static style is created. First the test string is padded with `'-'` to take up 10 places, then the style options are applied to the whole string by a simple concatenation.

The wrapper string size is computed at compile time, so we will pass that as well to the template parameter of `sformat` to guarantee the size of the result string.

If we run it in a supported console, we will see

<pre><code><span style="background:yellow;color:red;font-weight:bold">---test---</span></code></pre>

In contrast to the previous example:
```cpp
auto styled = sformat<4 + style::wrapper_str_size>(
  "{}{}{}", style::enabler_str(), my_string, style::disabler_str()
);
auto padded = pad<10 + style::wrapper_str_size>(styled, padding::middle, '-');
puts(styled.c_str());
```

the `pad` function does not know about escape codes. hence to only style the text that matters, extra calculation is required. the result is still a 10-column wide string, but yellow background is only applied to the text in the middle:

<pre><code>---<span style="background:yellow;color:red;font-weight:bold">test</span>---</code></pre>

A helper type `static_ansi_style_options_none_t` is also defined to be `static_ansi_style_options<>`, i.e. no styles. In that case no wrapper string overhead is created.

<hr />

## `<class Heading, class TitleIt, class HeadingStyle, class TitleStyle> struct tabulate_title_row_args`

A helper class to pass arguments for title rows to table builder.

### `bool style_is_same`

Whether the leftmost (heading) column uses the same style as the columns on the right.

### `Heading heading`

The leftmost (heading) column used for the title row.

### `TitleIt begin, end`

Range for titles (names).

### `<class Hding, class It1, class It2> tabulate_title_row_args(Hding &&heading, It1 &&begin, It2 &&end, HeadingStyle, TitleStyle)`

Apply different styles on the leftmost column (heading column) and columns on the right.

### `<class Hding, class It1, class It2> tabulate_title_row_args(Hding &&heading, It1 &&begin, It2 &&end, TitleStyle)`

Apply the same style on the leftmost column (heading column) and columns on the right.

### `<class It1, class It2> tabulate_title_row_args(It1 &&begin, It2 &&end, TitleStyle)`

Provide no heading column.

## `<class Heading, class ElemIt, class HeadingStyle, class ElemStyle> struct tabulate_elem_row_args`

A helper class to pass arguments for element rows to table builder.

### `bool style_is_same`

Whether the leftmost (heading) column uses the same style as the columns on the right.

### `Heading heading`

The leftmost (heading) column used for the title row.

### `TitleIt begin`

Range start for elements (names). An end is not required because it would be deduced from the title row.

### `<class Hding, class It> tabulate_elem_row_args(Hding &&heading, It &&begin, HeadingStyle, ElemStyle)`

Apply different styles on the leftmost column (heading column) and columns on the right.

### `<class Hding, class It> tabulate_elem_row_args(Hding &&heading, It &&begin, ElemStyle)`

Apply the same style on the leftmost column (heading column) and columns on the right.

### `<class It> tabulate_elem_row_args(It &&begin, ElemStyle)`

Provide no heading column.

## `<size_t ElemsPerRow, size_t HeadingPadding, size_t ContentPadding, class DividerStyle, class TitleRowArgs, class ...ElemRowArgs> class tabulate`

Helper class to tabulate text.

### `size_type num_elem_row_args`

The number of element rows.

### `size_type elems_per_row`

The maximum number of elements per row.

### `size_type heading_padding`

The fixed width of the leftmost (heading) column.

### `size_type content_padding`

The fixed width of the columns on the right (content column).

### `size_type max_line_width`

The maximum width of any row in the result.

Calculated based on the number of elements per row and formatting, excluding escapes.

### `char divider_horizontal`
### `char divider_vertical`
### `char divider_cross`

Characters for table dividers.

### `<class Tit, class ...Elems> tabulate(Tit &&title, Elems &&...elems)`

Constructor. To avoid passing excess template parameters, use `make_tabulate` instead.

### `class iterator`
### `iterator begin()`
### `iterator end()`

Single use iterator for getting the result. In general, an [`input iterator`](https://en.cppreference.com/w/cpp/named_req/InputIterator).

### `<class Tit1, class Tit2, class ...Elems> void reset_src_iterator(Tit1 &&title_begin, Tit2 &&title_end, Elems &&...elem_begins)`

Provide new ranges of data (title rows and element rows) and replaces the iterators already in the tabulate object, so that it can be iterated again to print another table.

### `<size_t ArgRow, class V> ::std::tuple<size_t, size_t, ::etl::string<...>> patch_str(size_t it_index, const V &v)`

Returns the column, row, and a string to be used to patch a already printed table if the value in it is supposed to change _(only modify)_.

If elements are added or deleted from the source table, use `reset_src_iterator` instead.

- ArgRow: ith row  as provided to `make_tabulate()`, eg. 0 -> title row, 1 -> first element row
- it_index: index of the value in that element row
- v: replacement value (type can be different)


### `<size_t ElemsPerRow, size_t HeadingPadding, size_t ContentPadding, class DividerStyle, class TitleRowArgs, class ...ElemRowArgs> tabulate<...> make_tabulate(DividerStyle, TitleRowArgs &&title, ElemRowArgs &&...elems)`

Helper function to make a tabulator when leftmost (heading) column have a different width than the right-hand-side columns.

Template arguments:
- ElemsPerRow: The maximum number of elements per row.
- HeadingPadding: The fixed width of the leftmost (heading) column.
- ContentPadding: The fixed width of the columns on the right (content column).

Runtime arguments:
- divider_style: an `static_ansi_style` instance
- title_row_args: an `tabulate_title_row_args` instance
- elem_row_args: an `tabulate_elem_row_args` instance

### `<size_t ElemsPerRow, size_t Padding, class DividerStyle, class TitleRowArgs, class ...ElemRowArgs> tabulate<...> make_tabulate(DividerStyle, TitleRowArgs &&title, ElemRowArgs &&...elems)`

Helper function to make a tabulator when all columns have the same width.

Template arguments:
- ElemsPerRow: The maximum number of elements per row.
- Padding: The fixed width of each column.

Runtime arguments:
- divider_style: an `static_ansi_style` instance
- title_row_args: an `tabulate_title_row_args` instance
- elem_row_args: an `tabulate_elem_row_args` instance

<hr />

The tabulate object takes in iterators that point to items and behaves as an iterable to output strings, line by line. It also supports passing the style options.

The terminology follows the diagram below:

```
+------+------------------------+
|      |                        | < title row (title iterator)
+------+------------------------+
|      |                        |
|      |                        | < element row (elem iterators)
|      |                        |
+------+------------------------+
   ^               ^
heading         content
column          columns
```

Rows are supplied as "row args", which contain the heading, contents and styles.

For example, if we want to print these:
```cpp
// they do not have to be char *'s or ints; anything that can
// be used in sformat will work
const char *cars[] = {"BMW", "Mercedes", "Audi", "VW", "Opel"};
int speeds[] = {200, 180, 190, 180, 160};
int brakes[] = {100, 90, 100, 90, 80};
```

The cars are title rows and the other two are elem rows. The following code will tabulate them

```cpp
using no_style = static_ansi_style_options<>;
using bold = static_ansi_style_options<ansi_font::bold>;
using yellow = static_ansi_style_options<ansi_font::none, ansi_color::yellow>;
using bold_and_yellow = static_ansi_style_options<ansi_font::bold, ansi_color::yellow>;

auto tab = make_tabulate<5, 11, 6>(
  // style for the +-|
  no_style{},
  // title row's heading, iterator begin & end, heading style, content style
  tabulate_title_row_args{"Car", cars, cars + 5, bold_and_yellow{}, yellow{}},
  // elem row's heading, iterator begin, heading style, content style
  tabulate_elem_row_args{"Speed", speeds, bold{}, no_style{}},
  tabulate_elem_row_args{"Brake", brakes, bold{}, no_style{}}
);

for (::etl::string_view s : tab) {
  puts(s.data());
}
```

The class will use the digits to calculate buffer sizes at compile time automatically. The result is:

<pre><code style="white-space:pre">+-----------------------------------------+
|    <span style="color:yellow"><span style="font-weight:bold">Car</span>     BMW    MB   Audi   VW   Opel</span> |
+-----------------------------------------+
|   <span style="font-weight:bold">Speed</span>    200   180   190   180   160  |
+-----------------------------------------+
|   <span style="font-weight:bold">Brake</span>    100    90   100    90    80  |
+-----------------------------------------+
</code></pre>

Note that the iterator will return a string view pointing to its underlying buffer, so they are invalidated after each iteration.

It is also possible to go without the heading. Here is an example:

```cpp
  ::etl::string<2> titles[] = {"0A", "1A", "2A", "3A", "4A", "0B", "1B", "2B", "3B", "4B"};
  ::etl::string<1> statuses[] = {"o", "o", "x", "x", "o", "x", "x", "x", "x", "?"};

  auto tab = make_tabulate<5, 6>(
    no_style{},
    tabulate_title_row_args{titles, titles + 10, no_style{}},
    tabulate_elem_row_args{statuses, no_style{}}
  );

  for (::etl::string_view s : tab) {
    puts(s.data());
  }
```

The output table is split, because each row can only contain 5 elements.

<pre><code style="white-space:pre">+------------------------------+
|  0A    1A    2A    3A    4A  |
+------------------------------+
|  o     o     x     x     o   |
+------------------------------+
|  0B    1B    2B    3B    4B  |
+------------------------------+
|  x     x     x     x     ?   |
+------------------------------+</code></pre>

The source iterators can be swapped after iterations to create a new table, essentially reusing the object:

```cpp
::etl::string<1> statuses2[] = {"o", "o", "o", "o", "o", "x", "o", "o", "x", "?"};
tab.reset_src_iterator(titles, titles + 9, statuses2);
//                                      ^
for (::etl::string_view s : tab) {
  puts(s.data());
}
```

Output:
<pre><code style="white-space:pre">+------------------------------+
|  0A    1A    2A    3A    4A  |
+------------------------------+
|  o     o     o     o     o   |
+------------------------------+
|  0B    1B    2B    3B        |
+------------------------------+
|  x     o     o     x         |
+------------------------------+</code></pre>

Here the title source is the same, but its end only goes past the ninth element, thus this table only lists nine elements, even we have not specified the end of statuses2.

If it is expensive to redraw a table and we do not grow or shrink the element lists, one can obtain a patching string along with a coordinate to update the table on the screen, once there is an individual update:

```cpp
// flipping 3B to "o"
auto [row, col, patch] = tab.patch_str<1>(8, "o");
// row: 7
// col: 19
// patch: "  o   "
```

The position is relative to the beginning of the drawn table. All needed is to write the patch string to that location.

<hr />

## `<size_t MaxLineWidth, size_t MaxLines, size_t MaxQueueSize = MaxLines> class OutputControl`

The class supports specifying text at a certain line and at a certain column.

It maintains an internal queue for texts, and when on demand, it will output a string with ansi escape codes which can be used in writing to a terminal.

The usage of this class means that it takes entire control of the terminal ui mostly if not all.

### `size_type max_line_width`

The maximum number of characters per line.

### `size_type max_lines`

The number of lines displayable on the terminal.

### `size_type max_queue_size`

The maximum number of lines queueable for output.

### `OutputControl()`

Constructor.

### `size_type enqueue(size_type line, size_type column, const char *text)`

Submit a text to be outputted at a certain line and column. returns the number of characters that were successfully enqueued.

- line: 0-based
- column: 0-based
- text: do not include ansi escape codes other than color etc. But notice that colors occupy character buffers. Text is null-terminated. If it is a nullptr, then it corresponds to clearing the line. The call will return 0.

### `::etl::string_view dequeue()`

Get a string ready to be outputted to a terminal (contains the original string wrapped with ANSI escape characters to move the cursor), or nullptr if there is none. The string returned is a reference to an internal buffer, and it will be invalid after the next call to this function.

### `bool empty()`

Queue is empty.

<hr />

Instance of this class is almost a "virtual screen" if every text writes go through this it. it is used like a queue. requests need to be enqueued:

```cpp
// max line buffer size and max number lines
OutputControl<100, 30> oc;

// submit requests
oc.enqueue(0, 0, "write here");
oc.enqueue(1, 10, "also write here");
oc.enqueue(2, 10, "and here");
oc.enqueue(2, 10, "over");
```

the dequeueing will convert user's strings into strings with ANSI escape codes used to relocate the cursor:

```cpp
while (!oc.empty()) {
  etl::string_view line = oc.dequeue();
  puts(line.c_str());
}
```
The terminal should display
```
write here
          also write here
          overhere
```

It is recommended to use this along with `pad` so that the width of any text is determined, hence easier UI.
