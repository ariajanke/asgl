#include <common/ParseOptions.hpp>

#include <string>
#include <cstring>
#include <iostream>

namespace {

struct Example {
    std::string opt_a = "default";
    int opt_b = -1;
    bool opt_c = false, opt_d = false;
};

void parse_a(Example &, char ** beg, char ** end);
void parse_b(Example &, char ** beg, char ** end);
void parse_c(Example &, char ** beg, char ** end);
void parse_d(Example &, char ** beg, char ** end);

const char * to_string(bool v) { return v ? "true" : "false"; }

// in: "-ab" out: "", 0
// in: "--option-b a b c d" out: "default", 4
// in: "--option-a cat dog" out: "cat", -1
// in: "oranges beer --option-a sixty" out: "sixty", -1

} // end of <anonymous> namespace

int main(int argc, char ** argv) {
    using namespace std;
    Example a = parse_options<Example>(argc, argv, {
        { "option-a", 'a', parse_a },
        { "option-b", 'b', parse_b },
        { "option-c", 'c', parse_c },
        { "option-d", 'd', parse_d }
    });
    cout << "a: " << a.opt_a << " b: " << a.opt_b 
		 << " c: " << to_string(a.opt_c) << " d: " 
		 << to_string(a.opt_d) << std::endl;
}

namespace {

void parse_a(Example & ex, char ** beg, char ** end) {
    if (beg == end) return;
    ex.opt_a = *beg;
}

void parse_b(Example & ex, char ** beg, char ** end) {
    ex.opt_b = int(end - beg);
}

void parse_c(Example & ex, char **, char **) {
	ex.opt_c = true;
}

void parse_d(Example & ex, char **, char **) {
	ex.opt_d = true;
}

} // end of <anonymous> namespace
