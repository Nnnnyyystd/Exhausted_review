/*
 * trivialProg.c
 *
 *  Created on: 2024-11-3
 *      Author: Deng Rong
 */

#include <stdio.h>
#include <stdlib.h>

int main1(int argc, char *argv[])
{
    int i;
    for(i = 0; i < argc; i++)
        printf("argument %d:\t%s\n", i, argv[i]);
    exit(0);
}

