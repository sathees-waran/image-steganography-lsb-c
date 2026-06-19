/* File: decode.c
 * Brief: Implements the LSB decoding pipeline - extracts a hidden secret file's data back out of a stego BMP image. */

#include <stdio.h>
#include <string.h>
#include "decode.h"
#include "types.h"

/* Validates and stores stego image filename and optional output base name from argv. */
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    if (argv[2] == NULL)
    {
        printf("Stego file has not been passed !\n");
        return e_failure;
    }
    if (strstr(argv[2], ".bmp") == NULL)
    {
        printf("Invalid bmp file !\n");
        return e_failure;
    }
    decInfo->stego_image_fname = argv[2];

    if (argv[3] == NULL)
    {
        strcpy(decInfo->output_fname, "output"); // no extension yet
    }
    else
    {
        strcpy(decInfo->output_fname, argv[3]); // user-given base name
    }

    return e_success;
}

/* Opens the stego image for reading in binary mode. */
Status open_file(DecodeInfo *decInfo)
{
    // Stego Image file
    decInfo->fptr_stego = fopen(decInfo->stego_image_fname, "rb");
    // Do Error handling
    if (decInfo->fptr_stego == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->stego_image_fname);

        return e_failure;
    }
    return e_success;
}

/* Top-level driver: opens stego file, verifies magic string, then decodes extension/size/data in order. */
Status do_decoding(DecodeInfo *decInfo)
{
    int ret = open_file(decInfo);
    if (ret == e_failure)
    {
        printf("open files failed\n");
        return e_failure;
    }

    char magic_str[10];
    decode_magic_string(decInfo, magic_str);

    char magic_pass[10];
    printf("Enter Magic string to continue Decoding : ");
    scanf("%s", magic_pass);

    if (strcmp(magic_pass, magic_str) != 0)
    {
        printf("Wrong Magic String !\n");
        return e_failure;
    }

    decode_secret_file_extn_size(&decInfo->extn_size, decInfo);
    decode_secret_file_extn(decInfo->secret_file_extn, decInfo);

    strcat(decInfo->output_fname, decInfo->secret_file_extn);

    // Opening output file
    decInfo->fptr_output = fopen(decInfo->output_fname, "w");

    // Do Error handling
    if (decInfo->fptr_output == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->output_fname);

        return e_failure;
    }

    decode_secret_file_size(&decInfo->size_secret_file, decInfo);
    decode_secret_data(decInfo);

    fclose(decInfo->fptr_stego);
    fclose(decInfo->fptr_output);
    
    return e_success;
}  

/* Decodes the 2-character magic string from the 16 image bytes right after the header. */
Status decode_magic_string(DecodeInfo * decInfo, char magic_str[])
{
    fseek(decInfo->fptr_stego, 54, SEEK_SET);
    for (int i = 0; i < 2; i++)
    {
        char buffer[8];
        fread(buffer, 1, 8, decInfo->fptr_stego);
        magic_str[i] = decode_byte_to_lsb(buffer);
    }
    magic_str[2] = '\0';
    return e_success;
}

/* Decodes the extension's character count from the next 32 image bytes. */
Status decode_secret_file_extn_size(int *extn_size, DecodeInfo *decInfo)
{
    char buffer[32];
    fread(buffer, 1, 32, decInfo->fptr_stego);
    *extn_size = decode_size_to_lsb(buffer);
    return e_success;
}

/* Decodes extn_size characters of the secret file's extension and null-terminates the result. */
Status decode_secret_file_extn(char *secret_file_extn, DecodeInfo *decInfo)
{
    for (int i = 0; i < decInfo->extn_size; i++)
    {
        char buffer[8];
        fread(buffer, 1, 8, decInfo->fptr_stego);
        secret_file_extn[i] = decode_byte_to_lsb(buffer);
    }
    secret_file_extn[decInfo->extn_size] = '\0';
    return e_success;
}

/* Decodes the secret file's byte count from the next 32 image bytes. */
Status decode_secret_file_size(long *file_size, DecodeInfo *decInfo)
{
    char buffer[32];
    fread(buffer, 1, 32, decInfo->fptr_stego);
    *file_size = decode_size_to_lsb(buffer);
    return e_success;
}

/* Decodes size_secret_file bytes of hidden data and writes each byte to the output file. */
Status decode_secret_data(DecodeInfo * decInfo)
{
    char ch;
    for (int i = 0; i < decInfo -> size_secret_file; i++)
    {
        char buffer[8];
        fread(buffer, 1, 8, decInfo->fptr_stego);
        ch = decode_byte_to_lsb(buffer);
        fwrite(&ch, 1, 1, decInfo->fptr_output);
    }
    return e_success;
}

/* Reassembles one byte of data from the LSBs of 8 image bytes. */
char decode_byte_to_lsb(char *image_buffer)
{ // image_buffer arr size is 8 bytes
    char ch = 0;
    int n = 7;
    for (int i = 0; i < 8; i++)
    {
        char bit = image_buffer[i] & 1;
        bit = bit << n;
        ch = ch | bit;
        n--;
    }
    return ch;
}

/* Reassembles a 32-bit integer from the LSBs of 32 image bytes. */
int decode_size_to_lsb(char *image_buffer)
{ // image_buffer size is 32 bytes
    int n = 31;
    int size = 0;
    for (int i = 0; i < 32; i++)
    {
        int bit = image_buffer[i] & 1;
        bit = bit << n;
        size = size | bit;
        n--;
    }
    return size;
}
