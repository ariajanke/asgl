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

#pragma once
#include <initializer_list>

template <typename T>
using ParseFunc = void(*)(T &, char ** beg, char ** end);

template <typename T>
struct OptionTableEntry {
    const char * longname;
    char abbr;
    ParseFunc<T> parser;
};

template <typename T>
using OptionsTable = std::initializer_list<OptionTableEntry<T>>;

/** Parses program options and their arguments to a user specified type.
 *  @tparam OptionsType 
 *  @param  argc
 *  @param  argv
 */
template <typename OptionsType>
OptionsType parse_options
	(int argc, char ** argv, OptionsTable<OptionsType>);

class ParseOptionsPriv {
	template <typename OptionsType>
	friend OptionsType parse_options
		(int argc, char ** argv, OptionsTable<OptionsType> options_table);
		
	static bool is_same(const char * a, const char * b);

	enum { k_is_long, k_is_short, k_is_not_option };

	static decltype(k_is_long) detect_option_type(const char * opt);
	
	template <typename OptionsType>
	static ParseFunc<OptionsType> do_inbetweens
		(OptionsType * options, const char * opt, 
		 OptionsTable<OptionsType> options_table);

	template <typename OptionsType>
	static ParseFunc<OptionsType> get_long_option
		(const char * opt, OptionsTable<OptionsType> options_table);
};

template <typename OptionsType>
OptionsType parse_options
    (int argc, char ** argv, OptionsTable<OptionsType> options_table)
{
	// generally this follows a "wait until we get a whole list of 
	// arguments before executing the option"
	using PFunc = ParseFunc<OptionsType>;
	using Pop = ParseOptionsPriv;
    
    OptionsType rv;
    PFunc last_parser = nullptr;
    decltype(argv) last = nullptr;
    auto end = argv + argc;
    for (; argv != end; ++argv) {
        if (!last) last = argv;
        
        PFunc sel_f = nullptr;
        auto opt_type = Pop::detect_option_type(*argv);
        switch (opt_type) {
        case Pop::k_is_long:
			sel_f = Pop::get_long_option(*argv + 2, options_table);
            break;
        case Pop::k_is_short:
			sel_f = Pop::do_inbetweens(&rv, *argv + 1, options_table);
            break;
        case Pop::k_is_not_option: break;
        }
        
        // if we found an option, process the last one
        if (sel_f || opt_type != Pop::k_is_not_option) {
            if (last_parser) {
                last_parser(rv, last, last ? argv : nullptr);
            }
            
			last = nullptr;		
            last_parser = sel_f;
        }
    }
    // process any remaining option
    if (last_parser) {
		last_parser(rv, last, last ? end : nullptr);
	}
    return rv;
}

/* static */ inline bool ParseOptionsPriv::is_same(const char * a, const char * b) {
	while (*a && *b) {
		if (*a++ != *b++) return false;
	}
	return *a == *b;
}

/* static */ inline decltype(ParseOptionsPriv::k_is_long) ParseOptionsPriv::
	detect_option_type(const char * opt)
{
	if (opt[0] == '-') {
		return (opt[1] == '-') ? k_is_long : k_is_short;
	}
	return k_is_not_option;
}

template <typename OptionsType>
/* static */ ParseFunc<OptionsType> ParseOptionsPriv::do_inbetweens
	(OptionsType * options, const char * opt, 
	 OptionsTable<OptionsType> options_table)
{
	ParseFunc<OptionsType> last_f = nullptr;
	for (; *opt; ++opt) {
		for (const auto & entry : options_table) {
			if (entry.abbr == *opt) {
				if (last_f) last_f(*options, nullptr, nullptr);
				last_f = entry.parser;
				break;
			}
		}
	}
	return last_f;
}

template <typename OptionsType>
/* static */ ParseFunc<OptionsType> ParseOptionsPriv::get_long_option
	(const char * opt, OptionsTable<OptionsType> options_table)
{
	for (const auto & entry : options_table) {
		if (is_same(entry.longname, opt)) {
			return entry.parser;
		}
	}
	return nullptr;
}
