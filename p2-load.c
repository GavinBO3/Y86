/*
 * CS 261 PA2: Mini-ELF loader
 *
 * Name: WRITE  YOUR FULL NAME HERE
 */

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "p1-check.h"
#include "p2-load.h"

void usage_p2 ()
{
    printf("Usage: y86 <option(s)> mini-elf-file\n");
    printf(" Options are:\n");
    printf("  -h      Display usage\n");
    printf("  -H      Show the Mini-ELF header\n");
	printf("  -a      Show all with brief memory\n");
    printf("  -f      Show all with full memory\n");
	printf("  -s      Show the program headers\n");
	printf("  -m      Show the memory contents (brief)\n");
	printf("  -M      Show the memory contents (full)\n");
    printf("Options must not be repeated neither explicitly nor implicitly.\n");
}

bool parse_command_line_p2 (int argc, char **argv,
        bool *header, bool *segments, bool *membrief, bool *memfull,
        char **file)
{
    if (argc == 0 || argv == NULL)
	{
		return false;
	}
	
	int opt;
    *header = false;
	*segments = false;
	*membrief = false;
	*memfull = false;
	*file = NULL;
	char *option_Str = malloc(sizeof(char) * 7);
	strcpy(option_Str, "hHafsmM");


	while ((opt = getopt(argc, argv, option_Str)) != -1)  
    {
        switch (opt) 
        {
			// Display usage
            case 'h':	usage_p2();
						*header = false;
						*segments = false;
						*membrief = false;
						*memfull = false;
						*file = NULL;
						free(option_Str);                    
                        return true;
                        break;

			// Show Mini-ELF header
			case 'H':	*header = true;
						break;
			// Show the program headers
			case 's':	*segments = true;
						break;

			// Show the brief memory contents
			case 'm':	*membrief = true;
						*memfull = false;
						break;

			// Show the full memory contents
			case 'M':	*membrief = false;
						*memfull = true;
						break;

			// Show Mini-elf and program headers, along with brief memory contents
			case 'a':	*header = true;
						*segments = true;
						*membrief = true;
						*memfull = false;
						break;

			// Show Mini-elf and program headers, along with brief memory contents
			case 'f':	*header = true;
						*segments = true;
						*membrief = false;
						*memfull = true;
						break;

            case '?':	usage_p2();
						*header = false;
						*segments = false;
						*file = NULL; 
						*membrief = false;
						*memfull = false;
						free(option_Str);						
						return false;
						break;
        }

		free(option_Str);
		// Reconstructs the options based on what's been used.
		option_Str = malloc(sizeof(char) * 7);
		
		strcat(option_Str, "h");	// The help option is always available
		if(!*header)
		{
			strcat(option_Str, "H"); // 
		}
	
		if(!*segments)
		{
			strcat(option_Str, "s"); //
		}
		
		if(!*membrief && !*memfull)
		{
			strcat(option_Str, "mM");
		}
	
		if(!*header && !*segments && !*membrief && !*memfull)
		{
			strcat(option_Str, "af");
		}
    }

	free(option_Str);



	// I ripped this from the hexdump project.
	// Sets the name to the last argument,
	// before checking if the last option scanned up above
	// is the second to last argument(as it would be if there was 1 file name
	// as the last argument)

	if (optind + 1 != argc) 
	{
        usage_p2();
        return false;
    }
    *file = argv[argc - 1];

    

    return true;

}

/**
 * Reads the program header from file starting at offset into phdr.
 */
bool read_phdr (FILE *file, uint16_t offset, elf_phdr_t *phdr)
{
	if (file == NULL || phdr == NULL) 
	{
		return false;
	}

	// Determines if the file isn't large enough
	if (fseek(file, offset + sizeof(elf_phdr_t), SEEK_SET) != 0) 
	{
		return false;
	}
	
	// Resets where to start looking in the file
	fseek(file, offset, SEEK_SET);
	fread(phdr, sizeof(elf_phdr_t), 1, file);
	
	if (phdr->magic != 0xDEADBEEF) 
	{
		return false;
	}
	
    return true;
}

/**
 * Prints all the program headers in the phdr array.
 */
void dump_phdrs (uint16_t numphdrs, elf_phdr_t phdr[])
{

	printf("Segment   Offset    VirtAddr  FileSize  Type      Flag\n");
	for (int i = 0; i < numphdrs; i++) 
	{	
		char *type;
		switch(phdr[i].p_type)
		{
			case 0: type = "DATA      ";
				break;
			case 1: type = "CODE      ";
				break;
			case 2: type = "STACK     ";
				break;
			case 3: type = "HEAP      ";
				break;
			default: type = "UNKNOWN   ";
				break;
		}
		
		char *flag;
		switch(phdr[i].p_flag)
		{
			case 4:	flag = "R  ";
				break;
			case 5: flag = "R X";
				break;
			case 6: flag = "RW ";
				break;
		}
					

		printf("  %02d      0x%04x    0x%04x    0x%04x    %s%s\n",
			i, phdr[i].p_offset, phdr[i].p_vaddr, phdr[i].p_filesz, type, flag);
	}
	printf("\n");

}

/**
 * Reads the data specified in the program header from the file into the virtual memory
 */
bool load_segment (FILE *file, memory_t memory, elf_phdr_t phdr)
{
	if (file == NULL || memory == NULL) // Sanity checking
	{
		return false;
	}

	if (phdr.p_filesz == 0) // If the data to read is = 0, skip the rest.
	{
		return true;
	}

	// Would load into an invalid space.	
	if (phdr.p_vaddr + phdr.p_filesz > MEMSIZE) 
	{
		return false;
	}

	// Checks to see if the file is large enough
	if (fseek(file, phdr.p_offset + phdr.p_filesz, SEEK_SET) != 0)
 	{
		return false;
	}
	
	fseek(file, phdr.p_offset, SEEK_SET);
	fread(&memory[phdr.p_vaddr], phdr.p_filesz, 1, file); 
	

	

    return true;
}

/**
 * Prints the memory from start to end.
 */
void dump_memory (memory_t memory, uint16_t start, uint16_t end)
{
	printf("Contents of memory from %04x to %04x:", start, end);
	int i = 16; // keeps track of certain spacing requirements
	for (; start < end; start++, i++)
	{




		if (i == 8) 
		{
			printf(" ");
		}
		if (i == 16)
		{
			printf("\n  %04x  ", start);
			i = 0;
		}
		

		printf("%.02x", memory[start]); 
		if (i != 15)
		{
			printf(" ");
		}
	}
			
	printf("\n");
	printf("\n");
		
		
}
