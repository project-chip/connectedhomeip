#include <fstream>
#include "inipp.h"

int main() {
	inipp::Ini<char> ini;
	std::ifstream is("example.ini");
	ini.parse(is);
	std::cout << "raw ini file:" << std::endl;
	ini.generate(std::cout);
	ini.default_section(ini.sections["DEFAULT"]);
	ini.interpolate();
	std::cout << "ini file after default section and interpolation:" << std::endl;
	ini.generate(std::cout);
	int compression_level = -1;
	inipp::extract(ini.sections["bitbucket.org"]["CompressionLevel"], compression_level);
	std::cout << "bitbucket.org compression level: " << compression_level << std::endl;
	return 0;
}
