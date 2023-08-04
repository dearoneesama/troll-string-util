# troll-string-util

[![License: MIT](https://img.shields.io/badge/License-MIT-blue?style=for-the-badge)](https://github.com/dearoneesama/troll-string-util/blob/main/LICENSE)
[![GitHub](https://img.shields.io/badge/GitHub-8A2BE2?style=for-the-badge&logo=github)](https://github.com/dearoneesama/troll-string-util)

This project contains string formatting utilities for embedded applications which use fixed-size strings and where exceptions, run-time type info (RTTI) and dynamic memory allocations are unavailable. It relies on and complements the library [etl](https://github.com/ETLCPP/etl). It requires C++17 at minimum.

Some useful functionalities for displaying that `troll` provides include:
* Substituting values into string's placeholders (format string)
* Console ANSI style code insertions
* Simple tabulations

Utilities here in this project originated as helper modules from my taking of the course "CS 452 - Real-time Programming" at University of Waterloo, in which participants were asked to complete a real-time OS throughout the term.

## Usage
Drop the header files or templates you like into your project, or include as your submodule. Note that you need to enable `-std=c++17` or similar for them to work.

Please see following pages for the documentation:

* [`format`](https://dearoneesama.github.io/troll-string-util/docs/format.html)
* [`format_scan`](https://dearoneesama.github.io/troll-string-util/docs/format_scan.html)
* [`utils`](https://dearoneesama.github.io/troll-string-util/docs/utils.html)

## Testing
The CMake files exist primarily for running the test suites. Here are example commands to run them from the command line:

```bash
cmake -DBUILD_TESTS=ON -B./build
cmake --build ./build --target troll_util_tests
./build/troll_util_tests 
```
