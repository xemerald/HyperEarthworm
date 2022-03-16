/* Test program for converting between nominal and true epoch times. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "qlib2.h"

extern void dump_leapsecond_table();

int main (int argc, char **argv)
{
    dump_leapsecond_table();
    return (0);
}
