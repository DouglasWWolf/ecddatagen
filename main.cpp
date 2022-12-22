#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include "endian_types.h"

void execute1();
void execute2();

// The geometry of a row of data
const uint32_t BYTES_PER_CYCLE = 64;
const uint32_t CYCLES_PER_ROW  = 32;
const uint32_t BYTES_PER_ROW   = BYTES_PER_CYCLE * CYCLES_PER_ROW;  /* 2048 bytes */

//=================================================================================================
// This is the 64-byte wide structure of a single record.   A record contains 1 data-cycle's worth
// of data.  Data types beginning with "be_" are big-endian
//=================================================================================================
struct cycle_t
{
    uint8_t     cycle;
    uint8_t     dummy1[2];
    be_uint32_t row; 
    uint8_t     dummy2[2];
    uint8_t     filler[55];
} record;
//=================================================================================================


//=================================================================================================
// main() - Execution starts here - no command line options
//=================================================================================================
int main(int argc, char** argv)
{
    // By default, we'll create a file of output type 0
    int output_type = 1;

    // If the user gave us an output-type, decode it to binary
    if (argv[1]) output_type = atoi(argv[1]);

    if (output_type < 1 || output_type > 2)
    {
        printf("%i is an invalid output type\n", output_type);
        exit(1);        
    }

    // Tell the caller what kind of data-file we're creating
    printf("Creating data file type %i\n", output_type);

    if (output_type == 1) execute1();
    if (output_type == 2) execute2();
}
//=================================================================================================


//=================================================================================================
// execute1() - Writes a data-file containing however many rows of data will fit into 4GB.
//
// The code below stamps the row # and cycle # into every cycle-record that gets written
//=================================================================================================
void execute1()
{
    const char* filename = "bigdata.dat";

    // Open the file we're going to write, and complain if we can't
    FILE* ofile = fopen(filename, "w");
    if (ofile == nullptr)
    {
        fprintf(stderr, "Can't create %s\n", filename);
        exit(1);
    }   

    // The default value of every field in the record is 0xFF
    memset(&record, 0xFF, sizeof record);
    
    // The "filler" field just contains consecutive bytes starting at 0
    for (int i=0; i<sizeof(record.filler); ++i) record.filler[i] = i;

    // How many rows will fit into 4GB?
    uint32_t row_count = 0x100000000LL / BYTES_PER_ROW;

    // Loop through every row of data we're going to write
    for (uint32_t row = 0; row < row_count; ++row)
    {
        // Store the row number into the record
        record.row = row;

        // Loop through each cycles in a row
        for (int cycle = 0; cycle <CYCLES_PER_ROW; ++cycle)
        {
            // Store the cycle number into the record
            record.cycle = cycle;

            // And write the record (i.e., one data-cycle) to disk
            fwrite(&record, 1, BYTES_PER_CYCLE, ofile);
        }
    }

    // Close the output file, we're done
    fclose(ofile);
}
//=================================================================================================



//=================================================================================================
// execute2() - Writes data suitable for a bulk transfer data-integrity test
//=================================================================================================
void execute2()
{
    const char* filename = "bigdata.dat";
    be_uint32_t data[16];
    uint32_t value1, value2, value3, value4;

    // Open the file we're going to write, and complain if we can't
    FILE* ofile = fopen(filename, "w");
    if (ofile == nullptr)
    {
        fprintf(stderr, "Can't create %s\n", filename);
        exit(1);
    }   

    // How many rows will fit into 4GB?
    uint32_t row_count = 0x100000000LL / BYTES_PER_ROW;

    // Seed value is zero
    value1 = 0;

    // Loop through every row of data we're going to write
    for (uint32_t row = 0; row < row_count; ++row)
    {

        // Loop through each data-cycle in the row
        for (int cycle = 0; cycle <CYCLES_PER_ROW; ++cycle)
        {
            value2 = value1 ^ 0xFFFFFFFF;
            value3 = value1 ^ 0xAAAAAAAA;
            value4 = value1 ^ 0x55555555;

            // Stamp the current set of values into the data for this cycle
            data[0] = data[4] = data[ 8] = data[12] = value1;
            data[1] = data[5] = data[ 9] = data[13] = value2;
            data[2] = data[6] = data[10] = data[14] = value3;
            data[3] = data[7] = data[11] = data[15] = value4;

            // And write the record (i.e., one data-cycle) to disk
            fwrite(data, 1, BYTES_PER_CYCLE, ofile);

            // Bump the value by some odd amount that won't fit evenly into 32-bits
            value1 += 57;
        }
    }

    // Close the output file, we're done
    fclose(ofile);
}
//=================================================================================================

