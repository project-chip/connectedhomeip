#include "inipp.h"

#include <fstream>
#include <sstream>

using namespace inipp;

template <class CharT>
static inline auto read_all(const std::string & filename) {
	std::basic_ifstream<CharT> is(filename);
	std::basic_stringstream<CharT> sstr;
	sstr << is.rdbuf();
	return sstr.str();
}

template <class CharT>
static inline auto parse(const std::string & filename, Ini<CharT> & ini) {
	std::basic_ifstream<CharT> is(filename);
	ini.parse(is);
}

template <class CharT>
static inline void errors(std::basic_ostream<CharT> & os, const Ini<CharT> & ini) {
	for (auto const & err : ini.errors) {
		os << err << std::endl;
	}
}

template <class CharT>
static inline auto test(const std::string & inifile, Ini<CharT> & ini) {
	std::basic_ostringstream<CharT> os;
	parse(inifile, ini);
	os << ">>> ERRORS <<<" << std::endl;
	errors(os, ini);
	os << ">>> GENERATE <<<" << std::endl;
	ini.generate(os);
	os << ">>> INTERPOLATE <<<" << std::endl;
	ini.interpolate();
	ini.generate(os);
	return os.str();
}

template <class CharT>
static inline auto runtest(const char *inifile, const char *expectedfile, std::basic_ostream<CharT> & os) {
	Ini<CharT> ini;
	auto actual = test(inifile, ini);
	auto expected = read_all<CharT>(expectedfile);
	os << actual;
	return (actual == expected);
}
