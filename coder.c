#include "coder.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

int encode (uint32_t code_point, CodeUnits *code_units)
{
	int bitnum = 0;
	int sup = 0;
	code_units->length = 0;
	
	for (int i = 0; i < MaxCodeLength; i++) {
		code_units->code [i] = 0;
	}

	for (uint32_t i = 1; i != 0; i = i << 1){
		sup++;
		if ((code_point & i) != 0) {
			bitnum = sup;
		}
	}

	if (bitnum > 21) {
		printf ("A lot of bits\n");
		return -1;
	}

	if (bitnum <= 7) {
		code_units->length = 1;
		} else if (bitnum > 7 && bitnum <= 11 ) {
			code_units->length = 2;
		} else if (bitnum > 11 && bitnum <= 16 ) {
			code_units->length = 3;
		} else if (bitnum > 16 && bitnum <= 21 ) {
			code_units->length = 4;
	}

	if (code_units->length == 1) {
		code_units->code[0] = code_point & 0x7F;
	} else if (code_units->length == 2) {
		code_units->code[0] = (code_point >> 6) | 0xC0;
		code_units->code[1] = (code_point & 0x3F ) | 0x80;
	} else if (code_units->length == 3) {
		code_units->code[0] = (code_point >> 12) | 0xE0;
		code_units->code[1] = ((code_point >> 6) & 0x3F ) | 0x80;
		code_units->code[2] = (code_point & 0x3F ) | 0x80;
	} else if (code_units->length == 4) {
		code_units->code[0] = (code_point >> 18) | 0xF0;
		code_units->code[1] = ((code_point >> 12) & 0x3F ) | 0x80;
		code_units->code[2] = ((code_point >> 6) & 0x3F ) | 0x80;
		code_units->code[3] = (code_point & 0x3F ) | 0x80;
	}

	return 0;
}

uint32_t decode (const CodeUnits *code_units)
{
	uint32_t code_point = 0;

	if (code_units->length == 1) {
		code_point = code_point | code_units->code [0];
	} else if (code_units->length == 2) {
		for (int i = 0; i < code_units->length; i++) {
			if (i == 0) {
				code_point = code_point | (code_units->code [i] & 0x1F);
			} else {
				code_point = code_point << 6;
				code_point = code_point | (code_units->code [i] & 0x3F);
			}
		}
	} else if (code_units->length == 3) {
		for (int i = 0; i < code_units->length; i++) {
			if (i == 0) {
				code_point = code_point | (code_units->code [i] & 0xF);
			} else {
				code_point = code_point << 6;
				code_point = code_point | (code_units->code [i] & 0x3F);
			}
		}
	} else if (code_units->length == 4) {
		for (int i = 0; i < code_units->length; i++) {
			if (i == 0) {
				code_point = code_point | (code_units->code [i] & 0x7);
			} else {
				code_point = code_point << 6;
				code_point = code_point | (code_units->code [i] & 0x3F);
			}
		}
	}

	return code_point;
}


int read_next_code_unit (FILE *in, CodeUnits *code_units)
{
	CodeUnits *form = malloc (sizeof (CodeUnits));
	if (form == NULL) {
		return -1;
	}
	uint8_t *byte = malloc (sizeof (uint8_t));
	while (1) {
		if (fread (byte, sizeof (uint8_t), 1, in) != 1) {
		if (ferror (in)) {
			free (byte);
			free (form);
			return -1;
		}
		if (feof (in)) {
			free (byte);
			free (form);
			return -2;
		}
	}

	if ((*byte & 0xc0) == 0x80) continue;
	if ((*byte & 0x80) == 0) {
		form->code [0] = *byte;
		form->length = 1;
	} else if ((*byte & 0xe0) == 0xc0) {
		form->code [0] = *byte;
		form->length = 2;
		uint8_t *cont = malloc (sizeof (uint8_t));
		if (cont == NULL) {
			return -1;
		}
		for (int i = 1; i < form->length; i++) {
			if (fread (cont, sizeof (uint8_t), 1, in) != 1) {
				if (ferror (in)) {
					free (byte);
					free (form);
					return -1;
				}
				if (feof (in)) {
					free (byte);
					free (form);
					return -2;
				}
			}
			if ((*cont & 0xc0) == 0x80) {
				form->code [i] = *cont;
			} else {
				if (fseek (in, -1, SEEK_CUR)) {
					free (byte);
					free (form);
					return -1;
				}
				continue;
			}
		}
	} else if ((*byte & 0xf0) == 0xe0) {
		form->code [0] = *byte;
		form->length = 3;
		uint8_t *cont = malloc (sizeof (uint8_t));
		if (cont == NULL) {
			return -1;
		}
		for (int i = 1; i < form->length; i++) {
			if (fread (cont, sizeof (uint8_t), 1, in) != 1) {
				if (feof (in)) {
					free (byte);
					free (form);
					return -2;
				}
				if (ferror (in)) {
					free (byte);
					free (form);
					return -1;
				}
			}
			if ((*cont & 0xc0) == 0x80) {
				form->code [i] = *cont;
			} else {
				if (fseek (in, -1, SEEK_CUR)) {
					free (byte);
					free (form);
					return -1;
				}
				continue;
			}
		}
	} else if ((*byte & 0xf8) == 0xf0) {
		form->code [0] = *byte;
		form->length = 4;
		uint8_t *cont = malloc (sizeof (uint8_t));
		if (cont == NULL) {
			return -1;
		}
		for (int i = 1; i < form->length; i++) {
			if (fread (cont, sizeof (uint8_t), 1, in) != 1) {
				if (ferror (in)) {
					free (byte);
					free (form);
					return -1;
				}
				if (feof (in)) {
					free (byte);
					free (form);
					return -2;
				}
			}
			if ((*cont & 0xc0) == 0x80) {
				form->code [i] = *cont;
			} else {
				if (fseek (in, -1, SEEK_CUR)) {
					free (byte);
					free (form);
					return -1;
				}
				continue;
			}
		}
	} break;
}
	for (int i = 0; i < MaxCodeLength; i++) {
		code_units->code[i] = form->code[i];
	}
	code_units->length = form->length;
	free (form);
	free (byte);
	return 0;
}

int write_code_unit (FILE *out, const CodeUnits *code_units)
{
	if (code_units->length == 1) {
		fwrite (&code_units->code[0], 1, 1, out);
		return 0;
	} else if (code_units->length == 2) {
		for (int i = 0; i < code_units->length; i++) {
			if (fwrite (&code_units->code[i], 1, 1, out) != 1) {
				return -1;
			}
		}
		return 0;
	} else if (code_units->length == 3) {
		for (int i = 0; i < code_units->length; i++) {
			if (fwrite (&code_units->code[i], 1, 1, out) != 1) {
				return -1;
			}
		}
		return 0;
	} else if (code_units->length == 4) {
		for (int i = 0; i < code_units->length; i++) {
			if (fwrite (&code_units->code[i], 1, 1, out) != 1) {
				return -1;
			}
		}
		return 0;
	}
	return -1;
}