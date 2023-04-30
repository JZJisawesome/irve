/* NAME//TODO
 * By: John Jekel
 *
 * TODO description
 *
*/

/* Constants And Defines */

//TODO

/* Includes */

#include <cassert>
#include <iostream>
#include <fstream>

#include "emulator.h"

#define INST_COUNT emulator.get_inst_count()
#include "logging.h"

/* Types */

//TODO

/* Variables */

//TODO

/* Static Function Declarations */

static void load_memory_image(emulator_t &emulator, const char *filename);

/* Function Implementations */

int main(int argc, char **argv) {

    emulator_t emulator;
    irvelog(0, "Starting IRVE");
    irvelog(1, "The Inextensible RISC-V Emulator");

    if (argc < 2) {
        irvelog(0, "No memory image file specified. Starting with empty memory.");
    } else {
        assert(argc == 2 && "Too many arguments for now");//TODO remove this if we need in the future
        load_memory_image(emulator, argv[1]);
    }

    //TESTING do an infinite loop instead
    for (uint32_t i = 0; i < 10; ++i) {
        emulator.tick();
    }

    irvelog(0, "IRVE is shutting down. Bye bye!");
    return 0;
}

/* Static Function Implementations */

static void load_memory_image(emulator_t &emulator, const char *filename) {
    irvelog(0, "Loading memory image from file \"%s\"", filename);

    // //Read a file into the emulator byte-by-byte
    // FILE *file = fopen(filename, "rb");
    // assert((file != NULL) && "Failed to open memory image file");
    // fseek(file, 0, SEEK_END);
    // long file_size = ftell(file);
    // rewind(file);
    // assert((file_size >= 0) && "Failed to get file size");
    // irvelog(1, "Memory image file size is %ld bytes", file_size);
    // for (long i = 0; i < file_size; ++i) {
    //     int8_t byte = fgetc(file);
    //     emulator.mem_write(i, 0b000, static_cast<int32_t>(byte));
    // }

    std::string mem_file{filename};
    std::fstream fin("testfiles/" + mem_file);
    assert(fin && "Failed to open memory image file");
    std::string in;
    uint32_t addr{};
    while(fin >> in) {
        int32_t instruction{};
        assert(in.length() == 8 && "Memory image file is not formatted correctly");
        for(int i{}; i<8; ++i) {
            char c{static_cast<char>(toupper(in[i]))};
            int32_t hex{};
            if(c > 47 && c < 58) {
                hex = c - 48;
            }
            else if(c > 64 && c < 71) {
                hex = c - 55;
            }
            else {
                assert(0 && "Invalid character in memory image file");
            }
            instruction += (hex << (28 - 4*i));
        }
        emulator.mem_write(addr, 0b010, instruction);
        addr += 4;
    }
    fin.close();
}
