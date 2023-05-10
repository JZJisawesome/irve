/* unit_tester.cpp
 * Copyright (C) 2023 John Jekel and Nick Chan
 * See the LICENSE file at the root of the project for licensing info.
 *
 * Performs unit tests for IRVE 
 *
*/

//FIXME when in release mode, assertions in libirve will be disabled. But we want to leave assertions actual unit tests enabled!

/* Includes */

#include "unit_tests.h"

#include <iostream>
#include <cassert>
#include <cstdlib>
#include <ctime>
#include <string>
#include <unordered_map>

/* Constants And Defines */

#define X(test_name) {#test_name, test_##test_name},
const std::unordered_map<std::string, int (*)()> TEST_MAP = {
    TEST_LIST
};
#undef X

/* Function Implementations */

int main(int argc, char** argv) {
    assert(argc == 2 && "Incorrect number of arguments. Usage: unit_tester test_to_run");

    //Initialize random number generator for tests that use it
    srand(time(NULL));
    
    //Lookup the test to run and run it
    auto test = TEST_MAP.find(argv[1]);
    assert((test != TEST_MAP.end()) && "Test not found. Check the spelling and that it is in TEST_LIST");
    std::cout << "Running test: \"" << argv[1] << "\"" << std::endl;
    return test->second();
}
