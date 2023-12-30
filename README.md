# MIPS-Pipeline-Simulator
Cycle-accurate MIPS 5-Stage Pipeline Simulator in C++, emulating a subset of MIPS instructions. Tracks instruction execution cycle by cycle, models pipeline stages, hazards, and performance metrics. Ideal for understanding pipelined processor architectures and optimizations.

An example MIPS program is provided for the simulator as a text file “imem.txt”, which is used to initialize
the Instruction Memory. Each line of the file corresponds to a Byte stored in the Instruction Memory in binary format, with the first line at address 0, the next line at address 1, and so on. Four contiguous lines correspond to one whole instruction. Note that the words stored in memory are in “Big-Endian” format,
meaning that the most significant byte is stored first.

The Data Memory is initialized using the “dmem.txt” file. The format of the stored words is the same as
the Instruction Memory. As with the instruction memory, the data memory addresses also begin at 0 and
increment by one in each line.

The instructions that the simulator supports and their encodings are shown in Table 1. Note that all
instructions, except for “halt”, exist in the MIPS ISA. The [MIPS Green Sheet](https://inst.eecs.berkeley.edu/~cs61c/resources/MIPS_Green_Sheet.pdf) defines the semantics of each
instruction.

For the purposes of this project, we use the bne instruction, rather than the beq instruction from single cycle MIPS simulator. bne jumps to the branch address if 𝑅[𝑟𝑠] != 𝑅[𝑟𝑡] and jumps to PC+4 otherwise, i.e., if 𝑅[𝑟𝑠] = 𝑅[𝑟𝑡]. This is to make writing loops slightly easier.

![Table 1 - Instruction encodings for a reduced MIPS ISA](https://github.com/rugvedmhatre/MIPS-Pipeline-Simulator/blob/main/Table-1.png)

## Pipeline Structure

MIPS pipeline has the following five stages:

1. **Fetch (IF):** Fetches an instruction from the instruction memory. Updates PC.

2. **Decode (ID/RF):** Reads from the register RF and generates control signals required in subsequent
stages. In addition, branches are resolved in this stage by checking for the branch condition and
computing the effective address.

3. **Execute (EX):** Performs an ALU operation.

4. **Memory (MEM):** Loads or stores a 32-bit word from data memory.

5. **Writeback (WB):** Writes back data to the RF.

Each pipeline stage takes inputs from flip-flops. The input flip-flops for each pipeline stage are described in the tables below. 

### IF Stage Input Flip-Flops

| Flip-Flop Name | Bit-width | Functionality |
|----------------|-----------|---------------|
| PC | 32 | Current value of PC |
| Nop | 1	| If set, IF stage performs a nop |

### IF/ID Stage Flip-Flops

| Flip-Flop Name | Bit-width | Functionality |
|----------------|-----------|---------------|
|Instr|	32	|32b instruction read from imem |
|Nop|	1	|If set, ID stage performs a nop|

### ID/EXE Stage Flip-Flops

| Flip-Flop Name | Bit-width | Functionality |
|----------------|-----------|---------------|
|Read\_data1, Read\_data2	|32	|32b data values read from RF|
|Rs, Rt|	5|	Addresses of source registers rs, rt. *Note: these are defined for both R-type and I-type instructions* |
|Wrt\_reg\_addr|	5	|Address of the instruction’s destination register. *Don’t care is the instruction doesn’t update RF* |
|Alu\_op	|1|	Set for addu, lw, sw; unset for subu|
|Is\_I\_type	|1|	Set if the instruction is I-type|
|Wrt\_enable	|1|	Set if instruction updates RF |
|Rd\_mem, Wr\_mem	|1|	Rd\_mem set for lw and wrt\_mem set for sw instructions. |
|Nop|	1|	If set, EXE stage performs a nop|

### EXE/MEM Stage Flip-Flops

| Flip-Flop Name | Bit-width | Functionality |
|----------------|-----------|---------------|
|ALU\_result	|32|	32b ALU result. *Don’t care for beq*
|Store\_data	|32|	32b value to be stored in DMEM for sw instruction. *Don’t care otherwise* |
|Rs, Rt	|5|	Addresses of source registers rs, rt. *Note these are defined for both R-type and I-type instructions* |
|Wrt\_reg\_addr	|5|	Address of the instruction’s destination register. *Don’t care is the instruction doesn’t update RF* |
|Wrt\_enable	|1|	Set if instruction updates RF|
|Rd\_mem, Wr\_mem	|1|	Rd_mem set for lw and wrt\_mem set for sw instructions. |
|Nop|	1|	If set, MEM stage performs a nop|

### WB Stage Input Flip-Flops
| Flip-Flop Name | Bit-width | Functionality |
|----------------|-----------|---------------|
|Wrt\_data|	32	|32b value to be written back to RF. *Don’t care for sw and beq.* |
|Rs, Rt	|5|	Addresses of source registers rs, rt. *Note these are defined for both R-type and I-type instructions* |
|Wrt\_reg\_addr	|5|	Address of the instruction’s destination register. *Don’t care is the instruction doesn’t update RF* |
|Wrt_enable	|1|	Set if instruction updates RF|
|Nop|	1|	If set, WB stage performs a nop|

## Dealing with Hazards

The processor must deal with two types of hazards. 

1.	**RAW Hazards:** RAW hazards are dealt with using either only forwarding (if possible) or, if not, using stalling + forwarding. You must follow the mechanisms described in Lecture to deal with RAW hazards. 

2.	**Control Flow Hazards:** You will assume that branch conditions are resolved in the ID/RF stage of the pipeline. Your processor deals with bne instructions as follows: 
	1. Branches are always assumed to be NOT TAKEN. That is, when a bne is fetched in the IF stage, the PC is speculatively updated as PC+4. 
	2. Branch conditions are resolved in the ID/RF stage. To make your life easier, we will ensure that every bne instruction has no RAW dependency with its previous two instructions. In other words, you do NOT have to deal with RAW hazards for branches! 
	3. Two operations are performed in the ID/RF stage: (i) Read_data1 and Read_data2 are compared to determine the branch outcome; (ii) the effective branch address is computed. 
	4. If the branch is NOT TAKEN, execution proceeds normally. However, if the branch is TAKEN, the speculatively fetched instruction from PC+4 is quashed in its ID/RF stage using the nop bit and the next instruction is fetched from the effective branch address. Execution now proceeds normally. 

## The NOP bit 

The nop bit for any stage indicates whether it is performing a valid operation in the current clock cycle. The nop bit for the IF stage is initialized to 0 and for all other stages is initialized to 1. (This is because in the first clock cycle, only the IF stage performs a valid operation) 

In the absence of hazards, the value of the nop bit for a stage in the current clock cycle is equal to the nop bit of the prior stage in the previous clock cycle. 

However, the nop bit is also used to implement stalls that result from a RAW hazard or to squash speculatively fetched instructions if the branch condition evaluates to TAKEN. See slides for more details on implementing stalls and squashing instructions. 


## The HALT Instruction

The halt instruction is a “custom” instruction we introduced so you know when to stop the simulation. When a HALT instruction is fetched in the IF stage at cycle N, the nop bit of the IF stage in the next clock cycle (cycle N+1) is set to 1 and subsequently stays at 1. The nop bit of the ID/RF stage is set to 1 in cycle N+1 and subsequently stays at 1. The nop bit of the EX stage is set to 1 in cycle N+2 and subsequently stays at 1. The nop bit of the MEM stage is set to 1 in cycle N+3 and subsequently stays at 1. The nop bit of the WB stage is set to 1 in cycle N+4 and subsequently stays at 1. 

At the end of each clock cycle the simulator checks to see if the nop bit of each stage is 1. If so, the simulation halts. 

## Output

The OutputRF() function is called at the end of each iteration of the while loop, and will add the new state of the Register File to “RFresult.txt”. Therefore, at the end of the program execution “RFresult.txt” contains all the intermediate states of the Register File. Once the program terminates, the OutputDataMem() function will write the final state of the Data Memory to “dmemresult.txt”. 