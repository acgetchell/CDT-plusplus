### How to Contribute ###

First, thank you!

Writing esoteric scientific software can be it's own reward, but it's not for the faint of heart.

If you want a general overview as to why this software package exists, please look at my talk [Causal Dynamical Triangulations with CGAL][slides], or the [Wiki][2].

Second, here are some simple guidelines that will make it easier on me to process and accept your contributions.

1. Fork the repository.
2. Install GMock/GTest. See [.travis.yml][1] for details.
3. Write a unit test for your proposed contribution. Unit tests go in the `unittests` directory and are named \{YourContribution\}Test.cpp, so that they can be automatically built. All proposed features of your contribution should have a corresponding test in \{YourContribution\}Test.cpp. Consult the [GMock][3] and [GTest][4] documentation if you are unsure, or consult existing tests for examples. Prefer [predicate assertions][5] because their output is much easier to understand and useful for debugging.
4. I highly recommend writing your tests first, before your contribution, as this helps to think about how the rest of the program will use your functions and/or classes. This [Test-Driven Development][6] has saved me quite a lot from various mistakes.
5. Project source code goes into the `src` directory, generally as header files named \{YourContribution\}.h. This makes inclusion into tests and the main program easy and modular.
6. Don't forget documentation! It's helpful if you state explicitly what your functions and/or classes do. I use [Doxygen][7] to automatically build documentation, so using `/// @brief` and `/// @param[]` is helpful and easy. Consult existing code for examples.
7. Commit your changes with a clear, [well-written commit message][8]. _I am guilty of not always writing the clearest commit messages in this project, and will do better. For the longest time, however, no one but me actually read them. When I contribute to other projects, I definitely try to have useful, clear commit messages and you should, too._
8. Run `clang-format` using `.clang-format` included with the project.
9. Open a pull request against the main repository. [Travis-CI][9] will test it against combinations of Linux (Ubuntu 14.04), MacOS, clang, and gcc, so ensure that your code compiles on both platforms with both compilers.
10. Look at the results from the [Valgrind][14] tests on [Travis-CI][9], which will tell you about any memory leaks or other subtle issues.
11. I will get to your change as soon as I can. You will receive proper credit for your contributions both in the code and any resulting scientific papers using the output of `git log --format='%aN | sort -u`.

### Style Guide ###

This project generally follows [Stroustrup formatting with Allman brackets][10], enforced by [`clang-format`][13] using the [.clang-format][11] configuration. We also test against the [C++ Core Guidelines][cpp-core] using [clang-tidy][clang-tidy-sh] with an appropriate configuration. Clang-tidy changes the code when run, so comparing results and running unit tests is recommended before you commit, especially since some of clang-tidy's fixes break the codebase (the script runs just the tests that don't).

Most editors/IDEs have plugins for `clang-format` and `clang-tidy`.

[1]: https://github.com/acgetchell/CDT-plusplus/blob/master/.travis.yml
[2]: https://github.com/acgetchell/CDT-plusplus/wiki
[3]: https://github.com/google/googletest/blob/master/googlemock/README.md
[4]: https://github.com/google/googletest/blob/master/googletest/docs/Primer.md
[5]: https://github.com/google/googletest/blob/master/googletest/docs/AdvancedGuide.md#predicate-assertions-for-better-error-messages
[6]: http://alexott.net/en/cpp/CppTestingIntro.html
[7]: http://doxygen.org
[8]: http://chris.beams.io/posts/git-commit/
[9]: https://travis-ci.org/acgetchell/CDT-plusplus
[10]: https://isocpp.org/wiki/faq/coding-standards
[11]: https://github.com/acgetchell/CDT-plusplus/blob/master/.clang-format
[slides]: http://slides.com/acgetchell/causal-dynamical-triangulations-3
[13]: http://llvm.org/releases/4.0.0/tools/clang/docs/ClangFormatStyleOptions.html
[14]: http://valgrind.org/docs/manual/quick-start.html#quick-start.mcrun
[cpp-core]: https://github.com/isocpp/CppCoreGuidelines/blob/master/CppCoreGuidelines.md
[clang-tidy-sh]: https://github.com/acgetchell/CDT-plusplus/blob/master/clang-tidy.sh
