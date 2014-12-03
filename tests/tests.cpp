#include "tests.h"

boost::filesystem::path dir_of(const char* filename) {

	boost::filesystem::path filepath(filename);
	return filepath.parent_path();
}
