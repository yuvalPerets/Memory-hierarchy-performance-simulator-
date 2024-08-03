# Memory Hierarchy Performance Simulator

This project is a performance simulator for branch prediction in a memory hierarchy. The simulator is implemented in C and consists of two main files: `config.c` and `MemoryHierarchySimulator.c`.

## Files

### config.c

This file contains configuration definitions for the simulator. It includes macro definitions that configure various parameters such as:

- `ASSOCIAT_AMOUNT`: Defines the associativity of the cache.
- `BHR_BITS`: Defines the number of bits used for the Branch History Register (BHR).
- `FSM_SIZE`: Defines the size of the Finite State Machine (FSM) for local prediction.
- `GHR_BITS`: Defines the number of bits used for the Global History Register (GHR).
- `FSM_SIZE_SHARED`: Defines the size of the FSM for global prediction.
- `ENTRIES`: Defines the number of entries in the cache.
- `SET_SIZE`: Defines the size of each set in the cache.
- `IS_PRIVATE` and `IS_LOCAL`: Flags to indicate if the predictor is private or local.
- `FILE_NAME` and `FILE_PATH`: Define the name and path of the trace file used for simulation.

### MemoryHierarchySimulator.c

This file contains the main logic for the simulator, including data structures, initialization functions, and the main simulation loop.

#### Key Components

1. **Constants and Global Variables**:
   - Define various constants and global variables used throughout the simulator, such as counters for miss counts and instruction counts.

2. **Enumerations and Structs**:
   - `Door_State_t`: An enumeration representing the state of the branch predictor (e.g., strongly taken, weakly taken).
   - `Set`, `Way`, `L_predictor`, `G_predictor`, `T_predictor`: Structs representing different components of the predictor.

3. **Initialization Functions**:
   - Functions to initialize the various components of the predictor (e.g., `init_Sets`, `init_Way`, `init_L_predictor`, `init_G_predictor`, `init_T_predictor`).

4. **Instruction Parsing**:
   - Functions to parse the trace file and extract instruction information.

5. **Branch Prediction Logic**:
   - Functions to implement the branch prediction logic for local, global, and tournament predictors (e.g., `mapping`, `Global_mapping`, `Tournament_mapping`).

6. **Simulation Loop**:
   - The main loop that reads instructions from the trace file, simulates the execution, and updates the predictors accordingly.

7. **Utility Functions**:
   - Various utility functions for tasks such as clearing the screen and displaying progress.

8. **Results Printing**:
   - Function to print the simulation results, including miss rates and other statistics.

## How to Run

1. Ensure you have a C compiler installed on your system.
2. Clone the repository.
3. Modify the `config.c` file to set the desired configuration parameters.
4. Compile the code using your C compiler. For example:
   ```sh
   gcc MemoryHierarchySimulator.c -o simulator

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
