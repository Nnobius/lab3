#include "coder.h"
#include "command.h"
#include <string.h>
#include <stdlib.h>

int encode_file(const char *in_file_name, const char *out_file_name) 
{
	FILE *in;
	FILE *out;

	in = fopen ( in_file_name, "r");
	if (in == NULL) {
		printf ("Opening error %s\n", in_file_name);

		return -1;
	}

	out = fopen ( out_file_name, "wb");
	if (out == NULL) {
		printf ("Opening error %s\n", out_file_name);
		if (fclose (in)) {
			printf ("Closing error %s\n", in_file_name);
		}
		return -1;
	}

	CodeUnits *code = malloc (sizeof (CodeUnits));
	if (code == NULL) {
		printf ("Initialization error\n");
		return -1;
	}
	for (int f = 0;;) {
		char ch;

		do {
			ch = getc(in);
			if (ch == EOF) {
				f = -2;
			}
		} while (ch == ' ' || ch == '\n');

		if (fseek (in, -1, SEEK_CUR)) {
			f = -1;
		}
		char codel [8];
		uint32_t num;

		if (fgets (codel, 8, in) == NULL) {
			if (feof (in)) {
				f = -2;
			}
			if (ferror (in)) {
				f = -1;
			}
		}

		num = strtol (codel, NULL, 16);
		if (encode (num, code)) {
			f = -1;
		}
		
		if (f == 0) {
			if (write_code_unit (out, code)) {
				printf("Error of output\n");
			}
		} else if (f == -1) {
			printf("Error\n");
			break;
		} else if (f == -2) {
			printf("Operation complete \n");
			break;
		}
	}
	free (code);
	if (fclose (in)) {
		printf ("Closing error %s\n", in_file_name);
		return -1;
	}
    if (fclose (out)) {
		printf ("Closing error %s\n", out_file_name);
		return -1;
	}
	return 0;
}

int decode_file(const char *in_file_name, const char *out_file_name) 
{
	FILE *in;
	FILE *out;

	in = fopen ( in_file_name, "rb");
	if (in == NULL) {
		printf ("Opening error %s\n", in_file_name);
		return -1;
	}

	out = fopen ( out_file_name, "wt");
	if (out == NULL) {
		printf ("Opening error %s\n", out_file_name);

		if (fclose (in)) {
			printf ("Closing error %s\n", in_file_name);
		}
		return -1;
	}

	CodeUnits *code = malloc (sizeof (CodeUnits));
	if (code == NULL) {
		printf ("Initialization error\n");
		return -1;
	}
	for (int a = 0;;) {
		a = read_next_code_unit (in, code);
		if (a == 0) {
			uint32_t code_point = 0;

			code_point = decode (code);

			fprintf(out, "%x\n", code_point);
		} else if (a == -1) {
			printf("Error\n");
			break;
		} else if (a == -2) {
			printf("Operation complete\n");
			break;
		}
	}
    free (code);
	if (fclose (in)) {
		printf ("Closing error %s\n", in_file_name);
		return -1;
		}
	if (fclose (out)) {
		printf ("Closing error %s\n", out_file_name);
		return -1;
	}
	return 0;
}