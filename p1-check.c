/*
 * CS 261 PA1: Mini-ELF header verifier
 *
 * Name:     Gavin Boland
 */

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "p1-check.h"

void usage_p1 ()
{
    printf("Usage: y86 <option(s)> mini-elf-file\n");
    printf(" Options are:\n");
    printf("  -h      Display usage\n");
    printf("  -H      Show the Mini-ELF header\n");
    printf("Options must not be repeated neither explicitly nor implicitly.\n");
}

bool parse_command_line_p1 (int argc, char **argv, bool *header, char **file)
{
	if (argc == 0 || argv == NULL)
	{
		return false;
	}
	
	int opt;
    *header = false;
	*file = NULL;
	char *option_Str = "hH";


	while ((opt = getopt(argc, argv, option_Str)) != -1)  
    {
        switch (opt) 
        {
            case 'h':	usage_p1();
						*header = false;
						*file = NULL;                    
                        return true;
                        break;

			case 'H':	*header = true;
						option_Str = "h";
						break;


            case '?':	usage_p1();
						*header = false;
						*file = NULL; 
						return false;
						break;
        }
    }


	// I ripped this from the hexdump project.
	// Sets the name to the last argument,
	// before checking if the last option scanned up above
	// is the second to last argument(as it would be if there was 1 file name
	// as the last argument)

	if (optind + 1 != argc) 
	{
        usage_p1();
        return false;
    }
    *file = argv[argc - 1];

    


    return true;
}

/**
 * Reads the data stored in file into hdr
 */
bool read_header (FILE *file, elf_hdr_t *hdr)
{
	if (file == NULL || hdr == NULL) {
		return false;
	}

	// Determines if the file isn't large enough
	if (fseek(file, 16, SEEK_SET) != 0)
	{
    	return false;   
	}

	fseek(file, 0, SEEK_SET); // Resets where to start looking in the file
	fread(hdr, sizeof(uint16_t), 8, file); 

	char *actual = (char*)malloc(sizeof(char) * 9);
	if (sprintf(actual, "%08x", hdr->magic) < 0)
	{
		free(actual);
		return false;
	}
	
	// Compares the hex code for the magic number to the expected magic number
	// Represents "ELF/0" in hex.
	int comp = strcmp(actual, "00464c45");
	free(actual);

	if (comp != 0) // If the magic number isn't correct
	{
		return false;
	}


	return true;
}

/**
 * Prints out the header according to project specifications
 */
void dump_header (elf_hdr_t hdr)
{
	unsigned char *struct_string = (unsigned char *)&hdr;
	
	int i = sizeof(hdr);

	printf("00000000  ");
	while (i > 0) 
	{
		printf("%.2x ", *struct_string++);
		i--;
		if (i == sizeof(hdr) / 2)
		{
			printf(" ");
		}
	} 

	printf("\n");
	printf("Mini-ELF version %i\n", hdr.e_version);
	printf("Entry point 0x%x\n", hdr.e_entry);
	printf("There are %i program headers, starting at offset %i (0x%x)\n",
		hdr.e_num_phdr, hdr.e_phdr_start, hdr.e_phdr_start);
	if (hdr.e_symtab == 0) 
	{
		printf("There is no symbol table present\n");
	} 
	else
	{
		printf("There is a symbol table starting at offset %i (0x%x)\n", 
			hdr.e_symtab, hdr.e_symtab);
	}

	if (hdr.e_strtab == 0)
	{
		printf("There is no string table present\n");
	} 
	else
	{
		printf("There is a string table starting at offset %i (0x%x)\n", 
			hdr.e_strtab, hdr.e_strtab);
	}

		
}

