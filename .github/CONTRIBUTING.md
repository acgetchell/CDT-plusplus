# How to Contribute

First, thank you!

Writing esoteric scientific software can be it's own reward, but it's not for the faint of heart.

If you want a general overview as to why this software package exists, please look at my talk [Causal Dynamical Triangulations with CGAL][slides], or the [Wiki].

Second, here are some simple guidelines that will make it easier on me to process and accept your contributions.

1. Fork the repository.
2. Familiarize yourself with [Catch] and in particular, the [Gherkin] syntax.
3. Write a unit test for your proposed contribution. Unit tests go in the `test` directory and are named \{YourContribution\}Test.cpp, so that they can be automatically built.
All proposed features of your contribution should have a corresponding test in \{YourContribution\}Test.cpp.
Consult the [Catch Test cases and sections] if you are unsure, or consult existing tests for examples.
4. I highly recommend writing your tests first, before your contribution, as this helps to think about how the rest of the program will use your functions and/or classes.
[Test-Driven Development] (and [BDD]) has saved me quite a lot from various mistakes.
5. Project source files go into the `src` directory; header files go into `include`.
This makes integration into tests and the main program easy and modular, and follows convention.
6. Don't forget documentation! It's helpful if you state explicitly what your functions and classes do.
I use [Doxygen] to automatically build documentation, so using `/// @brief` and `/// @param` is helpful and easy.
Consult existing code for examples.
7. Commit your changes with a clear, [well-written commit message].
*I am guilty of not always writing the clearest commit messages in this project, and will do better.
For the longest time, however, no one but me actually read them.
When I contribute to other projects, I definitely try to have useful, clear commit messages and you should, too.*
8. Run `clang-format` using the project's [.clang-format].
9. Open a pull request against the develop branch of the main repository (which is the default).
[Travis-CI] will test it against combinations of Linux (Ubuntu 14.04), MacOS, clang, and gcc, so ensure that your code compiles on both platforms with both compilers.
10. All pull requests must pass [Travis-CI] and [AppVeyor] to be accepted.
In particular, look at results from git whitespace checks (`git diff --check HEAD^`), [ClangFormat], [ClangTidy], [Valgrind], [Coverity], and [Codecov] .
11. I will get to your change as soon as I can.
Feel free to ping me on [Gitter] with any questions.
You will receive proper credit for your contributions both in the code and any resulting scientific papers using the output of `git log --format='%aN | sort -u`.

# Style Guide

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
[ClangFormat]: https://github.com/acgetchell/CDT-plusplus/blob/master/.clang-format
[slides]: http://slides.com/acgetchell/causal-dynamical-triangulations-3
[Valgrind]: http://valgrind.org/docs/manual/quick-start.html#quick-start.mcrun
[cpp-core]: https://github.com/isocpp/CppCoreGuidelines/blob/master/CppCoreGuidelines.md
[clang-tidy.sh]: https://github.com/acgetchell/CDT-plusplus/blob/master/clang-tidy.sh
[AppVeyor]: https://ci.appveyor.com/project/acgetchell/cdt-plusplus
[Catch]: https://github.com/catchorg/Catch2/blob/master/docs/Readme.md
[Gherkin]: https://www.tutorialspoint.com/behavior_driven_development/behavior_driven_development_gherkin.htm
[BDD]: https://en.wikipedia.org/wiki/Behavior-driven_development
[Catch Test cases and sections]: https://github.com/catchorg/Catch2/blob/master/docs/test-cases-and-sections.md
[Codecov]: https://codecov.io/support
[Gitter]: https://gitter.im/acgetchell/CDT-plusplus
[ClangTidy]: http://clang.llvm.org/extra/clang-tidy/index.html
[Coverity]: https://scan.coverity.com/projects/acgetchell-cdt-plusplus
