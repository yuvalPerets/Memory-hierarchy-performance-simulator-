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
- `IS_PRIVATE` and `IS_LOCAL`: Flags to indicate if the predictor is using private or local state machines.
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

## Configuration

You can customize the simulation by modifying the parameters in the `config.c` file. Key parameters include:

- `ASSOCIAT_AMOUNT`: Set the cache associativity. For example, you can set it to 4 for a 4-way set associative cache.
- `BHR_BITS`: Configure the size of the Branch History Register (BHR). For instance, setting it to 8 will use an 8-bit BHR.
- `FSM_SIZE`: Define the size of the Finite State Machine (FSM) for local prediction. For example, you can set it to 256 for a 256-state FSM.
- `GHR_BITS`: Configure the size of the Global History Register (GHR). For instance, setting it to 12 will use a 12-bit GHR.
- `FSM_SIZE_SHARED`: Define the size of the FSM for global prediction. For example, you can set it to 128 for a 128-state FSM.
- `ENTRIES`: Set the number of entries in the cache. For instance, setting it to 1024 will use a 1024-entry cache.
- `SET_SIZE`: Define the size of each set in the cache. For example, you can set it to 64 for a 64-set cache.
- `IS_PRIVATE`: Toggle between private and shared predictors. Set to 1 for private, 0 for shared.
- `IS_LOCAL`: Toggle between local and global predictors. Set to 1 for local, 0 for global.
- `FILE_NAME` and `FILE_PATH`: Specify the name and path of the trace file used for simulation. For example, you can set `FILE_NAME` to `"trace.txt"` and `FILE_PATH` to `"/path/to/trace.txt"`.

To apply these configurations, modify the respective definitions in the `config.c` file before compiling the simulator.

## License

This project is licensed under the MIT License. See the LICENSE file for details.

