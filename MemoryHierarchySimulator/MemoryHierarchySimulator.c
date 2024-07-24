#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

#ifndef CONFIG_H
#include "Config.c"  // Include config.c to access the define
#endif

// Define constants
#define MAX_LINE_LENGTH 110

int miss_count = 0;
int branch_instruction_count = 0;
long long int instruction_count = 0;

int global_miss_count = 0;
int tournament_miss_count = 0;

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
    Set entries[SET_SIZE];
}Way;

typedef struct {
    Way ways[ASSOCIAT_AMOUNT];
    bool lru[SET_SIZE];
    int is_private;
    Door_State_t shared_fsm[FSM_SIZE];
}L_predictor;

typedef struct {
    unsigned int ghr : GHR_BITS;
    Door_State_t global_fsm[FSM_SIZE_SHARED];
}G_predictor;

typedef struct {
    L_predictor* local_predictor;
    G_predictor* global_predictor;
    Door_State_t chooser_fsm[SET_SIZE];
}T_predictor;

void init_Sets(Set* set) {
    set->v = 0;
    set->bhr = 0;
    set->tag = 0;
    set->beq_index = 0;
    // we can consider not init for global fsm
    for(int i=0;i<FSM_SIZE;i++)
    set->local_fsm[i] = WEAKLY_NOT_TAKEN;
}

void init_Way(Way* way) {
    for (int i = 0; i < SET_SIZE; i++) {
        init_Sets(&way->entries[i]);
    }
}

void init_L_predictor(L_predictor* predictor) {
    // Set default values for members
    for (int i = 0; i < ASSOCIAT_AMOUNT ; i++) {
        init_Way(&predictor->ways[i]);
    }
    for (int i = 0; i < SET_SIZE; i++) {
        predictor->lru[i] = 0;
    }
    predictor->is_private = IS_PRIVATE;
    //we can also init only when set not private fsm
    for (int i = 0; i < FSM_SIZE; i++)
        predictor->shared_fsm[i] = WEAKLY_NOT_TAKEN;// Default initial state
}

void init_G_predictor(G_predictor* predictor) {
    predictor->ghr = 0;
    for (int i = 0; i < FSM_SIZE_SHARED; i++)
        predictor->global_fsm[i] = WEAKLY_NOT_TAKEN;
}

void init_T_predictor(T_predictor* predictor) {
    predictor->local_predictor = NULL;
    predictor->global_predictor = NULL;
    for (int i = 0; i < SET_SIZE; i++)
        predictor->chooser_fsm[i] = WEAKLY_NOT_TAKEN;
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


int get_index(char* address) {
    int addr = strtol(address, NULL, 16);
    int temp = SET_SIZE - 1;
    return (addr >> 2) & (SET_SIZE - 1);
    //address/4 because the 2 lsb are allways zeros, and we dont taking them into account for the index.
    //for example entiries : (1024)10 = (10000000000)2 -> 1024-1 = ( 1111111111)2 , then the & should only leave us with the index bits.
}

int get_tag(char* address) {
    int addr = strtol(address, NULL, 16);
    return (addr & (0xFFFFFFFF - (SET_SIZE << 2 - 1))) / (SET_SIZE << 2);
    // entries * 4 -  1 creates a mask to make all the index bits as zero
    // the divion of entries * 4 does a shift right in the same amount of index bits.
}

void increase_fsm(Door_State_t* fsm)
{
    if (*fsm != 3)// unless strongly taken -> fsm + 1 
        *fsm += 1;
}

void decrease_fsm(Door_State_t* fsm)
{
    if (*fsm != 0)// unless strongly not taken -> fsm - 1 
        *fsm -= 1;
}

void clear_screen() {
    // Clear the screen based on the operating system
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void display_progress(int progress, int total) {
    int bar_width = 50;
    float progress_percentage = (float)progress / total;
    printf("file name: %s\n", FILE_NAME);
    printf("is private: %d\n",IS_PRIVATE);
    printf("is local: %d\n", IS_LOCAL);
    printf("Entries: %d\n", ENTRIES);
    printf("history size: %d\n", BHR_BITS);
    printf("fsm size: %d\n", FSM_SIZE);
    printf("global history: %d\n", GHR_BITS);
    printf("[");
    int pos = bar_width * progress_percentage;
    for (int i = 0; i < bar_width; ++i) {
        if (i < pos) printf("=");
        else if (i == pos) printf(">");
        else printf(" ");
    }
    printf("] %d%%\r", (int)(progress_percentage * 100));

    fflush(stdout);
}

//function that handles all the logic for the local predicotr .
int mapping(Instruction* current_inst, L_predictor* local_predictor) {
    //split the index and tag name from the address.
    int idx = get_index(current_inst->address);
    int tag= get_tag(current_inst->address);
    Set* set;
    Way* way = &local_predictor->ways[local_predictor->lru[idx]];
    //logic for local predictor behavior.
    //run on each way separately.
    for (int i = 0; i < ASSOCIAT_AMOUNT; i++) {
         set= &local_predictor->ways[i].entries[idx];
        if (set->v) {
            if (set->tag==tag) {
                // need to check if we are using local or shared fsm
                if (local_predictor->is_private) {
                    if (set->local_fsm[set->bhr] >> 1) { //check the msb of local state machine to make a decision.
                        // (11 || 10 ) >> 1  --> 1 . true  ||  ( 00 || 01 ) >> 1  --> 0 . false

                        if (current_inst->branch_taken) {//correct predict taken.
                            increase_fsm(&(set->local_fsm[set->bhr]));
                        }
                        else //incorrect predict taken.
                        {
                            decrease_fsm(&(set->local_fsm[set->bhr]));
                            miss_count++; // we predict taken (msb of local fsm is 1 ) but the branch wasn't taken
                        }
                    }
                    else {
                        if (!current_inst->branch_taken) {//correct predict not-taken.
                            decrease_fsm(&(set->local_fsm[set->bhr]));
                        }
                        else //incorrect predict not-taken.
                        {
                            increase_fsm(&(set->local_fsm[set->bhr]));
                            miss_count++;
                        }
                    }
                }
                else {
                    if (local_predictor->shared_fsm[set->bhr] >> 1) { //check the msb of local state machine to make a decision.
                       // (11 || 10 ) >> 1  --> 1 . true  ||  ( 00 || 01 ) >> 1  --> 0 . false

                        if (current_inst->branch_taken) {//correct predict taken.
                            increase_fsm(&(local_predictor->shared_fsm[set->bhr]));
                        }
                        else //incorrect predict taken.
                        {
                            decrease_fsm(&(local_predictor->shared_fsm[set->bhr]));
                            miss_count++; // we predict taken (msb of local fsm is 1 ) but the branch wasn't taken
                        }
                    }
                    else {
                        if (!current_inst->branch_taken) {//correct predict not-taken.
                            decrease_fsm(&(local_predictor->shared_fsm[set->bhr]));
                        }
                        else //incorrect predict not-taken.
                        {
                            increase_fsm(&(local_predictor->shared_fsm[set->bhr]));
                            miss_count++;
                        }
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
                       if (local_predictor->is_private) {
                           increase_fsm(&(set->local_fsm[set->bhr]));
                       }
                       else {
                           increase_fsm(&(local_predictor->shared_fsm[set->bhr]));
                       }
                   //change the lru to the second way.
                   
                   way->entries[idx].tag = tag;
                   way->entries[idx].v = 1;
                   local_predictor->ways[local_predictor->lru[idx]] = *way;
                   local_predictor->lru[idx] = !local_predictor->lru[idx];
                   return 1;
                }
            }
        }
        else { //setup empty way.
            if (current_inst->branch_taken) {
                if (local_predictor->is_private)
                    set->local_fsm[set->bhr] += 1;
                else
                    local_predictor->shared_fsm[set->bhr] += 1;
                miss_count++;
                set->bhr = (set->bhr << 1) + current_inst->branch_taken;
                local_predictor->lru[idx] = 1 - i;
                set->tag = tag;
                set->v = 1;
            }
            return 1;
        }
        
    }
}

int Global_mapping(Instruction* current_inst, G_predictor* global_predictor) {
    //checking msb for the fsm . getting the correct one from the global preditor ghr
    //after we get our result we need to update according to if the branch was taken or not
    if (global_predictor->global_fsm[global_predictor->ghr] >> 1) { // checking the msb
        if (current_inst->branch_taken)
            increase_fsm(&global_predictor->global_fsm[global_predictor->ghr]);
        else {
            global_miss_count++;
            decrease_fsm(&global_predictor->global_fsm[global_predictor->ghr]);
        }
    }
    else {
        if (current_inst->branch_taken) {
            global_miss_count++;
            increase_fsm(&global_predictor->global_fsm[global_predictor->ghr]);
        }else
            decrease_fsm(&global_predictor->global_fsm[global_predictor->ghr]);
    }
    // now updtaing the ghr , we should shift left all the ghr and add if the branch was taken or not
    global_predictor->ghr = (global_predictor->ghr << 1) + current_inst->branch_taken;
}

//we are taking the miss count for the predictor of the tournament is going to use.
int Tournament_mapping(int idx,T_predictor* tournament_predictor,int temp_m_count,int temp_g_m_count) {
    
    if (tournament_predictor->chooser_fsm[idx] >> 1) {
        //we are getting the index of the instruction to get a correct fsm from the chooser
        // (11 || 10 ) >> 1  --> 1 . local predictor  ||  ( 00 || 01 ) >> 1  --> 0 . global predictor
        if (temp_m_count != miss_count) {
            //if local predictor was wrong
            tournament_miss_count++;
            if (temp_g_m_count == global_miss_count) {
                //global predictor was right
                decrease_fsm(&tournament_predictor->chooser_fsm[idx]);
            }
        }
        else {
            if (temp_g_m_count != global_miss_count) {
                    //global predictor was wrong
                    increase_fsm(&tournament_predictor->chooser_fsm[idx]);    
            }
        }

    }
    else {
        if (temp_g_m_count != global_miss_count) {
            //if global predictor was wrong
            tournament_miss_count++;
            if (temp_m_count == miss_count) {
                //local predictor was right
                increase_fsm(&tournament_predictor->chooser_fsm[idx]);
            }
        }
        else {
            if (temp_m_count != miss_count) {
                //local predictor was wrong
                decrease_fsm(&tournament_predictor->chooser_fsm[idx]);
            }
        }
    }
}

void print_results() {
    printf("instruction_count: %lld\n", instruction_count);
    printf("branch_instruction_count: %d\n", branch_instruction_count);
    printf("prints for local predictor\n");
    printf("miss_count: %d\n", miss_count);
    printf("miss predict: %.2f%%\n", ((float)miss_count / branch_instruction_count) * 100);
    printf("local mpi: %.2f%%\n", ((float)miss_count / instruction_count) * 100);
    printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
    printf("prints for global predictor\n");
    printf("showing the miss rate for global precitor : ");
    printf("%.2f%%\n", ((float)global_miss_count / branch_instruction_count) * 100);
    printf("global mpi: %.2f%%\n", ((float)global_miss_count / instruction_count) * 100);
    printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
    printf("prints for tournament predictor\n");
    printf("showing the miss rate for tournament precitor : ");
    printf("%.2f%%\n", ((float)tournament_miss_count / branch_instruction_count) * 100);
    printf("tournament mpi: %.2f%%\n", ((float)tournament_miss_count / instruction_count) * 100);
}

// Main function
int main() {
    display_progress(0,9999999);
    FILE* trace_file;
    char line[MAX_LINE_LENGTH];
    Instruction current_inst;
    Instruction next_inst;
    L_predictor local_predictor;
    G_predictor global_predictor;
    T_predictor tournament_predictor;
    char full_path[300];
    snprintf(full_path, sizeof(full_path), "%s%s", FILE_PATH, FILE_NAME);
    int idx;
    int temp_m_count;
    int temp_g_m_count;
    init_L_predictor(&local_predictor);
    init_G_predictor(&global_predictor);
    init_T_predictor(&tournament_predictor);
    tournament_predictor.global_predictor = &global_predictor;
    tournament_predictor.local_predictor = &local_predictor;

    // Open trace file
    trace_file = fopen(full_path, "r");
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
                        //function takes miss rate for the current tournament prediction
                        idx= get_index(current_inst.address);
                        temp_m_count = miss_count;
                        temp_g_m_count = global_miss_count;
                        mapping(&current_inst,&local_predictor);
                        Global_mapping(&current_inst, &global_predictor);
                        Tournament_mapping(idx,&tournament_predictor,temp_m_count,temp_g_m_count);
                        // Print the branch instruction
                        //printf("Branch Instruction: %s %s %s |", current_inst.address, current_inst.mnemonic, current_inst.operands);
                        //printf("Branch Taken: %d\n", current_inst.branch_taken);
                    }
                    if (instruction_count % 300000 == 0) {
                        clear_screen();
                        display_progress(instruction_count, 9999999);
                    }
                    // Simulate instruction execution
                    // Move to the next instruction
                    current_inst = next_inst;
                }
            }
        }
    }
    printf("\n");
    // Close trace file
    fclose(trace_file);
    print_results();
    return 0;
}