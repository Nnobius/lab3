#include "coder.h"
#include "command.h"
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
	if (argc != 4) {
		printf("Usage:\n");
		printf("coder encode <in-file-name> <out-file-name>\n");
		printf("coder decode <in-file-name> <out-file-name>\n");
		return -1;
	}
	if (((strcmp (argv[1], "encode\0")) == 0)\
			&& (strstr (argv[2], ".txt\0")) && (strstr (argv[3], ".bin\0"))) {
		if (encode_file(argv[2], argv[3])) {
			printf("Some sort of error\n");
		}
	} else if (((strcmp (argv[1], "decode\0")) == 0)\
	 		&& (strstr (argv[2], ".bin\0")) && (strstr (argv[3], ".txt\0"))) {
		if (decode_file(argv[2], argv[3])) {
			printf("Some sort of error\n");
		}
	} else {
		printf("Usage:\n");
		printf("coder encode <in-file-name> <out-file-name>\n");
		printf("coder decode <in-file-name> <out-file-name>\n");
		return -1;
	}
	return 0;
}