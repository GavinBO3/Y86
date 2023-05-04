/*
 * CS 261 PA3: Mini-ELF disassembler
 *
 * Name: Gavin Boland
 */

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "p3-disas.h"

char *registerName(y86_rnum_t in);
char *opName(y86_op_t in);
char *cmovName(y86_cmov_t in);
char *jumpName(y86_jump_t in);

//============================================================================
void usage_p3 ()
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
	printf("  -d      Disassemble code contents\n");
	printf("  -D      Disassemble data contents\n");

    printf("Options must not be repeated neither explicitly nor implicitly.\n");

}

//============================================================================
bool parse_command_line_p3 (int argc, char **argv,
        bool *header, bool *segments, bool *membrief, bool *memfull,
        bool *disas_code, bool *disas_data, char **file)
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
	*disas_code = false;
	*disas_data = false;
	*file = NULL;
	char *option_Str = malloc(sizeof(char) * 7);
	strcpy(option_Str, "hHafsmMdD");


	while ((opt = getopt(argc, argv, option_Str)) != -1)  
    {
        switch (opt) 
        {
			// Display usage
            case 'h':	usage_p3();
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

			// Disassemble code
			case 'd':	*disas_code = true;
						break;
	
			// Disassemble data
			case 'D':	*disas_data = true;
						break;
						

            case '?':	usage_p3();
						*header = false;
						*segments = false;
						*file = NULL; 
						*membrief = false;
						*memfull = false;
						*disas_code= false;
						*disas_data = false;
						free(option_Str);						
						return false;
						break;
        }

		free(option_Str);
		// Reconstructs the options based on what's been used.
		option_Str = malloc(sizeof(char) * 9);
		
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

		if (!*disas_code)
		{
			strcat(option_Str, "d");
		}
		
		if (!*disas_data)
		{
			strcat(option_Str, "D");
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
        usage_p3();
        return false;
    }
    *file = argv[argc - 1];

    

    return true;

}

//============================================================================
/**
 * Returns "ins", a struct containing necessary information about the current instruction specified in y86.h
 */
y86_inst_t fetch (y86_t *cpu, memory_t memory)
{
	y86_inst_t ins = {0};
	if (cpu == NULL || memory == NULL)
	{
		ins.type = INVALID;
        cpu->stat = INS;
        ins.opcode = INVALID;
		return ins;
	}
    

	

    // Initialize the instruction
    memset( &ins , 0 , sizeof(y86_inst_t) );  // Clear all fields i=on instr.    
    ins.type = INVALID;   // Invalid instruction until proven otherwise

	ins.opcode = memory[cpu->pc]; // The opcode is the first byte in the instruction.

    
	
	ins.type = ins.opcode >> 4; // The type of instruction is the 4 left most bits of the opcode.
    


    


	switch (ins.type) // See Y86 reference sheet for information on each instruction.
		{
			case HALT:		ins.size = 1;
				cpu->stat = HLT;
				if ((ins.opcode & 0xf) != 0)
				{
					ins.type = INVALID;
					cpu->stat = INS;
				}
				break;
		
			case NOP:		ins.size = 1;	
				if ((ins.opcode & 0xf) != 0)
				{
					ins.type = INVALID;
					cpu->stat = INS;
				}	
				break;
	
			case CMOV:		ins.size = 2;
				ins.ra = memory[cpu->pc + 1] >> 4;
				ins.rb = memory[cpu->pc + 1] & 0x0f;
	            if (ins.ra >= BADREG || ins.rb >= BADREG)
	            {   
		            cpu->stat = INS;
		            ins.type = INVALID;
                    break;
	            }
				switch(ins.opcode)
				{
					case 0x20: 	ins.cmov = RRMOVQ;
						break;
					case 0x21: 	ins.cmov = CMOVLE;
						break;
					case 0x22: 	ins.cmov = CMOVL;
						break;
					case 0x23: 	ins.cmov = CMOVE;
						break;
					case 0x24:	ins.cmov = CMOVNE;
						break;
					case 0x25:	ins.cmov = CMOVGE;
						break;
					case 0x26:	ins.cmov = CMOVG;
						break;
					default: 	ins.cmov = BADCMOV;
								ins.type = INVALID;
                                ins.opcode = INVALID;
								cpu->stat = INS;
						break;
				}
				break;

			case IRMOVQ:	ins.size = 10;	
				ins.rb = memory[cpu->pc + 1] & 0x0f;
	            if (memory[cpu->pc + 1] >> 4 != 0xf || ins.rb > BADREG || (ins.opcode & 0xf))
	            {   
		            cpu->stat = INS;
		            ins.type = INVALID;
                    break;
	            }
				memcpy(&(ins.value), memory + cpu->pc + 2, 8);

				break;

			case RMMOVQ:	ins.size = 10;
				ins.ra = memory[cpu->pc + 1] >> 4;
				ins.rb = memory[cpu->pc + 1] & 0x0f;
	            if (ins.ra > BADREG || ins.rb > BADREG || (ins.opcode & 0xf))
	            {   
		            cpu->stat = INS;
		            ins.type = INVALID;
                    break;
	            }
				memcpy(&(ins.d), memory + cpu->pc + 2, 8);


				break;

			case MRMOVQ: 	ins.size = 10;
				ins.ra = memory[cpu->pc + 1] >> 4;
				ins.rb = memory[cpu->pc + 1] & 0x0f;
	            if (ins.ra >= BADREG || ins.rb > BADREG || (ins.opcode & 0xf))
	            {   
		            cpu->stat = INS;
		            ins.type = INVALID;
                    break;
	            }
				memcpy(&(ins.d), memory + cpu->pc + 2, 8);


				break;
			
			case OPQ:		ins.size = 2;
				ins.ra = memory[cpu->pc + 1] >> 4;
				ins.rb = memory[cpu->pc + 1] & 0x0f;
	            if (ins.ra >= BADREG || ins.rb >= BADREG)
	            {   
		            cpu->stat = INS;
		            ins.type = INVALID;
                    break;
	            }
				switch(memory[cpu->pc])
				{
					case 0x60: ins.op = ADD;
						break;
					case 0x61: ins.op = SUB;
						break;
					case 0x62: ins.op = AND;
						break;
					case 0x63: ins.op = XOR;
						break;
					default: ins.op = BADOP;
							 ins.type = INVALID;
							 cpu->stat = INS;
						break;
				}		
				break;
		
			case JUMP:		ins.size = 9;	
				memcpy(&(ins.dest), memory + cpu->pc + 1, 8);
				switch(memory[cpu->pc])
				{	
					case 0x70: ins.jump = JMP;
						break;
					case 0x71: ins.jump = JLE;
						break;
					case 0x72: ins.jump = JL;
						break;	
					case 0x73: ins.jump = JE;
						break;
					case 0x74: ins.jump = JNE;
						break;
					case 0x75: ins.jump = JGE;
						break;
					case 0x76: ins.jump = JG;
						break;
					default: ins.jump = BADJUMP;
							ins.type = INVALID;
							cpu->stat = INS;
						break;
				}
				break;

			case CALL:		ins.size = 9;
				memcpy(&(ins.dest), memory + cpu->pc + 1, 9);
				if ((ins.opcode & 0xf) != 0)
				{
					ins.type = INVALID;
                    ins.opcode = INVALID;
					cpu->stat = INS;
				}

				break;
			
			case RET:		ins.size = 1;
				if ((ins.opcode & 0xf) != 0)
				{
					ins.type = INVALID;
					cpu->stat = INS;
				}
				
				break;
			
			case PUSHQ:	ins.size = 2;	
				ins.ra = memory[cpu->pc + 1] >> 4;
	            if (ins.ra >= BADREG || (memory[cpu->pc + 1] & 0xf) != 0xf || ((ins.opcode & 0xf) != 0))
	            {   
		            cpu->stat = INS;
		            ins.type = INVALID;
                    break;
	            }

				
				break;
				
			case POPQ:		ins.size = 2;
				ins.ra = memory[cpu->pc + 1] >> 4;

	            if (ins.ra >= BADREG || (memory[cpu->pc + 1] & 0xf) != 0xf || ((ins.opcode & 0xf) != 0))
	            {   
		            cpu->stat = INS;
		            ins.type = INVALID;
                    break;
	            }

				break;

			default:	
					cpu->stat = INS;
                    ins.type = INVALID;
				break;


		}
    if (cpu->pc + ins.size >= MEMSIZE)
    {
       	cpu->stat = ADR;
		ins.type = INVALID;
        ins.opcode = INVALID;     
    }






    return ins;
}

//============================================================================
/**
 * Prints an instruction according to project specifications.
 */
void disassemble (y86_inst_t inst)
{
	switch (inst.type)
		{
			case HALT:		printf("halt");
				break;
		
			case NOP:		printf("nop");	
				break;
	
			case CMOV:		printf("%s %%%s, %%%s", cmovName(inst.cmov), registerName(inst.ra), registerName(inst.rb));
				break;

			case IRMOVQ:	printf("irmovq 0x%zx, %%%s", inst.value, registerName(inst.rb));
				break;

			case RMMOVQ:	
				if (inst.rb != BADREG) 
				{
					printf("rmmovq %%%s, 0x%zx(%%%s)", registerName(inst.ra), inst.d, registerName(inst.rb));	
				}
				else 
				{
					printf("rmmovq %%%s, 0x%zx", registerName(inst.ra), inst.d);					
				}
				break;

			case MRMOVQ: 	
				if (inst.rb != BADREG)
				{
					printf("mrmovq 0x%zx(%%%s), %%%s", inst.d, registerName(inst.rb), registerName(inst.ra));
				}
				else 
				{
					printf("mrmovq 0x%zx, %%%s", inst.d, registerName(inst.ra));
				}			
				break;
			
			case OPQ:		printf("%s %%%s, %%%s", opName(inst.op), registerName(inst.ra), registerName(inst.rb));
				break;
		
			case JUMP:		printf("%s 0x%zx", jumpName(inst.jump), inst.dest);
				break;

			case CALL:		printf("call 0x%zx", inst.dest);
				break;
			
			case RET:		printf("ret");
				break;
			
			case PUSHQ:		printf("pushq %%%s", registerName(inst.ra));	
				break;
				
			case POPQ:		printf("popq %%%s", registerName(inst.ra));
				break;

			case INVALID:		
				break;


		}

	
	printf("\n");
}

/**
 * A helper method that returns a char pointer containing the specific kind of jump.
 */
char *jumpName(y86_jump_t in)
{
	switch(in)
	{
		case JMP:		return "jmp";
			break;
		case JLE:		return "jle";
			break;
		case JL:		return "jl";
			break;
		case JE:		return "je";
			break;
		case JNE:		return "jne";
			break;
		case JGE:		return "jge";
			break;
		case JG:		return "jg";
			break;
		case BADJUMP:	return "badjump";
			break;
	}
	return NULL;
}

/**
 * A helper method that returns a char pointer containing the specific kind of cmov.
 */

char *cmovName(y86_cmov_t in)
{
	switch(in)
	{
		case RRMOVQ: 	return "rrmovq";
			break;
		case CMOVLE: 	return "cmovle";
			break;
		case CMOVL: 	return "cmovl";
			break;
		case CMOVE: 	return "cmove";
			break;
		case CMOVNE: 	return "cmovne";
			break;
		case CMOVGE:	return "cmovge";
			break;
		case CMOVG:		return "cmovg";
			break;
		default: 		return "badcmov";
			break;
	}
	return NULL;
}

/**
 * A helper method that returns a char pointer containing the specific kind of operand.
 */

char *opName(y86_op_t in)
{
	switch(in)
	{
		case ADD:	return "addq";
			break;
		
		case SUB:	return "subq";
			break;

		case AND:	return "andq";
			break;
		
		case XOR:	return "xorq";
			break;
		
		default:	return "badop";
			break;
	}
	return NULL;
}

/**
 * A helper method that returns a char pointer containing the register name.
 */
char *registerName(y86_rnum_t in)
{
	
	switch(in)
	{
		case RAX: return "rax";
			break;

		case RCX: return "rcx";
			break;

		case RDX: return "rdx";
			break;

		case RBX: return "rbx";
			break;

		case RSP: return "rsp";
			break;

		case RBP: return "rbp";
			break;

		case RSI: return "rsi";
			break;

		case RDI: return "rdi";
			break;

		case R8: return "r8";
			break;

		case R9: return "r9";
			break;

		case R10: return "r10";
			break;

		case R11: return "r11";
			break;

		case R12: return "r12";
			break;

		case R13: return "r13";
			break;

		case R14: return "r14";
			break;

		case BADREG: return "badreg";
			break;
	}
	return NULL;
}


//============================================================================
void disassemble_code (memory_t memory, elf_phdr_t *phdr, elf_hdr_t *hdr)
{
	if (phdr == NULL || hdr == NULL || phdr->p_vaddr + phdr->p_filesz > MEMSIZE)
	{
		printf("Null header\n");
		return;
	} 
	y86_t *cpu = malloc(sizeof(y86_t));
	cpu->pc = phdr->p_vaddr;
	
	printf("  0x%.3lx:                      | .pos 0x%.3lx code\n", cpu->pc, cpu->pc);

	


	
	
	while (cpu->pc < phdr->p_filesz + phdr->p_vaddr)
	{
		if (cpu->pc == hdr->e_entry)
		{
			printf("  0x%.3lx:                      | _start:\n", cpu->pc);
		}
		y86_inst_t inst = fetch(cpu, memory);



		if (inst.type == INVALID)
		{
			printf("Invalid opcode: 0x%x\n", memory[cpu->pc]);
			break;
		}

		char* pr = calloc(1, sizeof(char) * 25);
		char* temp = malloc(sizeof(char) * 3);
 
		for (int i = 0; i < 10; i++)
		{
			if (i < inst.size) 
			{
				sprintf(temp, "%.2x", memory[cpu->pc + i]);
				strcat(pr, temp);
			} 
			else 
			{	
				strcat(pr, "  ");
			}
		}
			
		printf("  0x%.3lx: %s |   ", cpu->pc, pr);
		free(pr);
		free(temp);
		
		disassemble(inst); 
		cpu->pc += inst.size;
				
	}
	free(cpu);
	

				
		
}

//============================================================================
void disassemble_data (memory_t memory, elf_phdr_t *phdr)
{
	if (phdr == NULL)
	{
		printf("Null program header\n");
		return;
	} 

	y86_t *cpu = malloc(sizeof(y86_t));
	cpu->pc = phdr->p_vaddr;
	
	printf("  0x%.3lx:                      | .pos 0x%.3lx data\n", cpu->pc, cpu->pc);

	


	
	
	while (cpu->pc < phdr->p_filesz + phdr->p_vaddr)
	{

		char* pr = malloc(sizeof(char) * 25);
		char* temp = malloc(sizeof(char) * 2);
 
		for (int i = 0; i < 10; i++)
		{
			if (i < 8) 
			{
				sprintf(temp, "%.2x", memory[cpu->pc + i]);
				strcat(pr, temp);
			} 
			else 
			{	
				strcat(pr, "  ");
			}
		}

		
		printf("  0x%.3lx: %s |   ", cpu->pc, pr);
		free(pr);
		free(temp);
		
		int64_t *tempTwo = malloc(sizeof(int64_t));
		memcpy(tempTwo, memory + cpu->pc, 8);	

		printf(".quad 0x%zx\n", *tempTwo);
		free(tempTwo); 
		
		cpu->pc += 8;
				
	}
	free(cpu);
}


//============================================================================
void disassemble_rodata (memory_t memory, elf_phdr_t *phdr) // Read-only data
{
	if (phdr == NULL)
	{
		printf("Null program header\n");
		return;
	} 
	y86_t *cpu = malloc(sizeof(y86_t));
	cpu->pc = phdr->p_vaddr;
	
	printf("  0x%.3lx:                      | .pos 0x%.3lx rodata\n", cpu->pc, cpu->pc);

	
	bool finishedWord = false;

	
	
	while (cpu->pc < phdr->p_filesz + phdr->p_vaddr)
	{

		char* pr = malloc(sizeof(char) * 25);
		char* temp = malloc(sizeof(char) * 2);
 			
		bool end = false;
		int l = 0;
		for (int i = 0; i < 10; i++)
		{
			if(end) 
			{
				strcat(pr, "  ");			
			} 
			else 
			{	
				sprintf(temp, "%.2x", memory[cpu->pc + i]);
				strcat(pr, temp);
				l++;
			}
			if (memory[cpu->pc + i] == 0)
			{
				end = true;
			}
		}

		
		printf("  0x%.3lx: %s |  ", cpu->pc, pr);
		free(pr);
		free(temp);

		
		if (!finishedWord)
		{
			char *s = malloc(sizeof(char) * phdr->p_filesz);
			char *temp = malloc(sizeof(char) * 2);
/**
			for (int i = cpu->pc; i < phdr->p_filesz + phdr->p_vaddr; i++)
			{
				memcpy(temp, memory + i, 1);	
				strcat(s, temp);

				
				if(memory[i] == 0)
				{	
					finishedWord = true;
					break;
				}
			
			}*/
//printf(" .string \"%s\"", s);

            printf(" .string \"");
            int i;
            for (i = cpu->pc; memory[i] != 0x00; i++)
            {
                printf("%c", memory[i]);
            }
            printf("\"");

            if(memory[i] == 0)
			{	
				finishedWord = true;
			}

			free(s);
			free(temp);	 
		}
		
		
		cpu->pc += l;
		if (end)
		{
			finishedWord = false;
			
		}
		printf("\n");
	}
	free(cpu);
}
//============================================================================

