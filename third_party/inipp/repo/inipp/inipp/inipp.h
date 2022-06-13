/*
MIT License

Copyright (c) 2017-2020 Matthias C. M. Troffaes

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
*/

#pragma once

#include <cstring>
#include <string>
#include <iostream>
#include <list>
#include <map>
#include <algorithm>
#include <functional>
#include <cctype>
#include <sstream>

namespace inipp {

namespace detail {

// trim functions based on http://stackoverflow.com/a/217605

template <class CharT>
inline void ltrim(std::basic_string<CharT> & s) {
	s.erase(s.begin(),
                std::find_if(s.begin(), s.end(),
                             [](int ch) { return !std::isspace(ch); }));
}

template <class CharT>
inline void rtrim(std::basic_string<CharT> & s) {
	s.erase(std::find_if(s.rbegin(), s.rend(),
                             [](int ch) { return !std::isspace(ch); }).base(),
                s.end());
}

// string replacement function based on http://stackoverflow.com/a/3418285

template <class CharT>
inline bool replace(std::basic_string<CharT> & str, const std::basic_string<CharT> & from, const std::basic_string<CharT> & to) {
	auto changed = false;
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::basic_string<CharT>::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length();
		changed = true;
	}
	return changed;
}

} // namespace detail

template <typename CharT, typename T>
inline bool extract(const std::basic_string<CharT> & value, T & dst) {
	CharT c;
	std::basic_istringstream<CharT> is{ value };
	T result;
	if ((is >> std::boolalpha >> result) && !(is >> c)) {
		dst = result;
		return true;
	}
			return false;

}

template <typename CharT>
inline bool extract(const std::basic_string<CharT> & value, std::basic_string<CharT> & dst) {
	dst = value;
	return true;
}

template<class CharT>
class Ini
{
public:
	typedef std::basic_string<CharT> String;
	typedef std::map<String, String> Section;
	typedef std::map<String, Section> Sections;

	Sections sections;
	std::list<String> errors;

	static const CharT char_section_start  = (CharT)'[';
	static const CharT char_section_end    = (CharT)']';
	static const CharT char_assign         = (CharT)'=';
	static const CharT char_comment        = (CharT)';';
	static const CharT char_interpol       = (CharT)'$';
	static const CharT char_interpol_start = (CharT)'{';
	static const CharT char_interpol_sep   = (CharT)':';
	static const CharT char_interpol_end   = (CharT)'}';

	static const int max_interpolation_depth = 10;

	void generate(std::basic_ostream<CharT> & os) const {
		for (auto const & sec : sections) {
			os << char_section_start << sec.first << char_section_end << std::endl;
			for (auto const & val : sec.second) {
				os << val.first << char_assign << val.second << std::endl;
			}
			os << std::endl;
		}
	}

	void parse(std::basic_istream<CharT> & is) {
		String line;
		String section;
		while (std::getline(is, line)) {
			detail::ltrim(line);
			detail::rtrim(line);
			const auto length = line.length();
			if (length > 0) {
				const auto pos = line.find_first_of(char_assign);
				const auto & front = line.front();
				if (front == char_comment) {
					continue;
				}
				if (front == char_section_start) {
					if (line.back() == char_section_end)
						section = line.substr(1, length - 2);
					else
						errors.push_back(line);
				}
				else if (pos != 0 && pos != String::npos) {
					String variable(line.substr(0, pos));
					String value(line.substr(pos + 1, length));
					detail::rtrim(variable);
					detail::ltrim(value);
					auto & sec = sections[section];
					if (sec.find(variable) == sec.end())
						sec.insert(std::make_pair(variable, value));
					else
						errors.push_back(line);
				}
				else {
					errors.push_back(line);
				}
			}
		}
	}

	void interpolate() {
		int global_iteration = 0;
		auto changed = false;
		// replace each "${variable}" by "${section:variable}"
		for (auto & sec : sections)
			replace_symbols(local_symbols(sec.first, sec.second), sec.second);
		// replace each "${section:variable}" by its value
		do {
			changed = false;
			const auto syms = global_symbols();
			for (auto & sec : sections)
				changed |= replace_symbols(syms, sec.second);
		} while (changed && (max_interpolation_depth > global_iteration++));
	}

	void default_section(const Section & sec) {
		for (auto & sec2 : sections)
			for (const auto & val : sec)
				sec2.second.insert(val);
	}

	void clear() {
		sections.clear();
		errors.clear();
	}

private:
	typedef std::list<std::pair<String, String> > Symbols;

	String local_symbol(const String & name) const {
		return char_interpol + (char_interpol_start + name + char_interpol_end);
	}

	String global_symbol(const String & sec_name, const String & name) const {
		return local_symbol(sec_name + char_interpol_sep + name);
	}

	String local_symbols(const String & sec_name, const Section & sec) const {
		Symbols result;
		for (const auto & val : sec)
			result.push_back(std::make_pair(local_symbol(val.first), global_symbol(sec_name, val.first)));
		return result;
	}

	String global_symbols() const {
		Symbols result;
		for (const auto & sec : sections)
			for (const auto & val : sec.second)
				result.push_back(
					std::make_pair(global_symbol(sec.first, val.first), val.second));
		return result;
	}

	bool replace_symbols(const Symbols & syms, Section & sec) const {
		auto changed = false;
		for (auto & sym : syms)
			for (auto & val : sec)
				changed |= detail::replace(val.second, sym.first, sym.second);
		return changed;
	}
};

} // namespace inipp
