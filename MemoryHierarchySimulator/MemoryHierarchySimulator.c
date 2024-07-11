﻿#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

#ifndef CONFIG_H
#include "Config.c"  // Include config.c to access the define
#endif

// Define constants
#define MAX_LINE_LENGTH 256

int miss_count = 0;
int branch_instruction_count = 0;
long long int instruction_count = 0;

typedef enum {
    STRONGLY_NOT_TAKEN = 0,
    WEAKLY_NOT_TAKEN = 1,
    WEAKLY_TAKEN = 2,
    STRONGLY_TAKEN = 3
} Door_State_t;  // Use a clear and meaningful name

typedef struct {
    int v ;
    unsigned int bhr : BHR_BITS;
    int tag;
    int beq_index;
    Door_State_t local_fsm[FSM_SIZE];
}Set;

typedef struct {
    Set entries[ENTRIES];
}Way;

typedef struct {
    Way ways[ASSOCIAT_AMOUNT];
    bool lru[ENTRIES];
    int is_private;
    Door_State_t global_fsm;

}L_predictor;

void init_Sets(Set* set) {
    set->v = 0;
    set->bhr = 0;
    set->tag = 0;
    set->beq_index = 0;
    for(int i=0;i<FSM_SIZE;i++)
    set->local_fsm[i] = WEAKLY_NOT_TAKEN;
}

void init_Way(Way* way) {
    for (int i = 0; i < ENTRIES; i++) {
        init_Sets(&way->entries[i]);
    }
}

void init_L_predictor(L_predictor* predictor) {
    // Set default values for members
    for (int i = 0; i < ASSOCIAT_AMOUNT; i++) {
        init_Way(&predictor->ways[i]);
    }
    for (int i = 0; i < ENTRIES; i++) {
        predictor->lru[i] = 0;
    }
    predictor->is_private = IS_PRIVATE;  // Default to not private
    predictor->global_fsm = WEAKLY_NOT_TAKEN;  // Default initial state
}

// Struct for an instruction
typedef struct {
    char address[9];
    char mnemonic[MAX_LINE_LENGTH];
    char operands[MAX_LINE_LENGTH];
    int is_branch;
    int branch_taken;
} Instruction;

// Function to parse a line from the trace file
int parse_line(char* line, Instruction* inst) {
    // taking from the line the opperand name and the pc address of the instruction.
    if (sscanf(line, "Info '%*[^']', 0x00000000 %8s %*[^:]: %s %s", inst->address, inst->mnemonic, inst->operands) == 3) {
        // Add the null terminator to inst->address
        inst->address[8] = '\0';
        //all and only brach operands starts with "b".
        if (inst->operands[0] == 'b') {
            inst->is_branch = 1;
            // Assuming branch not taken unless proven otherwise
            inst->branch_taken = 0;
        }
        else {
            inst->is_branch = 0;
        }
        return 1;
    }
    return 0;
}

// Function to check if branch is taken
int is_branch_taken(char* current_address, char* next_address) {
    long curr_addr = strtol(current_address, NULL, 16);
    long next_addr = strtol(next_address, NULL, 16);
    //if brach is taken the next address shouldn't be a jump of 4.
    return next_addr != (curr_addr + 4);
}

int mapping(Instruction* current_inst, L_predictor* local_predictor) {
    //split the index and tag name from the address.
    int idx = get_index(current_inst->address);
    int tag= get_tag(current_inst->address);
    Set* set;
    Way* way = &local_predictor->ways[local_predictor->lru[idx]];
    //run on each way separately.
    for (int i = 0; i < ASSOCIAT_AMOUNT; i++) {
         set= &local_predictor->ways[i].entries[idx];
        if (set->v) {
            if (set->tag==tag) {
                if (set->local_fsm[set->bhr] >> 1) { //check the msb of local state machine to make a decision.
                    if (current_inst->branch_taken) {//correct predict taken.
                        if (set->local_fsm[set->bhr] != 3)
                        set->local_fsm[set->bhr] += 1;
                    }
                    else //incorrect predict taken.
                    {
                        if (set->local_fsm[set->bhr] != 0)
                        set->local_fsm[set->bhr] -= 1;
                        miss_count++;
                    }
                }
                else {
                    if (!current_inst->branch_taken) {//correct predict not-taken.
                        if (set->local_fsm[set->bhr] != 0)
                        set->local_fsm[set->bhr] -= 1;
                    }
                    else //incorrect predict not-taken.
                    {
                        if (set->local_fsm[set->bhr] != 3)
                        set->local_fsm[set->bhr] += 1;
                        miss_count++;
                    }
                }
                set->bhr = (set->bhr << 1) + current_inst->branch_taken;
                //set the lru
                local_predictor->lru[idx] = 1 - i;
                return 1;
            }
            else
            {
                //both ways are valid but tags are different from current.
                //and implementing lru replacement.
                if (i == 1 & current_inst->branch_taken) {
                    miss_count++;
                    //get the lru way.
                   way = &local_predictor->ways[local_predictor->lru[idx]];
                   init_Sets(&way->entries[idx]);
                       way->entries[idx].bhr +=1;
                       if(set->local_fsm[set->bhr]!=3)
                       set->local_fsm[set->bhr] += 1;
                   //change the lru to the second way.
                   local_predictor->lru[idx] = !local_predictor->lru[idx];
                   local_predictor->ways[i] = *way;
                   printf("in way 1\n");
                   return 1;
                }
            }
        }
        else { //setup empty way.
            if (current_inst->branch_taken) {
                set->local_fsm[set->bhr] += 1;
                miss_count++;
                set->bhr = (set->bhr << 1) + current_inst->branch_taken;
                local_predictor->lru[idx] = 1 - i;
                set->tag = tag;
                set->v = 1;
            }
            return 1;
        }
        //local_predictor->ways[i].entries[idx] = *set;
        
    }
}

int get_index(char* address) {
    int addr = strtol(address, NULL, 16);
    return (addr / 4) & (ENTRIES - 1);
    //address/4 because the 2 lsb are allways zeros, and we dont taking them into account for the index.
    //for example entiries : (1024)10 = (10000000000)2 -> 1024-1 = ( 1111111111)2 , then the & should only leave us with the index bits.
}

int get_tag(char* address) {
    int addr = strtol(address, NULL, 16);
    return (addr & (0xFFFFFFFF - (ENTRIES*4 - 1))) /(ENTRIES * 4);
    // entries * 4 -  1 creates a mask to make all the index bits as zero
    // the divion of entries * 4 does a shift right in the same amount of index bits.
}

// Main function
int main() {
    FILE* trace_file;
    char line[MAX_LINE_LENGTH];
    Instruction current_inst;
    Instruction next_inst;
    L_predictor local_predictor;
    init_L_predictor(&local_predictor);

    // Open trace file
    trace_file = fopen("E:\\downloads\\RiscV traces with register values\\RiscV traces with register values\\linpack_val.trc", "r");
    if (trace_file == NULL) {
        perror("Error opening trace file");
        return 1;
    }

 // Read first line
    if (fgets(line, sizeof(line), trace_file)) {
        if (parse_line(line, &current_inst)) {
            // Simulation loop
            while (fgets(line, sizeof(line), trace_file)) {
                if (parse_line(line, &next_inst)) {
                    instruction_count++;
                    // Check if current instruction is a branch and if it is taken
                    if (current_inst.is_branch) {
                        branch_instruction_count++;
                        current_inst.branch_taken = is_branch_taken(current_inst.address, next_inst.address);
                        mapping(&current_inst,&local_predictor);
                        // Print the branch instruction
                        //printf("Branch Instruction: %s %s %s |", current_inst.address, current_inst.mnemonic, current_inst.operands);
                        //printf("Branch Taken: %d\n", current_inst.branch_taken);
                        if (instruction_count % 100000 == 0) {
                            printf("yayyyy\n");
                        }
                    }
                    // Simulate instruction execution
                    // Move to the next instruction
                    current_inst = next_inst;
                }
            }
        }
    }

    // Close trace file
    fclose(trace_file);
    printf("miss_count: %d\n", miss_count);
    printf("branch_instruction_count: %d\n", branch_instruction_count);
    printf("instruction_count: %lld\n", instruction_count);
    printf("miss predict: %f\n", (float)miss_count/branch_instruction_count);
    printf("mpi: %f\n", (float)miss_count/ instruction_count);
    return 0;
}