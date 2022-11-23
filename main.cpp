#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include "endian_types.h"

void execute();

// The geometry of a row of data
const uint32_t BYTES_PER_CYCLE = 32;
const uint32_t CYCLES_PER_ROW  = 64;
const uint32_t BYTES_PER_ROW   = BYTES_PER_CYCLE * CYCLES_PER_ROW;  /* 2048 bytes */

//=================================================================================================
// This is the 32-byte wide structure of a single record.   A record contains 1 data-cycle's worth
// of data.  Data types beginning with "be_" are big-endian
//=================================================================================================
struct cycle_t
{
    uint8_t     cycle;
    uint8_t     dummy1;
    uint8_t     dummy2;
    be_uint32_t row; 
    uint8_t     dummy3;
    uint8_t     dummy4;
    uint8_t     filler[23];
} record;
//=================================================================================================


//=================================================================================================
// main() - Execution starts here - no command line options
//=================================================================================================
int main()
{
    execute();
}
//=================================================================================================


//=================================================================================================
// execute() - Writes a data-file containing however many rows of data will fit into 4GB.
//
// The code below stamps the row # and cycle # into every cycle-record that gets written
//=================================================================================================
void execute()
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

        // Loop through each of the 64 cycles in a row
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