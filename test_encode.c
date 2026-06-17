#include <stdio.h>
#include "encode.h"
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

    return 0;
}
