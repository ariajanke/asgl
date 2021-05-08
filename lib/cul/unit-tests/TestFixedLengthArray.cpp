/****************************************************************************

    MIT License

    Copyright (c) 2020 Aria Janke

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.

*****************************************************************************/

#include <common/FixedLengthArray.hpp>

#include <iostream>

namespace {

struct CountedType {
    CountedType(): number(++inst_created_so_far) { ++counter; }
    ~CountedType() { --counter; }
    static int counter;
    int number;
    static int inst_created_so_far;
};

/* static */ int CountedType::counter = 0;
/* static */ int CountedType::inst_created_so_far = -1;

struct Simple {
    Simple(): c(' ') {}
    Simple(const Simple &) = default;
    char c;
};

using Error = std::runtime_error;

const char * const TEST_FAILED_MSG =
    "A test has failed for fixed length arrays.\n"
    "This indicates that the application is not ready to run and that the "
    "object will need to be fixed and debugged.";

void run_fixed_length_array_tests(std::ostream & out);

} // end of <anonymous> namespace

int main() {
    run_fixed_length_array_tests(std::cout);
}

namespace {

void run_fixed_length_array_tests(std::ostream & out) {
    out << " --- Fixed Length Array Test Suite --- \n"
           "If you are seeing this in a \"release\" build, something went "
           "horribly wrong in the compliation process (preprocessors were not "
           "properly defined)." << std::endl;
    // test 01, construct and destructer
    {
        {
        FixedLengthArray<CountedType, 5> test_array;
        (void)test_array;
        if (5 != CountedType::counter) {
            out << "[ FAILED ] TEST 01" << std::endl;
            throw Error(TEST_FAILED_MSG);
        }
        }
    if (0 != CountedType::counter) {
        out << "[ FAILED ] TEST 01" << std::endl;
        throw Error(TEST_FAILED_MSG);
    }
    }
    out << "[ PASSED ] TEST 01" << std::endl;
    // test 02, copying
    {
    FixedLengthArray<Simple, 5> target;
    FixedLengthArray<Simple, 5> source;
    for (Simple & sim : source)
        sim.c = '!';
    target = source;
    const FixedLengthArray<Simple, 5> & const_target = target;
    for (const Simple & sim : const_target) {
        if (sim.c != '!') {
            out << "[ FAILED ] TEST 02";
            throw Error(TEST_FAILED_MSG);
        }
    }
    }
    out << "[ PASSED ] TEST 02" << std::endl;
}

} // end of <anonymous> namespace
