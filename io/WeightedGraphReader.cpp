#include "WeightedGraphReader.h"

std::istream& operator>>(std::istream& is, host::ArcType& type) {

	unsigned int i;
	is >> i;
	type = static_cast<host::ArcType>(i);

	return is;
}
