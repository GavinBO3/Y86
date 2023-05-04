/*
 * CS 261 PA4: Mini-ELF disassembler Main driver
 *
 * Name: Gavin Boland
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "p1-check.h"
#include "p2-load.h"
#include "p3-disas.h"
#include "p4-interp.h"


int main (int argc, char **argv)
{
    // Write code to call the parse function then 
    // open the elf file, read its header, and react depending on whether -H was present
 
	bool header;    // Print the elf header
	bool segments;	// Print the program headers
	bool membrief;	// Print the parts of memory specified in the program headers
	bool memfull;	// Print the entire memory
	bool discode;	// Disassemble and print the code
	bool disdata;	// Disassemble and print the data
	bool execnormal;// Print the cpu before and after execution 
	bool execdebug;	// Print the cpu after every instruction
	char *fileName;	// Character pointer containing the file name

	bool result = parse_command_line_p4(argc, argv, &header, &segments, 
		&membrief, &memfull, &discode, &disdata, &execnormal, &execdebug, &fileName);



/****************************************************************************************************************************************************
																	START OF PROJECT 1
****************************************************************************************************************************************************/
	if (result) {
		if (fileName == NULL) { // This means -h was invoked
			return EXIT_SUCCESS;
		}
			
		FILE *file = fopen(fileName, "r");
		if (file == NULL) {
			printf("Failed to open File\n");
			return EXIT_FAILURE;
		}

		elf_hdr_t hdr = {0};
		
		/**
		 * read_header returns false if there wasn't a valid
		 * mini-elf header. We want to terminate if it's invalid.
		 */
		if (!read_header(file, &hdr)) {
				printf("Failed to Read ELF Header\n");
				return EXIT_FAILURE;
		}

		// We want to store e_num_phdr headers inside a pointer we'll use as an array
		elf_phdr_t *ProgHeaders = malloc(sizeof(elf_phdr_t) * hdr.e_num_phdr);

		

		memory_t VirtMem = calloc(1, MEMSIZE); 


		if (header) {
			dump_header(hdr);
			printf("\n");
		}

		
		/**
		 * Reads all of the program headers into ProgHeaders.
		 */
		for (int i = 0; i < hdr.e_num_phdr; i++) {
			if (!read_phdr(file, (hdr.e_phdr_start + sizeof(elf_phdr_t) * i), 
				&ProgHeaders[i])) {	
				free(VirtMem);
				free(ProgHeaders);
				printf("Failed to Read Program Header\n");
				return EXIT_FAILURE;
			}
		}

		
		if (segments) {
			dump_phdrs(hdr.e_num_phdr, ProgHeaders);
		}

		
		/**
		 * Loads each segment into the vitual memory VirtMem.
		 */
		for (int i = 0; i < hdr.e_num_phdr; i++) {
			if (!load_segment(file, VirtMem, ProgHeaders[i])) {
				free(VirtMem);
				free(ProgHeaders);
				printf("Failed to Load Segment");
				return EXIT_FAILURE;
			}
		}
/****************************************************************************************************************************************************
																END OF PROJECT 1
****************************************************************************************************************************************************/
/****************************************************************************************************************************************************
															   START OF PROJECT 2
****************************************************************************************************************************************************/

		/**
		 * Dumps the full memory.
		 */
		if (memfull) {
			dump_memory(VirtMem, 0, MEMSIZE);
		}
		
		/**
		 * Dumps each part of the memory specified in header.
		 */

		if (membrief) {
			for (int i = 0; i < hdr.e_num_phdr; i++) {
				if (!ProgHeaders[i].p_filesz == 0) {
					uint16_t start = ProgHeaders[i].p_vaddr;
					uint16_t end = start + ProgHeaders[i].p_filesz;
					start += (16 - start) % 16;
					dump_memory(VirtMem, start, end);
				}
			}
		}
/*****************************************************************************************************************************************************
																	 END OF PROJECT 2
*****************************************************************************************************************************************************/
/*****************************************************************************************************************************************************
																	START OF PROJECT 3
*****************************************************************************************************************************************************/

		/**
		 * Disassembles the code portions of memory.
		 */
		if (discode) {
			printf("Disassembly of executable contents:\n");
			for (int i = 0; i < hdr.e_num_phdr; i++) {
				if (ProgHeaders[i].p_type == CODE) {
					disassemble_code(VirtMem, &ProgHeaders[i], &hdr);
					printf("\n");		
				}
			}
		}

		/**
		 * Disassembles the data portions of memory.
		 */
		if (disdata) {
			printf("Disassembly of data contents:\n");
			for (int i = 0; i < hdr.e_num_phdr; i++) {
				if (ProgHeaders[i].p_type == DATA) { 
					if (ProgHeaders[i].p_flag == RW) { 		  // For read write data
						disassemble_data(VirtMem, &ProgHeaders[i]);
						printf("\n");	
					} else if (ProgHeaders[i].p_flag == RO) { // For read only data
						disassemble_rodata(VirtMem, &ProgHeaders[i]);
						printf("\n");
					}	
				}
			}
		}
/*****************************************************************************************************************************************************
																	 END OF PROJECT 3
*****************************************************************************************************************************************************/
/*****************************************************************************************************************************************************
																	START OF PROJECT 4
*****************************************************************************************************************************************************/
		y86_t *cpu = calloc(1, sizeof(y86_t));
		y86_inst_t inst = {0};
		y86_register_t valA = {0};
		y86_register_t valE = {0};
		bool cond; // Represents the use of condition codes used in cmov and jump
		int instCount = 0; // Represents the number of instructions executed
	
		cpu->pc = hdr.e_entry; // Start executing the program at the line specified as the entry in the main header
		cpu->stat = AOK;

		printf("Entry execution point at 0x%.04lx\n", cpu->pc); 
		if (execnormal || execdebug)
		{
			printf("Initial ");
			dump_cpu(cpu);
		}
		






		while(cpu->stat == AOK)
		{

			inst = fetch(cpu, VirtMem); // We fetch each instruction. Implemented in p3-disas

			if (execdebug)
			{
				printf("Executing: ");
				disassemble(inst);

			}
			valE = decode_execute(cpu, &cond, &inst, &valA);
			instCount++;
		


			if (inst.type == INVALID) // In the case of an invalid opcode, don't write back or update program counter
			{
				printf("Corrupt Instruction (opcode 0x%x) at address 0x%.4lx\n", inst.opcode, cpu->pc);
				printf("Post-Fetch ");
				
				dump_cpu(cpu);

			}
			else 
			{

				memory_wb_pc(cpu, VirtMem, cond, &inst, valE, valA);


				if (execdebug)
				{
					printf("Post-Exec ");
					dump_cpu(cpu);
				}
			}






		}






		if (execnormal && inst.type != INVALID)
		{

			printf("Post-Exec ");

			dump_cpu(cpu);
		} 

		printf("Total execution count: %i instructions\n\n", instCount);

		if (execdebug && inst.type != INVALID)
		{
			dump_memory(VirtMem, 0, MEMSIZE);
		}

/*****************************************************************************************************************************************************
																	END OF PROJECT 4
*****************************************************************************************************************************************************/
		

	
		free(cpu);
		free(VirtMem);
		free(ProgHeaders);
		

		return EXIT_SUCCESS;
	}
	else {
		return EXIT_FAILURE;
	}

}




