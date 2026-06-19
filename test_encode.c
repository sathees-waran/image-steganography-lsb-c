/*
 * Name        : Sathees Waran
 * File Name   : test_encode.c
 * Brief       : Entry point for the image steganography (LSB) project.
 * Description : Reads command-line args to decide encode (-e) or decode
 *               (-d) mode, then calls the matching argument-validation
 *               and pipeline functions, reporting success/failure.
 */

#include <stdio.h>
#include "encode.h"
#include "decode.h"
#include "types.h"

int main(int argc, char *argv[])
{
    EncodeInfo encInfo;
    int ret = check_operation_type(argv);

    if (ret == e_encode)
    { // encoding
        int ret = read_and_validate_encode_args(argv, &encInfo);

        if (ret == e_failure)
        {
            printf("validation is wrong.\n");
            return 0;
        }

        ret = do_encoding(&encInfo);
        if (ret == e_failure)
        {
            printf("encoding is failed\n");
            return 0;
        }

        printf("encoding is successful ... \n");
        return 0;
    }
    
    DecodeInfo decInfo;
    if(ret == e_decode)
    {
        int ret = read_and_validate_decode_args(argv, &decInfo);
        if (ret == e_failure)
        {
            printf("validation is wrong.\n");
            return 0;
        }
         ret = do_decoding(&decInfo);
        if (ret == e_failure)
        {
            printf("decoding is failed\n");
            return 0;
        }

        printf("decoding is successful ... \n");
        return 0;
    }

    return 0;
}
