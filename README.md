Memory Hierarchy Performance Simulator
This repository contains a simulator to analyze the performance of memory hierarchies, specifically focusing on branch prediction using different predictors: local, global, and tournament. The simulator reads from a trace file and evaluates the prediction accuracy.

Files
config.c
This file contains macro definitions used throughout the simulator.

MemoryHierarchySimulator.c
This file contains the main implementation of the memory hierarchy performance simulator.

Compilation and Execution

1.Compile: Use the following command to compile the code:
![image](https://github.com/user-attachments/assets/ada77c64-80e4-404e-a33d-dd76138b3fd8)
2.Run: Execute the compiled program:
![image](https://github.com/user-attachments/assets/eec60777-464c-4f3e-98fa-6c813ba95dc6)
Configuration
The config.c file contains various configuration parameters that can be adjusted according to your needs:

ASSOCIAT_AMOUNT: Number of associativity ways.
BHR_BITS: Number of bits for the branch history register.
FSM_SIZE: Size of the finite state machine.
GHR_BITS: Number of bits for the global history register.
FSM_SIZE_SHARED: Size of the shared finite state machine.
ENTRIES: Number of entries.
SET_SIZE: Size of each set.
IS_PRIVATE: Flag indicating if the predictor is private.
IS_LOCAL: Flag indicating if the predictor is local.
FILE_NAME: Name of the trace file.
FILE_PATH: Path to the trace file.

Output
The simulator provides various statistics, including:

Total instruction count
Total branch instruction count
Miss count and miss prediction rate for local, global, and tournament predictors
License
This project is licensed under the MIT License.
