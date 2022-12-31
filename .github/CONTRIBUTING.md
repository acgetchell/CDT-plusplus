## How to Contribute

First, thank you!

Writing esoteric scientific software can be its own reward, but it's not for the faint of heart.

If you want a general overview as to why this software package exists, please look at the [Wiki] or my talk
[Causal Dynamical Triangulations with CGAL][slides].

Second, here are some simple guidelines that will make it easier on me to process and accept your contributions.

1. Fork the repository.

2. This project uses [GitFlow]. That is, new [features] are branched from/merged into [develop].
New [releases] are periodically made from [develop], then merged back into [master],
which is the stable work history. [Tagged] versions are [releases] at a point in time, citable via [ORCID].
for reproducibility.

3. Familiarize yourself with [doctest] and the [Gherkin] syntax.

4. Write a unit test for your proposed contribution. Unit tests go in the `tests` directory and are named
\{YourContribution\}_test.cpp, don't forget to add to `/tests/CMakeLists.txt`.
All proposed features of your contribution should have a corresponding test in \{YourContribution\}_test.cpp.
Consult the [doctest test cases] if you are unsure, or consult existing tests for examples.

5. I highly recommend writing your tests first, before your contribution, as this helps to think about how the
rest of the program will use your functions and/or classes.
[Test-Driven Development] (and [BDD]) has saved me quite a bit from various mistakes.

6. Project source files go into the `src` directory; header files go into `include`.
This makes integration into various tests and the main program easy and modular, and follows convention.

7. Don't forget documentation! It's helpful if you state explicitly what your functions and classes do.
I use [Doxygen] to automatically build documentation, so using `/// @brief` and `/// @param` is helpful and easy.
Consult existing code for examples.

8. Commit your changes with a clear, [well-written commit message].

9. Check your whitespace with `git diff --check HEAD^`.

10. Run `clang-format` using the project's [.clang-format].

11. Run `clang-tidy` using the project's [clang-tidy.sh].

12. Open a pull request against the develop branch of the main repository (which is the default).
[Travis-CI] will test it against combinations of Linux (Ubuntu 22.04) with clang and gcc. [GitHub Actions] will test
against macOS and run various other checks.
[AppVeyor] will test it against Visual Studio 2019 with `clang-cl` (version 14.0.6). Ensure that
your code compiles on Windows, macOS, and Linux with `msvc`, `gcc`, and `clang`.

13. All pull requests must pass [Travis-CI] and [AppVeyor] to be accepted.
In particular, look at results from [Cppcheck], [Valgrind], [ASAN], [LSAN], [MSAN], and [TSAN], because simulations may
run for a long time so memory leaks will be eventually fatal.
[GitHub Actions] also has a lot of useful checks that will help fix your code.

14. I will get to your change as soon as I can.
Feel free to ping me on [Gitter] with any questions.
You will receive proper credit for your contributions both in the code and any resulting scientific papers
using the output of `git log --format='%aN | sort -u`.

## Style Guide

This project generally follows [Stroustrup formatting with Allman brackets][1], enforced by [ClangFormat].
The [C++ Core Guidelines][cpp-core] are checked using [ClangTidy].
Running [clang-tidy.sh] changes the code (using `-fix`), so compare results and run unit tests before you commit.
Especially since some of ClangTidy's fixes break the codebase (the script runs just the tests that don't).

Most editors/IDEs have plugins for `clang-format` and `clang-tidy`.

[Wiki]: https://github.com/acgetchell/CDT-plusplus/wiki
[Test-Driven Development]: http://alexott.net/en/cpp/CppTestingIntro.html
[Doxygen]: http://doxygen.org
[well-written commit message]: https://chris.beams.io/posts/git-commit/
[Travis-CI]: https://travis-ci.org/acgetchell/CDT-plusplus
[1]: https://isocpp.org/wiki/faq/coding-standards
[2]: http://llvm.org/releases/4.0.0/tools/clang/docs/ClangFormatStyleOptions.html
[ClangFormat]: https://releases.llvm.org/6.0.1/tools/clang/docs/ClangFormat.html
[slides]: http://slides.com/acgetchell/causal-dynamical-triangulations-3
[Valgrind]: http://valgrind.org/docs/manual/quick-start.html#quick-start.mcrun
[cpp-core]: https://github.com/isocpp/CppCoreGuidelines/blob/master/CppCoreGuidelines.md
[clang-tidy.sh]: https://github.com/acgetchell/CDT-plusplus/blob/develop/clang-tidy.sh
[AppVeyor]: https://ci.appveyor.com/project/acgetchell/cdt-plusplus
[doctest]: https://github.com/doctest/doctest
[Gherkin]: https://www.tutorialspoint.com/behavior_driven_development/behavior_driven_development_gherkin.htm
[BDD]: https://en.wikipedia.org/wiki/Behavior-driven_development
[doctest test cases]: https://github.com/doctest/doctest/blob/master/doc/markdown/testcases.md
[Gitter]: https://gitter.im/acgetchell/CDT-plusplus
[ClangTidy]: https://releases.llvm.org/6.0.1/tools/clang/tools/extra/docs/clang-tidy/index.html
[LGTM]: https://lgtm.com/projects/g/acgetchell/CDT-plusplus/
[GitFlow]: https://leanpub.com/git-flow/read
[features]: https://leanpub.com/git-flow/read#leanpub-auto-feature-branches
[develop]: https://github.com/acgetchell/CDT-plusplus
[releases]: https://github.com/acgetchell/CDT-plusplus/releases
[master]: https://github.com/acgetchell/CDT-plusplus/tree/master
[.clang-format]: https://github.com/acgetchell/CDT-plusplus/blob/develop/.clang-format
[Tagged]: https://github.com/acgetchell/CDT-plusplus/tags
[ORCID]: https://orcid.org/
[Cppcheck]: http://cppcheck.sourceforge.net
[ASAN]: https://github.com/google/sanitizers/wiki/AddressSanitizer
[MSAN]: https://github.com/google/sanitizers/wiki/MemorySanitizer
[GitHub Actions]: https://github.com/acgetchell/CDT-plusplus/actions
[LSAN]: https://github.com/google/sanitizers/wiki/AddressSanitizerLeakSanitizer
[TSAN]: https://github.com/google/sanitizers/wiki/ThreadSanitizerCppManual