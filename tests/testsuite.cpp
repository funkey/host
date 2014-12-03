#include <iostream>
#include <tests.h>
#include <util/ProgramOptions.h>
#include <util/Logger.h>

BEGIN_TEST_MODULE(host)

	ADD_TEST_SUITE(graph);
	ADD_TEST_SUITE(conflict_candidates);
	ADD_TEST_SUITE(multi_factors);

END_TEST_MODULE()

int main(int argc, char** argv) {

	util::ProgramOptions::init(argc, argv);
	logger::LogManager::init();

	return ::boost::unit_test::unit_test_main(&host, argc, argv);
}
