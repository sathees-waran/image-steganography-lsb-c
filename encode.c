/* File: encode.c
 * Brief: Implements the LSB encoding pipeline - hides a secret file's data inside a BMP image. */

#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "types.h"
#include "common.h"

/* Determines encode/decode mode from argv[1] ("-e"/"-d"). */
OperationType check_operation_type(char *argv[])
{
    if (argv[1] == NULL) return e_unsupported;

    if (strcmp(argv[1], "-e") == 0) return e_encode;

    if (strcmp(argv[1], "-d") == 0) return e_decode;

    return e_unsupported;
}

/* Validates and stores source image, secret file, and optional output filename from argv. */
Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    if (argv[2] == NULL)
    {
        printf("bmp file has not been passed\n");
        return e_failure;
    }
    // Check argv[2] is bmp file
    if (strstr(argv[2], ".bmp") == NULL)
    {
        printf("Invalid bmp file\n");
        return e_failure;
    }
    // Storing argv[2] bmp filename to structure
    encInfo->src_image_fname = argv[2];

    if (argv[3] == NULL)
    {
        printf("Secret file is Not Present\n");
        return e_failure;
    }
    // Check argv[3] is Secret file
    if (strchr(argv[3], '.') == NULL)
    {
        printf("Invalid Secret file\n");
        return e_failure;
    }
    // Storing argv[3] secret filename to structure
    encInfo->secret_fname = argv[3];

    if (argv[4] == NULL)
    {
        encInfo->stego_image_fname = "stego.bmp";
    }
    else
    {
        if (strstr(argv[4], ".bmp") == NULL)
        {
            printf("Invalid output file name\n");
            return e_failure;
        }
        encInfo->stego_image_fname = argv[4];
        return e_success;
    }
    return e_success;
}

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    printf("width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    printf("height = %u\n", height);

    // Return image capacity
    return width * height * 3;
}

/* Opens source image, secret file, and stego output file; returns e_failure if any fopen fails. */
Status open_files(EncodeInfo *encInfo)
{
    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "rb");
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);

        return e_failure;
    }

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "rb");
    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);

        return e_failure;
    }

    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "wb");
    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);

        return e_failure;
    }

    // No failure return e_success
    return e_success;
}

/* Top-level driver: opens files, checks capacity, then encodes header/magic string/extension/size/data in order. */
Status do_encoding(EncodeInfo *encInfo)
{
    int ret = open_files(encInfo);
    if (ret == e_failure)
    {
        printf("open files failed\n");
        return e_failure;
    }

    if (check_capacity(encInfo) == e_failure)
    {
        printf("Check capacity is failed\n");
        return e_failure;
    }
    // Copy bmp header
    if (copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_failure)
    {
        printf("Copying bmp header  Process Failed\n");
        return e_failure;
    }

    encode_magic_string(MAGIC_STRING, encInfo);
    encode_secret_file_extn_size(encInfo -> extn_size, encInfo );
    encode_secret_file_extn(encInfo -> extn_secret_file,encInfo);
    encode_secret_file_size(encInfo -> size_secret_file, encInfo);
    encode_secret_file_data(encInfo);
    copy_remaining_img_data(encInfo->fptr_src_image, encInfo->fptr_stego_image);
    fclose(encInfo->fptr_src_image);
    fclose(encInfo->fptr_secret);
    fclose(encInfo->fptr_stego_image);

    return e_success;
}

/* Computes secret file's extension/size and checks the source image has enough bytes to hide it all. */
Status check_capacity(EncodeInfo *encInfo)
{
    // get secret file extension
    strcpy(encInfo->extn_secret_file, strchr(encInfo->secret_fname, '.'));

    // Get length of extension
    encInfo->extn_size = strlen(encInfo->extn_secret_file);

    // get secret file size
    fseek(encInfo->fptr_secret, 0, SEEK_END);
    encInfo->size_secret_file = ftell(encInfo->fptr_secret);

    // Reset secret file back to first Byte
    rewind(encInfo->fptr_secret);

    // Calculating total bytes
    /* Header + Magic string + extension size (always integer) + extension(encInfo -> extn_size * 8) + file size + file data (encInfo -> size_secret_file * 8)*/
    int Total_bytes_needed = 54 + 16 + 32 + (encInfo->extn_size * 8) + 32 + (encInfo->extn_size * 8) + 32 + (encInfo->size_secret_file * 8);

    // bmp file size
    int bmp_file_size = get_image_size_for_bmp(encInfo->fptr_src_image);

    // check if total bytes  is less than bmp file
    if (Total_bytes_needed <= bmp_file_size)
        return e_success;
    else
        return e_failure;
}

/* Copies the 54-byte BMP header unchanged from source to stego image. */
Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    char header[54];
    // read 54 bytes from fptr_src_image
    rewind(fptr_src_image);
    fread(header, 1, 54, fptr_src_image);
    // write 54 bytes to fptr_dest_image
    fwrite(header, 1, 54, fptr_dest_image);

    return e_success;
}

/* Hides one byte of data across the LSBs of 8 image bytes. */
Status encode_byte_to_lsb(char data, char *image_buffer)
{ // image_buffer arr size is 8 bytes
    int n = 7;
    for (int i = 0; i < 8; i++)
    {
        char mask = 1 << n;
        char bit = data & mask;
        bit = bit >> n;
        image_buffer[i] = image_buffer[i] & 0xfe;
        image_buffer[i] = image_buffer[i] | bit;
        n--;
    }
    return e_success;
}

/* Hides a 32-bit integer across the LSBs of 32 image bytes. */
Status encode_size_to_lsb(int data, char *image_buffer)
{ // image_buffer size is 32 bytes
    int n = 31;
    for (int i = 0; i < 32; i++)
    {
        int mask = 1 << n;
        int bit = data & mask;
        bit = bit >> n;
        image_buffer[i] = image_buffer[i] & 0xfe;
        image_buffer[i] = image_buffer[i] | bit;
        n--;
    }
    return e_success;
}

/* Encodes the 2-character magic string into the first 16 image bytes after the header. */
Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    for (int i = 0; i < 2; i++)
    { // 1. read 8 bytes of data buffer from src get_file_size
        char buffer[8];
        fread(buffer, 1, 8, encInfo -> fptr_src_image);
        encode_byte_to_lsb(magic_string[i], buffer);

        // 2.write 8 bytes of buffer to stego file
        fwrite(buffer, 1, 8, encInfo -> fptr_stego_image);
    }
    return e_success;
}

/* Encodes the extension's character count into the next 32 image bytes. */
Status encode_secret_file_extn_size(int extn_size, EncodeInfo *encInfo)
{
    char buffer[32];

    // 1.read 32 bytes from src file
    fread(buffer, 1, 32, encInfo -> fptr_src_image);
    encode_size_to_lsb(extn_size, buffer);
    // 2.write 32 bytes buffer to stego file
    fwrite(buffer, 1, 32, encInfo -> fptr_stego_image);
    return e_success;

}

/* Encodes each character of the secret file's extension into successive image bytes. */
Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    for (int i = 0; file_extn[i] != '\0'; i++)
    {
        char buffer[8];
        // read 8 bytes from src file and store to buffer
        fread(buffer, 1, 8, encInfo -> fptr_src_image);

        encode_byte_to_lsb(file_extn[i], buffer);

        // write 8 bytes of buffer to stego file
        fwrite(buffer, 1, 8, encInfo -> fptr_stego_image);
    }
    return e_success;
}

/* Encodes the secret file's byte count into the next 32 image bytes. */
Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{ 
    // 1.read 32 bytes from src image and store to buffer
    char buffer[32];
    fread(buffer, 1, 32, encInfo -> fptr_src_image);
    encode_size_to_lsb(file_size, buffer);
    // write 32 bytes of buffer to stego file
    fwrite(buffer, 1, 32, encInfo -> fptr_stego_image);

    return e_success;
}

/* Reads the secret file byte by byte and hides each byte across 8 image bytes until EOF. */
Status encode_secret_file_data(EncodeInfo *encInfo)
{
    char ch;
    while (fread(&ch, 1, 1, encInfo->fptr_secret) == 1)
    { // read 1 byte from sec file
        char buffer[8];
        // read 8 bytes from src image
        fread(buffer, 1, 8, encInfo -> fptr_src_image);

        encode_byte_to_lsb(ch, buffer);

        // write buffer to stego file
        fwrite(buffer, 1, 8, encInfo -> fptr_stego_image);
    }
    return e_success;
}

/* Copies remaining untouched image bytes from source to stego image after encoding. */
Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    char ch;
    while (!feof(fptr_src))
    {
        // Read 1 byte from src file
        fread(&ch, 1, 1, fptr_src);

        // Write 1 byte to dest file
        fwrite(&ch, 1, 1, fptr_dest);
    }
    return e_success;
}
