#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Define constants
#define MAX_LINE_LENGTH 256

// Struct for an instruction
typedef struct {
    char address[19];
    char mnemonic[MAX_LINE_LENGTH];
    char operands[MAX_LINE_LENGTH];
    int is_branch;
    int branch_taken;
} Instruction;

// Struct for a register file
typedef struct {
    int value;
    int ready;
} Register;

// Struct for the reorder buffer entry
typedef struct {
    Instruction inst;
    int exec_cycle;
} ReorderBufferEntry;



// Function to read a trace file
int read_trace_file(const char* filename) {
    FILE* file = fopen(filename, "r");
    int count = 0;
    if (file == NULL) {
        perror("Error opening file");
        
        return -1;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        //printf("%s", line);
        //count++;
        // Process each line (representing a memory access trace)
    }
   // printf("%d", count);

    fclose(file);
    return 0;
}

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


    /*
    const char* trace_file = "D:\\הנדסת מחשבים\\שנה 4\\סמסטר ב\\תכן מתקדים של מעגלים ספרתיים\\RiscV traces with register values\\linkpack_val_mini.trc";

    if (read_trace_file(trace_file) != 0) {
        fprintf(stderr, "Failed to read the trace file.\n");
        return 1;
    }
*/
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