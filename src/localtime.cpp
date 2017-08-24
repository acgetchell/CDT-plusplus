//
// Testing https://github.com/HowardHinnant/date
//
#include "date/tz.h"
#include <iostream>

int
main()
{
    using namespace date;
    using namespace std::chrono;
    auto t = make_zoned(current_zone(), system_clock::now());
    std::cout << t << '\n';
}
