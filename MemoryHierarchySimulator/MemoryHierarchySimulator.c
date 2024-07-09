#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef CONFIG_H
#include "Config.c"  // Include config.c to access the define
#endif

// Define constants
#define MAX_LINE_LENGTH 256
typedef enum {
    STRONGLY_NOT_TAKEN = 0,
    WEAKLY_NOT_TAKEN = 1,
    WEAKLY_TAKEN = 2,
    STRONGLY_TAKEN = 3
} Door_State_t;  // Use a clear and meaningful name

Door_State_t current_state = WEAKLY_NOT_TAKEN;  // Declare the initial state
typedef struct {
    int v ;
    unsigned int bhr : BHR_BITS;
    int tag;
    int beq_index;
    Door_State_t local_fsm;
}Set;

typedef struct {
    Set entries[ENTRIES];
}Way;

typedef struct {
    Way ways[ASSOCIAT_AMOUNT];
    int is_private;
    Door_State_t global_fsm;
}L_predictor;

void init_Sets(Set* set) {
    set->v = 0;
    set->bhr = 0;
    set->tag = 0;
    set->beq_index = 0;
    set->local_fsm = WEAKLY_NOT_TAKEN;
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
    predictor->is_private = IS_PRIVATE;  // Default to not private
    predictor->global_fsm = WEAKLY_NOT_TAKEN;  // Default initial state
}




// Struct for an instruction
typedef struct {
    char address[19];
    char mnemonic[MAX_LINE_LENGTH];
    char operands[MAX_LINE_LENGTH];
    int is_branch;
    int branch_taken;
} Instruction;

// Function to parse a line from the trace file
int parse_line(char* line, Instruction* inst) {
    if (sscanf(line, "Info '%*[^']', %18s %*[^:]: %s %s", inst->address, inst->mnemonic, inst->operands) == 3) {
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
    return next_addr != (curr_addr + 4);
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
    trace_file = fopen("D:\\הנדסת מחשבים\\שנה 4\\סמסטר ב\\ארכיטקטורות מחשבים מתקדמות\\RiscV traces with register values\\linkpack_val_mini.trc", "r");
    if (trace_file == NULL) {
        perror("Error opening trace file");
        return 1;
    }

    // Call other simulation functions here

 // Read first line
    if (fgets(line, sizeof(line), trace_file)) {
        if (parse_line(line, &current_inst)) {
            // Simulation loop
            while (fgets(line, sizeof(line), trace_file)) {
                if (parse_line(line, &next_inst)) {
                    // Check if current instruction is a branch and if it is taken
                    if (current_inst.is_branch) {
                        current_inst.branch_taken = is_branch_taken(current_inst.address, next_inst.address);
                        // Print the branch instruction
                        printf("Branch Instruction: %s %s %s |", current_inst.address, current_inst.mnemonic, current_inst.operands);
                        printf("Branch Taken: %d\n", current_inst.branch_taken);
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

    return 0;
}