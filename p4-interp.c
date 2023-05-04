/*
 * CS 261 PA4: Mini-ELF interpreter
 *
 * Name: WRITE  YOUR  NAME  HERE
 */

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "p4-interp.h"

//=======================================================================

void usage_p4 ()
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
	printf("  -e      Execute program\n");
	printf("  -E      Execute program (debug trace mode)\n");

    printf("Options must not be repeated neither explicitly nor implicitly.\n");



}

//=======================================================================

bool parse_command_line_p4 
    (   int argc         , char **argv      ,
        bool *header     , bool *segments   , bool *membrief , 
        bool *memfull    , bool *disas_code , bool *disas_data ,
        bool *exec_normal, bool *exec_debug , char **file 
    )
{
    if (argc == 0 || argv == NULL)
	{
		return false;
	}
	
	int opt = 0;
    *header = false;
	*segments = false;
	*membrief = false;
	*memfull = false;
	*disas_code = false;
	*disas_data = false;
	*exec_normal = false;
	*exec_debug = false;
	*file = NULL;

	char *option_Str = calloc(1, sizeof(char) * strlen("hHafsmMdDeE") + 2);
	strcpy(option_Str, "hHafsmMdDeE");


	while ((opt = getopt(argc, argv, option_Str)) != -1)  
    {

        switch (opt) 
        {
			// Display usage
            case 'h':	usage_p4();
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
	
			// Normal execution printing
			case 'e':	*exec_normal = true;
						break;
						
			// Debug execution printing
			case 'E':	*exec_debug = true;
						*exec_normal = false;
						break;
			
            case '?':	usage_p4();
						*header = false;
						*segments = false;
						*file = NULL; 
						*membrief = false;
						*memfull = false;
						*disas_code= false;
						*disas_data = false;
						*exec_normal = false;
						*exec_debug = false;
						free(option_Str);						
						return false;
						break;
        }

		free(option_Str);




		// Reconstructs the options based on what's been used.
		option_Str = calloc(1, sizeof(char) * 11);
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

		if (!*exec_normal && !*exec_debug)
		{
			strcat(option_Str, "e");
		}

		if (!*exec_debug) // E supercedes e
		{
			strcat(option_Str, "E");
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
        usage_p4();
        return false;
    }
    *file = argv[argc - 1];
	
    

    return true;

}


//=======================================================================

// A helper method that returns a pointer to the appropriate register in the cpu
y86_register_t *cpuRegister(y86_t *cpu, y86_rnum_t regnum)
{
	y86_register_t *toReturn;
	switch(regnum)
	{
		case RAX: toReturn = &(cpu->rax);
			break;

		case RCX: toReturn = &(cpu->rcx);
			break;

		case RDX: toReturn = &(cpu->rdx);
			break;

		case RBX: toReturn = &(cpu->rbx);
			break;

		case RSP: toReturn = &(cpu->rsp);
			break;

		case RBP: toReturn = &(cpu->rbp);
			break;

		case RSI: toReturn = &(cpu->rsi);
			break;

		case RDI: toReturn = &(cpu->rdi);
			break;

		case R8: toReturn = &(cpu->r8);
			break;

		case R9: toReturn = &(cpu->r9);
			break;

		case R10: toReturn = &(cpu->r10);
			break;

		case R11: toReturn = &(cpu->r11);
			break;

		case R12: toReturn = &(cpu->r12);
			break;

		case R13: toReturn = &(cpu->r13);
			break;

		case R14: toReturn = &(cpu->r14);
			break;
	}

	return toReturn;
}
//=======================================================================
void statString(y86_stat_t stat, char *name)
{
	switch(stat)
	{
		case AOK: strcpy(name, "AOK");
			break;

		case HLT: strcpy(name, "HLT");
			break;

		case ADR: strcpy(name, "ADR");
			break;

		case INS: strcpy(name, "INS");
			break;	
	}	
}



void dump_cpu( const y86_t *cpu ) 
{
    printf("dump of Y86 CPU:\n");
	char *status = malloc(sizeof(char) * 4);
	statString(cpu->stat, status);
	printf("  %%rip: %016lx   flags: SF%i ZF%i OF%i  %s\n", cpu->pc, cpu->sf, cpu->zf, cpu->of, status);
	free(status);

	printf("  %%rax: %016lx    %%rcx: %016lx\n", cpu->rax, cpu->rcx);
	printf("  %%rdx: %016lx    %%rbx: %016lx\n", cpu->rdx, cpu->rbx);
	printf("  %%rsp: %016lx    %%rbp: %016lx\n", cpu->rsp, cpu->rbp);
	printf("  %%rsi: %016lx    %%rdi: %016lx\n", cpu->rsi, cpu->rdi);
	printf("   %%r8: %016lx     %%r9: %016lx\n", cpu->r8, cpu->r9);
	printf("  %%r10: %016lx    %%r11: %016lx\n", cpu->r10, cpu->r11);
	printf("  %%r12: %016lx    %%r13: %016lx\n", cpu->r12, cpu->r13);
	printf("  %%r14: %016lx\n\n", cpu->r14);
}






//=======================================================================
// Decode and execude helper methods.

void haltDE(y86_t *cpu)
{
	cpu->stat = HLT;
	cpu->zf = false;
	cpu->sf = false;
	cpu->of = false;
}

void nopDE() // Nothing happens in this stage, but it's here for visual consistency
{
}

y86_register_t irmovDE(y86_t *cpu, const y86_inst_t *inst)
{
	return inst->value;
}

y86_register_t opDE(y86_t *cpu, const y86_inst_t *inst, y86_register_t *valA)
{
	valA = cpuRegister(cpu, inst->ra);
	y86_register_t *valB = cpuRegister(cpu, inst->rb);
	int64_t tempE = 0;
	cpu->of = false;	

	switch(inst->op)
	{
		case ADD:
			tempE = (int64_t)*valB + (int64_t)*valA;
			if (((int64_t)*valB > 0 && (int64_t)*valA > 0 && tempE <= 0)
				|| ((int64_t)*valB < 0 && (int64_t)*valA < 0 && tempE >= 0))
			{
				cpu->of = true;
			}
			
			break;
 
		case SUB:
			tempE = (int64_t)*valB - (int64_t)*valA;

			if (((int64_t)*valA > 0 && !(tempE < (int64_t)*valB)) // if A > 0, E < B or overflow
				|| ((int64_t)*valA < 0 && !(tempE > (int64_t)*valB))) //if A < 0, E > B or of
			{
				cpu->of = true;
			}
			
			break;

		case XOR:
			tempE = (int64_t)*valB ^ (int64_t)*valA;
			break;

		case AND:
			tempE = (int64_t)*valB & (int64_t)*valA;
			break;

		default:
			cpu->stat = INS;
			printf("I dunno lmao\n");
			break;

	}
	cpu->sf = tempE < 0;
	cpu->zf = tempE == 0;
	
	return tempE;
}

y86_register_t rmmovDE(y86_t *cpu, const y86_inst_t *inst, y86_register_t *valA)
{
	*valA = *cpuRegister(cpu, inst->ra);
	y86_register_t valB;
	if (inst->rb != BADREG)
	{
		valB = *cpuRegister(cpu, inst->rb);

	}
	else
	{
		valB = 0;
	}
	return valB + inst->d;
}

y86_register_t mrmovDE(y86_t *cpu, const y86_inst_t *inst)
{
	y86_register_t valB;
	if (inst->rb != BADREG)
	{
		valB = *cpuRegister(cpu, inst->rb);

	}
	else
	{
		valB = 0;
	}
	return valB + inst->d;
}

y86_register_t pushDE(y86_t *cpu, const y86_inst_t *inst, y86_register_t *valA)
{

	*valA = *cpuRegister(cpu, inst->ra);
	y86_register_t valB = cpu->rsp;
	return valB - 8;
}

y86_register_t popDE(y86_t *cpu, const y86_inst_t *inst, y86_register_t *valA)
{
	if (*cpuRegister(cpu, RSP) == 0)
	{
		cpu->stat = ADR;
		return 0;
	}
	*valA = cpu->rsp;
	y86_register_t valB = cpu->rsp;
	return valB + 8;
}

y86_register_t callDE(y86_t *cpu, const y86_inst_t *inst)
{
	if (*cpuRegister(cpu, RSP) == 0)
	{
		cpu->stat = ADR;
		return 0;
	}
	return (*cpuRegister(cpu, RSP)) - 8;
}

y86_register_t retDE(y86_t *cpu, const y86_inst_t *inst, y86_register_t *valA)
{
	*valA = *cpuRegister(cpu, RSP);
	y86_register_t valB = *cpuRegister(cpu, RSP);
	if (valB == 0)
	{
		cpu->stat = ADR;
		return 0;
	}
	return valB + 8;
}

y86_register_t cmovDE(y86_t *cpu, const y86_inst_t *inst, y86_register_t *valA, bool *cond)
{
	*valA = *cpuRegister(cpu, inst->ra);
	switch(inst->cmov)
	{
		case CMOVLE:
			*cond = cpu->zf || cpu->sf;
			break;
		case CMOVL:
			*cond = cpu->sf;
			break;
		case CMOVE:
			*cond = cpu->zf;
			break;
		case CMOVNE:
			*cond = !cpu->zf;
			break;
		case CMOVGE:
			*cond = cpu->zf || !cpu->sf;
			break;
		case CMOVG:
			*cond = !cpu->sf;
			break;
		case RRMOVQ:
			*cond = true;
			break;
		default:
			*cond = false;
			cpu->stat= INS;

	}
	return *valA;
}

void jumpDE(y86_t *cpu, const y86_inst_t *inst, bool *cond)
{
	switch(inst->jump)
	{
		case JMP:
			*cond = true;
			break;
		case JLE:
			*cond = cpu->zf || cpu->sf;
			break;
		case JL:
			*cond = cpu->sf;
			break;
		case JE:
			*cond = cpu->zf;
			break;
		case JNE:
			*cond = !cpu->zf;
			break;
		case JGE:
			*cond = cpu->zf || !cpu->sf;
			break;
		case JG:
			*cond = !cpu->sf;
			break;
		default:
			*cond = false;
			cpu->stat= INS;			
	}
}


y86_register_t decode_execute(  y86_t *cpu , bool *cond , const y86_inst_t *inst ,
                                y86_register_t *valA 
                             )
{
	y86_register_t valE = 0;
	if (cpu == NULL || cond == NULL || inst == NULL || valA == NULL)
	{
		cpu->stat = INS;
	}
	else
	{

		switch(inst->type)
		{
			case HALT:
				haltDE(cpu);
				break;

			case NOP:
				nopDE();
				break;

			case IRMOVQ:
				valE = irmovDE(cpu, inst);
				break;

			case OPQ:
				valE = opDE(cpu, inst, valA);
				break;

			case RMMOVQ:
				valE = rmmovDE(cpu, inst, valA);
				if (cpu->stat != ADR) printf("Memory write to 0x%04lx: 0x%lx\n", valE, *valA);
				break;

			case MRMOVQ:
				valE = mrmovDE(cpu, inst);
				break;
				
			case PUSHQ:
				valE = pushDE(cpu, inst, valA);	
				if (cpu->stat != ADR) printf("Memory write to 0x%04lx: 0x%lx\n", valE, *valA);
				break;

			case POPQ:
				valE = popDE(cpu, inst, valA);
				break;	

			case CALL:
				valE = callDE(cpu, inst);
				if (cpu->stat != ADR) printf("Memory write to 0x%04lx: 0x%lx\n", valE, cpu->pc + 9);
				break;

			case RET:
				valE = retDE(cpu, inst, valA);
				break;

			case CMOV:
				valE = cmovDE(cpu, inst, valA, cond);
				break;

			case JUMP:
				jumpDE(cpu, inst, cond);
				break;

			default:
				cpu->stat = INS;
				break;	
		}
	}
    return valE;
}




//=======================================================================

// Memory, write-back, and program counter update helper methods.
void haltWB(y86_t *cpu, const y86_inst_t *inst)
{
	cpu->pc = cpu->pc + inst->size; // This is in most helper methods instead of being outside because not every instruction updates the pc in this way, just most of them.
}

void nopWB(y86_t *cpu, const y86_inst_t *inst)
{
	cpu->pc = cpu->pc + inst->size;
}

void irmovWB(y86_t *cpu, const y86_inst_t *inst, y86_register_t valE)
{
	y86_register_t *rb = cpuRegister(cpu, inst->rb);
	*rb = valE;
	cpu->pc = cpu->pc + inst->size;
}

void opWB(y86_t *cpu, const y86_inst_t *inst, y86_register_t valE)
{
	y86_register_t *rb = cpuRegister(cpu, inst->rb);
	*rb = valE;
	cpu->pc = cpu->pc + inst->size;
}

void rmmovWB(y86_t *cpu, memory_t memory, const y86_inst_t *inst, y86_register_t valE, y86_register_t valA)
{
	if (valE >= MEMSIZE)
	{
		cpu->stat = INS;
		cpu->pc = cpu->pc + inst->size;
	}
	else
	{
		memcpy(memory + valE, &valA, 8);
		cpu->pc = cpu->pc + inst->size;
	}
}

void mrmovWB(y86_t *cpu, memory_t memory, const y86_inst_t *inst, y86_register_t valE)
{
    // printf("reading memory from 0x%04lx\n", valE);

	
	if (valE >= MEMSIZE)
	{
		cpu->stat = ADR;
		cpu->pc = cpu->pc + inst->size;
	}
	else
	{
		y86_register_t *reg = cpuRegister(cpu, inst->ra);
		memcpy(reg, memory + valE, 8);
		cpu->pc = cpu->pc + inst->size;
	}
}

void pushWB(y86_t *cpu, memory_t memory, const y86_inst_t *inst, y86_register_t valE, y86_register_t valA)
{
	memcpy(memory + valE, &valA, 8);
	cpu->rsp = valE;
	cpu->pc = cpu->pc + inst->size;
}


void popWB(y86_t *cpu, memory_t memory, const y86_inst_t *inst, 
			y86_register_t valE, y86_register_t valA)
{
	if (cpu->stat != ADR)
	{
		y86_register_t *reg = cpuRegister(cpu, inst->ra);
		memcpy(reg, memory + valA, 8);
		cpu->rsp = valE;
	}
	cpu->pc = cpu->pc + inst->size;
}

void callWB(y86_t *cpu, memory_t memory, const y86_inst_t *inst, 
			y86_register_t valE)
{
	if (cpu->stat != ADR)
	{
		y86_register_t valP = cpu->pc + 9;
		memcpy(memory + valE, &valP, 8);
		y86_register_t *reg = cpuRegister(cpu, RSP);
		*reg = valE;
	}
	cpu->pc = inst->dest;
}

void retWB(y86_t *cpu, memory_t memory, const y86_inst_t *inst, 
			y86_register_t valE, y86_register_t valA)
{
	if (cpu->stat != ADR)
	{
		y86_register_t valM = 0;
		memcpy(&valM, memory + valA, 8);
		y86_register_t *reg = cpuRegister(cpu, RSP);
		*reg = valE;
		cpu->pc = valM;
	}
	
}

void cmovWB(y86_t *cpu, memory_t memory, const y86_inst_t *inst, 
			y86_register_t valE, bool cond)
{
	if (cond)
	{
		*cpuRegister(cpu, inst->rb) = valE;
	}
	cpu->pc = cpu->pc + inst->size;
}		

void jumpWB(y86_t *cpu, const y86_inst_t *inst, bool cond)
{
	if (cond)
	{
		cpu->pc = inst->dest; 
	}
	else
	{
		cpu->pc = cpu->pc + inst->size;
	}
}

void memory_wb_pc(  y86_t *cpu , memory_t memory , bool cond , 
                    const y86_inst_t *inst , y86_register_t valE , 
                    y86_register_t  valA 
                 )
{
    switch(inst->type)
	{
		case HALT:
			haltWB(cpu, inst);
			break;

		case NOP:
			nopWB(cpu, inst);
			break;	

		case IRMOVQ:
			irmovWB(cpu, inst, valE);
			break;

		case OPQ:
			opWB(cpu, inst, valE);
			break;

		case RMMOVQ:
			rmmovWB(cpu, memory, inst, valE, valA);
			break;

		case MRMOVQ:
			mrmovWB(cpu, memory, inst, valE);
			break;

		case PUSHQ:
			pushWB(cpu, memory, inst, valE, valA);
			break;

		case POPQ:
			popWB(cpu, memory, inst, valE, valA);
			break;

		case CALL:
			callWB(cpu, memory, inst, valE);
			break;

		case RET:
			retWB(cpu, memory, inst, valE, valA);
			break;

		case CMOV:
			cmovWB(cpu, memory, inst, valE, cond);
			break;
		
		case JUMP:
			jumpWB(cpu, inst, cond);
			break;

		default:
			cpu->stat = ADR;
			break;
	}

}

