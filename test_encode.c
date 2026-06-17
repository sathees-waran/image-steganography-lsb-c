#include <stdio.h>
#include "encode.h"
#include "types.h"

int main(int argc, char *argv[])
{
    EncodeInfo encInfo;
    int ret = check_operation_type(argv);

    if (ret == e_encode)
    {   // encoding
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
    return 0;
}
Status do_encoding(EncodeInfo *encInfo)
{
    int ret = open_files(encInfo);
    if (ret == e_failure)
    {
        printf("open files failed\n");
        return e_failure;
    }
    
    if(check_capacity(encInfo) == e_failure)
    {
        printf("Check capacity is failed\n");
        return e_failure;
    }
    //Copy bmp header
    if(copy_bmp_header(encInfo -> fptr_src_image, encInfo -> fptr_stego_image) == e_failure)
    {
        printf("Copying bmp header  Process Failed\n");
        return e_failure;
    }
}

Status check_capacity(EncodeInfo *encInfo)
{
    //get secret file extension
    strcpy(encInfo -> extn_secret_file, strchr(encInfo -> secret_fname, "."));
    
    //Get length of extension
    encInfo -> extn_size = strlen(encInfo -> extn_secret_file);

    //get secret file size
    fseek(encInfo -> fptr_secret, SEEK_END, 1);
    encInfo -> size_secret_file = ftell(encInfo -> fptr_secret);

    //Reset secret file back to first Byte
    rewind(encInfo -> fptr_secret);

    //Calculating total bytes
    /* Header + Magic string + extension size (always integer) + extension(encInfo -> extn_size * 8) + file size + file data (encInfo -> size_secret_file * 8)*/
    int Total_bytes_needed = 54 + 16 + 32 + (encInfo -> extn_size * 8) + 32 + (encInfo->extn_size * 8) + 32 + (encInfo->size_secret_file * 8);

    //bmp file size
    int bmp_file_size = get_image_size_for_bmp(encInfo -> fptr_src_image);

    //check if total bytes  is less than bmp file
     if(Total_bytes_needed <= bmp_file_size) return e_success;
     else return e_failure;
}
Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    // read 54 bytes from fptr_src_image
    // write 54 bytes to fptr_dest_image
    return e_success;
}
