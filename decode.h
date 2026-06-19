/* File: decode.h
 * Brief: DecodeInfo struct and function prototypes for the decoding pipeline. */

#ifndef DECODE_H
#define DECODE_H

#include "types.h" // Contains user defined types

/* 
 * Structure to store information required for
 * encoding secret file to source Image
 * Info about output and intermediate data is
 * also stored
 */

#define MAX_SECRET_BUF_SIZE 1
#define MAX_FNAME_SIZE 50
#define MAX_FILE_SUFFIX 5

typedef struct _DecodeInfo
{
    //stego input
    char *stego_image_fname;
    FILE *fptr_stego;

    //secret file info
    int extn_size;
    char secret_file_extn[MAX_FILE_SUFFIX];
    long size_secret_file;

    /* output file Info */
    char output_fname[MAX_FNAME_SIZE];
    FILE *fptr_output;

} DecodeInfo;

/* Read and validate decode args from argv */
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo);

/* Perform the decoding */
Status do_decoding(DecodeInfo *decInfo);

/*Open file*/
Status open_file(DecodeInfo *decInfo);

/* Decode Magic String */
Status decode_magic_string(DecodeInfo *decInfo, char *magic_string);

/* Decode secret file extenstion size */
Status decode_secret_file_extn_size(int *extn_size, DecodeInfo *decInfoo);

/* Decode secret file extenstion  */
Status decode_secret_file_extn(char *secret_file_extn, DecodeInfo *decInfo);

/* decode secret file size */
Status decode_secret_file_size(long *file_size, DecodeInfo *decInfo);

/*Decode file data*/
Status decode_secret_data(DecodeInfo * decInfo);

/*Decode byte from lsb*/
char decode_byte_to_lsb(char *image_buffer);

/*Decode size from lsb*/
int decode_size_to_lsb(char *image_buffer);

#endif
