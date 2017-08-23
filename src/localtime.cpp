//
// Created by Adam Getchell on 2017-08-23.
//
#include "tz.h"
#include <iostream>

int
main()
{
    using namespace date;
    using namespace std::chrono;
    auto t = make_zoned(current_zone(), system_clock::now());
    std::cout << t << '\n';
}
