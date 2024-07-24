#ifndef BHR_BITS_H
#define BHR_BITS_H

#define ASSOCIAT_AMOUNT 2

#define BHR_BITS 7  // Replace with the actual value you want

#define FSM_SIZE (1<<BHR_BITS)

#define GHR_BITS 7

#define FSM_SIZE_SHARED (1<<GHR_BITS)

#define ENTRIES 2048

#define SET_SIZE (ENTRIES / ASSOCIAT_AMOUNT)

#define IS_PRIVATE 1

#define IS_LOCAL 1

#define FILE_NAME "fibonacci_val.trc"

#define FILE_PATH "E:\\downloads\\RiscV traces with register values\\RiscV traces with register values\\"

#endif