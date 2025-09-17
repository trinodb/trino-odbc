## Before getting started

Thank you for considering a contribution to the trino-odbc driver
project! This project is released to the open source community so
people like you can help make it better. Let's work together to
expand the Trino ecosystem! We welcome all types of contributions
including but not limited to:

* Documentation
* Bug reports
* Tests
* Automation
* Code Style Enforcement
* New Features
* Performance Improvements
* Code Refactors

We recognize that there are many use-cases for an ODBC driver, and
that the driver in this repository is incomplete and may only
address a subset of your requirements at this time. We welcome
extensions to the driver to more completely implement that ODBC
specification. We are happy to accept pull requests to bridge
feature gaps and make this better.  Any contribution, no matter
how small, is welcome.

# How to contribute

## Contributor License Agreement ("CLA")

In order to accept your pull request, we need you to [submit a CLA](https://github.com/trinodb/cla).

## License

By contributing to Trino, you agree that your contributions will be licensed under the [Apache License Version 2.0 (APLv2)](../LICENSE).

## Contribution process

See the suggested [process for contributions](https://trino.io/development/process.html).

## Getting started

To get started, you'll want to follow the instructions in the
[main README](./README.md) for Developer Prerequisites, as well
as for Testing and Debugging. This should get you a working
environment to compile the driver and test it with any
ODBC compatible application. Once you have compiled the driver,
you can follow the instructions about how to efficiently identify
specific limitations in the compiled driver. Once you know what
the limitations are, it shouldn't be too difficult to add the
missing feature(s) or fix the existing implementation(s). If
you're stuck at any point, include your findings in your existing
issue so somebody can take a look and provide some guidance.

We made extensive use of the TRACE-level logging option to develop
this driver. Turning that on, along with driver tracing in the
Windows ODBC driver manager usually provides enough diagnostic
logs to tell which source code file needs to be implemented or extended.

At some point, you'll want your change to show up in a release.
Pushing tags that follow semantic versioning create releases for
this project. At this time, our maintainance team intends to manually review
any contributions and create tags for releases on an as-needed basis.

## Coding Conventions

We have taken an opinionated stance on the C++ coding style in this
repo.


### General Style

We tried to optimize the code style for readability by non-expert
C++ programmers, which means it may seem non-idomataic or inefficient
to an expert. This design decision was intentional. Among
the maintenance team on this project, we have many more python
experts than C++ experts, and this style makes it easier for us
to understand and maintain the code. Here is an incomplete list
of potentially non-idiomatic C++ code style that was intentionally
adopted for this codebase:

* Avoid member initializer lists in constructors where possible in
  favor of default constructors and manual member assignment.
    ```cpp
    // Not this
    Height(int n) : number(n) {}

    // This!
    Height(int n) {
      this->number = n;
    }
    ```


* Include unnecessary curly-braces in if blocks, case statements, and loops.
    ```cpp
     // Not this
     if (condition)
       do_thing();

     // This!
     if (condition) {
       do_thing();
     }
    ```
* Avoid using typedefs to alias types. This forms a mini
  domain-specific-language within the a C++ codebase. Instead, write
  out the full type if required, and if possible consider
  using the `auto` keyword to indicate an inferred type without
  creating an explicit aliased type.
  ```cpp
  // Not this
  typedef std::map<std::string, std::vector<int>> StringToIntVectorMap;
  StringToIntVectorMap::iterator it = myMap.begin();

  // This!
  auto it = myMap.begin();
  ```

* Avoid adding const/static/&-reference operator unless there is a
  demonstrable need, for example in code or structures that interact
  with tight-loops for data retrieval or parsing. We acknowledge there
  is sometimes a performance penalty for this, but it improves
  the ratio of logic to boilerplate in the codebase, which in turn
  makes it easier to maintain.
  ```cpp
  // Not this
  const std::vector<int>& getNumbers() {
      static const std::vector<int> numbers = {1, 2, 3, 4, 5};
      return numbers;
  }

  // This!
  std::vector<int> getNumbers() {
      return {1, 2, 3, 4, 5};
  }
  ```
* Prefer the use of `static_cast<type>` and
  `reinterpret_cast<type>` over c-style casting `(type)value`
  for clarity and safety, as well as to enable high quality c++
  type checking.
  ```cpp
  // Not this
  void* rawPointer = ...;
  int* intPointer = (int*)rawPointer; // C-style cast

  // This!
  void* rawPointer = ...;
  int* intPointer = reinterpret_cast<int*>(rawPointer); // C++ style cast
  ```

The exception to these style guidelines (in particular the &-reference one)
is cases where implementing this style confers a demonstrable,
performance regression that is shown in a profiler to meaningfully
affect everyday use of the driver. In many cases, an optimizing compiler
produces performant code even without the boilerplate. However,
when profiling shows the compiler needs help, trading off readability
for performance is easy to justify.

### Automated Formatting

Code formatting in this repo is enforced by clang-format wherever possible.
We use a code editor setting to remove trailing whitespace on every line in
all cpp, hpp, yml, and md files. Please do the same prior to submitting a code or
documentation contribution.

### Variable Naming Convention

* Member Variables: Use `this->` where possible to refer to member variables instead
  of using the more common `m_` prefix.
* Casing: Use camelCase variable names and CapitalCase class names where possible.
  This seems more common in C++ codebases targeting Windows environments
  and roughly matches the odbc header file conventions.

### Directories

Code is organized into five main directories with many logical subdirectories.

* `src/driver` - Main driver code to implement the ODBC API spec.
* `src/driver/config` - Implements the ODBC Driver Manager configuration spec
* `src/trinoAPIWrapper` - Implements a C++ wrapper for Trino's REST API includin
  an auth layer. This should NOT include any ODBC specific header files like sql.h
  or sqlext.h and should avoid including Windows.h as much as is possible.
* `src/util` - General utilities used by the driver or trinoAPIWrapper that don't belong
  within those places because the functionality is generic.
* `test` - A GoogleTest unit test suite for the driver.
