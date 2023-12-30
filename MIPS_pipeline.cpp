#include <bitset>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

#define MemSize 1000 
// memory size, in reality, the memory size should be 2^32, but for this lab csa23, for the space resaon, we
// keep it as this large number, but the memory is still 32-bit addressable.

struct IFStruct
{
    bitset<32> PC;
    bool nop;
};

struct IDStruct
{
    bitset<32> Instr;
    bool nop;
};

struct EXStruct
{
    bitset<32> Read_data1;
    bitset<32> Read_data2;
    bitset<16> Imm;
    bitset<5> Rs;
    bitset<5> Rt;
    bitset<5> Wrt_reg_addr;
    bool is_I_type;
    bool rd_mem;
    bool wrt_mem;
    bool alu_op; // 1 for addu, lw, sw, 0 for subu
    bool wrt_enable;
    bool nop;
    bool is_branch;
};

struct MEMStruct
{
    bitset<32> ALUresult;
    bitset<32> Store_data;
    bitset<5> Rs;
    bitset<5> Rt;
    bitset<5> Wrt_reg_addr;
    bool rd_mem;
    bool wrt_mem;
    bool wrt_enable;
    bool nop;
    bool is_branch;
};

struct WBStruct
{
    bitset<32> Wrt_data;
    bitset<5> Rs;
    bitset<5> Rt;
    bitset<5> Wrt_reg_addr;
    bool wrt_enable;
    bool nop;
};

struct stateStruct
{
    IFStruct IF;
    IDStruct ID;
    EXStruct EX;
    MEMStruct MEM;
    WBStruct WB;
};

class RF
{
  public:
    bitset<32> Reg_data;

    RF()
    {
        Registers.resize(32);
        Registers[0] = bitset<32>(0);
    }

    bitset<32> readRF(bitset<5> Reg_addr)
    {
        Reg_data = Registers[Reg_addr.to_ulong()];
        return Reg_data;
    }

    void writeRF(bitset<5> Reg_addr, bitset<32> Wrt_reg_data)
    {
        Registers[Reg_addr.to_ulong()] = Wrt_reg_data;
    }

    void outputRF()
    {
        ofstream rfout;
        rfout.open("RFresult.txt", std::ios_base::app);
        if (rfout.is_open())
        {
            rfout << "State of RF:\t" << endl;
            for (int j = 0; j < 32; j++)
            {
                rfout << Registers[j] << endl;
            }
        }
        else
            cout << "Unable to open file";
        rfout.close();
    }

  private:
    vector<bitset<32> > Registers;
};

class INSMem
{
  public:
    bitset<32> Instruction;
    
    INSMem()
    {
        IMem.resize(MemSize);
        ifstream imem;
        string line;
        int i = 0;
        imem.open("imem.txt");
        if (imem.is_open())
        {
            while (getline(imem, line))
            {
                IMem[i] = bitset<8>(line);
                i++;
            }
        }
        else
            cout << "Unable to open file";
        imem.close();
    }

    bitset<32> readInstr(bitset<32> ReadAddress)
    {
        string insmem;
        insmem.append(IMem[ReadAddress.to_ulong()].to_string());
        insmem.append(IMem[ReadAddress.to_ulong() + 1].to_string());
        insmem.append(IMem[ReadAddress.to_ulong() + 2].to_string());
        insmem.append(IMem[ReadAddress.to_ulong() + 3].to_string());
        Instruction = bitset<32>(insmem); // read instruction memory
        return Instruction;
    }

  private:
    vector<bitset<8> > IMem;
};

class DataMem
{
  public:
    bitset<32> ReadData;
    
    DataMem()
    {
        DMem.resize(MemSize);
        ifstream dmem;
        string line;
        int i = 0;
        dmem.open("dmem.txt");
        if (dmem.is_open())
        {
            while (getline(dmem, line))
            {
                DMem[i] = bitset<8>(line);
                i++;
            }
        }
        else
            cout << "Unable to open file";
        dmem.close();
    }

    bitset<32> readDataMem(bitset<32> Address)
    {
        string datamem;
        datamem.append(DMem[Address.to_ulong()].to_string());
        datamem.append(DMem[Address.to_ulong() + 1].to_string());
        datamem.append(DMem[Address.to_ulong() + 2].to_string());
        datamem.append(DMem[Address.to_ulong() + 3].to_string());
        ReadData = bitset<32>(datamem); // read data memory
        return ReadData;
    }

    void writeDataMem(bitset<32> Address, bitset<32> WriteData)
    {
        DMem[Address.to_ulong()] = bitset<8>(WriteData.to_string().substr(0, 8));
        DMem[Address.to_ulong() + 1] = bitset<8>(WriteData.to_string().substr(8, 8));
        DMem[Address.to_ulong() + 2] = bitset<8>(WriteData.to_string().substr(16, 8));
        DMem[Address.to_ulong() + 3] = bitset<8>(WriteData.to_string().substr(24, 8));
    }

    void outputDataMem()
    {
        ofstream dmemout;
        dmemout.open("dmemresult.txt");
        if (dmemout.is_open())
        {
            for (int j = 0; j < 1000; j++)
            {
                dmemout << DMem[j] << endl;
            }
        }
        else
            cout << "Unable to open file";
        dmemout.close();
    }

  private:
    vector<bitset<8> > DMem;
};

void printState(stateStruct state, int cycle)
{
    ofstream printstate;
    printstate.open("stateresult.txt", std::ios_base::app);
    if (printstate.is_open())
    {
        printstate << "State after executing cycle:\t" << cycle << endl;

        printstate << "IF.PC:\t" << state.IF.PC.to_ulong() << endl;
        printstate << "IF.nop:\t" << state.IF.nop << endl;

        printstate << "ID.Instr:\t" << state.ID.Instr << endl;
        printstate << "ID.nop:\t" << state.ID.nop << endl;

        printstate << "EX.Read_data1:\t" << state.EX.Read_data1 << endl;
        printstate << "EX.Read_data2:\t" << state.EX.Read_data2 << endl;
        printstate << "EX.Imm:\t" << state.EX.Imm << endl;
        printstate << "EX.Rs:\t" << state.EX.Rs << endl;
        printstate << "EX.Rt:\t" << state.EX.Rt << endl;
        printstate << "EX.Wrt_reg_addr:\t" << state.EX.Wrt_reg_addr << endl;
        printstate << "EX.is_I_type:\t" << state.EX.is_I_type << endl;
        printstate << "EX.rd_mem:\t" << state.EX.rd_mem << endl;
        printstate << "EX.wrt_mem:\t" << state.EX.wrt_mem << endl;
        printstate << "EX.alu_op:\t" << state.EX.alu_op << endl;
        printstate << "EX.wrt_enable:\t" << state.EX.wrt_enable << endl;
        printstate << "EX.nop:\t" << state.EX.nop << endl;

        printstate << "MEM.ALUresult:\t" << state.MEM.ALUresult << endl;
        printstate << "MEM.Store_data:\t" << state.MEM.Store_data << endl;
        printstate << "MEM.Rs:\t" << state.MEM.Rs << endl;
        printstate << "MEM.Rt:\t" << state.MEM.Rt << endl;
        printstate << "MEM.Wrt_reg_addr:\t" << state.MEM.Wrt_reg_addr << endl;
        printstate << "MEM.rd_mem:\t" << state.MEM.rd_mem << endl;
        printstate << "MEM.wrt_mem:\t" << state.MEM.wrt_mem << endl;
        printstate << "MEM.wrt_enable:\t" << state.MEM.wrt_enable << endl;
        printstate << "MEM.nop:\t" << state.MEM.nop << endl;

        printstate << "WB.Wrt_data:\t" << state.WB.Wrt_data << endl;
        printstate << "WB.Rs:\t" << state.WB.Rs << endl;
        printstate << "WB.Rt:\t" << state.WB.Rt << endl;
        printstate << "WB.Wrt_reg_addr:\t" << state.WB.Wrt_reg_addr << endl;
        printstate << "WB.wrt_enable:\t" << state.WB.wrt_enable << endl;
        printstate << "WB.nop:\t" << state.WB.nop << endl;
    }
    else
        cout << "Unable to open file";
    printstate.close();
}

void print_state(stateStruct state, int cycle)
{
    cout << "IF.PC            : " << state.IF.PC.to_ulong() << endl;
    cout << "IF.nop           : " << state.IF.nop << endl;

    cout << "ID.Instr         : " << state.ID.Instr << endl;
    cout << "ID.nop           : " << state.ID.nop << endl;

    cout << "EX.Read_data1    : " << state.EX.Read_data1 << endl;
    cout << "EX.Read_data2    : " << state.EX.Read_data2 << endl;
    cout << "EX.Imm           : " << state.EX.Imm << endl;
    cout << "EX.Rs            : " << state.EX.Rs << endl;
    cout << "EX.Rt            : " << state.EX.Rt << endl;
    cout << "EX.Wrt_reg_addr  : " << state.EX.Wrt_reg_addr << endl;
    cout << "EX.is_I_type     : " << state.EX.is_I_type << endl;
    cout << "EX.rd_mem        : " << state.EX.rd_mem << endl;
    cout << "EX.wrt_mem       : " << state.EX.wrt_mem << endl;
    cout << "EX.alu_op        : " << state.EX.alu_op << endl;
    cout << "EX.wrt_enable    : " << state.EX.wrt_enable << endl;
    cout << "EX.nop           : " << state.EX.nop << endl;

    cout << "MEM.ALUresult    : " << state.MEM.ALUresult << endl;
    cout << "MEM.Store_data   : " << state.MEM.Store_data << endl;
    cout << "MEM.Rs           : " << state.MEM.Rs << endl;
    cout << "MEM.Rt           : " << state.MEM.Rt << endl;
    cout << "MEM.Wrt_reg_addr : " << state.MEM.Wrt_reg_addr << endl;
    cout << "MEM.rd_mem       : " << state.MEM.rd_mem << endl;
    cout << "MEM.wrt_mem      : " << state.MEM.wrt_mem << endl;
    cout << "MEM.wrt_enable   : " << state.MEM.wrt_enable << endl;
    cout << "MEM.nop          : " << state.MEM.nop << endl;

    cout << "WB.Wrt_data      : " << state.WB.Wrt_data << endl;
    cout << "WB.Rs            : " << state.WB.Rs << endl;
    cout << "WB.Rt            : " << state.WB.Rt << endl;
    cout << "WB.Wrt_reg_addr  : " << state.WB.Wrt_reg_addr << endl;
    cout << "WB.wrt_enable    : " << state.WB.wrt_enable << endl;
    cout << "WB.nop           : " << state.WB.nop << endl;
}

void initialise_state(stateStruct &state)
{
    state.IF.PC = bitset<32>(0);
    state.IF.nop = 0;

    state.ID.Instr = bitset<32>(0);
    state.ID.nop = 1;

    state.EX.is_I_type = 0;
    state.EX.rd_mem = 0;
    state.EX.wrt_mem = 0;
    state.EX.alu_op = 1;
    state.EX.wrt_enable = 0;
    state.EX.nop = 1;
    state.EX.is_branch = 0;

    state.MEM.rd_mem = 0;
    state.MEM.wrt_mem = 0;
    state.MEM.wrt_enable = 0;
    state.MEM.nop = 1;
    state.MEM.is_branch = 0;

    state.WB.wrt_enable = 0;
    state.WB.nop = 1;
}

bitset<32> sign_extend(bitset<16> number)
{
    bitset<32> signExtendedNumber(0);
    bool isNegative = number[15];
    if (isNegative)
    {
        signExtendedNumber = bitset<32>(string(16, '1') + number.to_string());
    }
    else
    {
        signExtendedNumber = bitset<32>(string(16, '0') + number.to_string());
    }
    return signExtendedNumber;
}

bitset<30> sign_extend_30(bitset<16> number)
{
    bitset<30> signExtendedNumber(0);
    bool isNegative = number[15];
    if (isNegative)
    {
        signExtendedNumber = bitset<30>(string(14, '1') + number.to_string());
    }
    else
    {
        signExtendedNumber = bitset<30>(string(14, '0') + number.to_string());
    }
    return signExtendedNumber;
}

unsigned long right_shift(bitset<32> inst, int shift_amt)
{
    return ((inst.to_ulong()) >> shift_amt);
}

int main()
{
    RF myRF;
    INSMem myInsMem;
    DataMem myDataMem;

    // Declaring state and newState structs
    stateStruct state;
    stateStruct newState;

    // Initialising state struct
    initialise_state(state);
    initialise_state(newState);

    int cycle = 0;

    while (1)
    {
        cout << "Cycle (" << dec << cycle << ") :\n";

        // if (cycle == 50)
        // {
        //     cout << "Stopping Execution\n";
        //     break;
        // }

        /* --------------------- WB stage --------------------- */
        if (state.WB.nop == 0)
        {
            cout << "| - WRITE BACK ----------------------- |\n";
            
            // In Write Back stage, if write enable is set, update the register
            if (state.WB.wrt_enable == 1)
            {
                // cout << "Writing to Register...";
                myRF.writeRF(state.WB.Wrt_reg_addr, state.WB.Wrt_data);
            }

            cout << "\n";
        }

        /* --------------------- MEM stage --------------------- */
        if (state.MEM.nop == 0)
        {
            cout << "| - MEMORY --------------------------- |\n";
            
            // Logic to check if we are writing to data memory
            if (state.WB.Wrt_reg_addr == state.WB.nop && state.MEM.Rt == 0)
            {
                state.MEM.Store_data = state.WB.Wrt_data;
            }

            // Logic to check if we are reading from data memory
            if (state.MEM.rd_mem == 1)
            {
                newState.WB.Wrt_data = myDataMem.readDataMem(state.MEM.ALUresult);
            }
            else
            {
                newState.WB.Wrt_data = state.MEM.ALUresult;
            }

            // Logic to write to data memory
            if (state.MEM.wrt_mem == 1)
            {
                myDataMem.writeDataMem(state.MEM.ALUresult, state.MEM.Store_data);
                newState.WB.Wrt_data = state.WB.Wrt_data;
            }
            
            if (state.MEM.is_branch)
            {
                newState.WB.Wrt_data = state.WB.Wrt_data;
            }

            newState.WB.Wrt_reg_addr = state.MEM.Wrt_reg_addr;
            newState.WB.wrt_enable = state.MEM.wrt_enable;
            newState.WB.Rs = state.MEM.Rs;
            newState.WB.Rt = state.MEM.Rt;
            newState.WB.nop = state.MEM.nop;
            
            cout << "\n";
        }
        else
        {
            newState.WB.nop = state.MEM.nop;
        }

        /* --------------------- EX stage --------------------- */
        if (state.EX.nop == 0)
        {
            cout << "| - EXECUTION ------------------------ |\n";
            
            if (state.EX.wrt_enable == 1 && state.EX.is_I_type == 0 && state.EX.alu_op == 1)
            {
                if (state.WB.Wrt_reg_addr == state.EX.Rs && state.WB.nop == 0 && state.WB.wrt_enable == 1)
                {
                    state.EX.Read_data1 = state.WB.Wrt_data;
                }

                if (state.WB.Wrt_reg_addr == state.EX.Rt && state.WB.nop == 0 && state.WB.wrt_enable == 1)
                {
                    state.EX.Read_data2 = state.WB.Wrt_data;
                }

                if (state.MEM.Wrt_reg_addr == state.EX.Rs && state.MEM.nop == 0 && state.MEM.wrt_enable == 1)
                {
                    state.EX.Read_data1 = state.MEM.ALUresult;
                }

                if (state.MEM.Wrt_reg_addr == state.EX.Rt && state.MEM.nop == 0 && state.MEM.wrt_enable == 1)
                {
                    state.EX.Read_data2 = state.MEM.ALUresult;
                }

                newState.MEM.ALUresult = bitset<32>(state.EX.Read_data1.to_ulong() + state.EX.Read_data2.to_ulong());
            }
            else if (state.EX.wrt_enable == 1 && state.EX.is_I_type == 0 && state.EX.alu_op == 0)
            {
                if (state.WB.Wrt_reg_addr == state.EX.Rs && state.WB.nop == 0 && state.WB.wrt_enable == 1) // m-e
                {
                    state.EX.Read_data1 = state.WB.Wrt_data;
                }
                if (state.WB.Wrt_reg_addr == state.EX.Rt && state.WB.nop == 0 && state.WB.wrt_enable == 1) // m-e
                {
                    state.EX.Read_data2 = state.WB.Wrt_data;
                }
                if (state.MEM.Wrt_reg_addr == state.EX.Rs && state.MEM.nop == 0 && state.MEM.wrt_enable == 1) // m-e
                {
                    state.EX.Read_data1 = state.MEM.ALUresult;
                }
                if (state.MEM.Wrt_reg_addr == state.EX.Rt && state.MEM.nop == 0 && state.MEM.wrt_enable == 1) // e-e
                {
                    state.EX.Read_data2 = state.MEM.ALUresult;
                }

                newState.MEM.ALUresult = bitset<32>(state.EX.Read_data1.to_ulong() - state.EX.Read_data2.to_ulong());
            }
            else if (state.EX.wrt_enable == 1 && state.EX.is_I_type == 1 && state.EX.alu_op == 1)
            {
                if (state.WB.Wrt_reg_addr == state.EX.Rt && state.WB.nop == 0 && state.WB.wrt_enable == 1) // m-e
                {
                    state.EX.Read_data2 = state.WB.Wrt_data;
                }
                if (state.WB.Wrt_reg_addr == state.EX.Rs && state.WB.nop == 0 && state.WB.wrt_enable == 1)
                {
                    state.EX.Read_data1 = state.WB.Wrt_data;
                }
                if (state.MEM.Wrt_reg_addr == state.EX.Rs && state.MEM.nop == 0 && state.MEM.wrt_enable == 1) // e-e
                {
                    state.EX.Read_data1 = state.MEM.ALUresult;
                }
                if (state.MEM.Wrt_reg_addr == state.EX.Rt && state.MEM.nop == 0 && state.MEM.wrt_enable == 1) // e-e
                {
                    state.EX.Read_data2 = state.MEM.ALUresult;
                }

                newState.MEM.ALUresult = bitset<32>(state.EX.Read_data1.to_ulong() + sign_extend(state.EX.Imm).to_ulong());
            }
            else if (state.EX.wrt_enable == 0 && state.EX.is_I_type == 1 && state.EX.alu_op == 1)
            {
                if (state.WB.Wrt_reg_addr == state.EX.Rt && state.WB.nop == 0 && state.WB.wrt_enable == 1)
                {
                    state.EX.Read_data2 = state.WB.Wrt_data;
                }

                if (state.WB.Wrt_reg_addr == state.EX.Rs && state.WB.nop == 0 && state.WB.wrt_enable == 1)
                {
                    state.EX.Read_data1 = state.WB.Wrt_data;
                }
                if (state.MEM.Wrt_reg_addr == state.EX.Rs && state.MEM.nop == 0 && state.MEM.wrt_enable == 1) // e-e
                {
                    state.EX.Read_data1 = state.MEM.ALUresult;
                }
                if (state.MEM.Wrt_reg_addr == state.EX.Rt && state.MEM.nop == 0 && state.MEM.wrt_enable == 1) // e-e
                {
                    state.EX.Read_data2 = state.MEM.ALUresult;
                }

                newState.MEM.ALUresult = bitset<32>(state.EX.Read_data1.to_ulong() + sign_extend(state.EX.Imm).to_ulong());
            }
            else
            {
                newState.MEM.ALUresult = bitset<32>(string(32, '0'));
            }

            newState.MEM.Rs = state.EX.Rs;
            newState.MEM.Rt = state.EX.Rt;
            newState.MEM.Wrt_reg_addr = state.EX.Wrt_reg_addr;
            newState.MEM.rd_mem = state.EX.rd_mem;
            newState.MEM.wrt_mem = state.EX.wrt_mem;
            newState.MEM.wrt_enable = state.EX.wrt_enable;
            newState.MEM.nop = state.EX.nop;
            newState.MEM.Store_data = state.EX.Read_data2;
            newState.MEM.is_branch = state.EX.is_branch;

            cout << "\n";
        }
        else
        {
            newState.MEM.nop = state.EX.nop;
        }

        /* --------------------- ID stage --------------------- */
        if (state.ID.nop == 0)
        {
            cout << "| - INSTRUCTION DECODE --------------- |\n";

            bitset<6> OpCode = bitset<6>(right_shift(state.ID.Instr, 26));
            cout << "OPCODE         : " << OpCode.to_string() << " (" << hex << OpCode.to_ulong() << ")\n";
            
            bitset<1> iType =
                bitset<1>((OpCode.to_ulong() != 0 && OpCode.to_ulong() != 2 && OpCode.to_ulong() != 5) ? 1 : 0);
            cout << "I-type         : " << iType.to_string() << "\n";
            newState.EX.is_I_type = iType.to_ulong();
            
            bitset<1> jType = bitset<1>((OpCode.to_ulong() == 2) ? 1 : 0);
            cout << "J-type         : " << jType.to_string() << "\n";
            
            bitset<1> bType = bitset<1>((OpCode.to_ulong() == 5) ? 1 : 0);
            cout << "B-type         : " << bType.to_string() << "\n";
            newState.EX.is_branch = bType.to_ulong();
            
            bitset<1> loadType = bitset<1>((OpCode.to_ulong() == 35) ? 1 : 0);
            cout << "Load Type      : " << loadType.to_string() << "\n";

            bitset<1> storeType = bitset<1>((OpCode.to_ulong() == 43) ? 1 : 0);
            cout << "Store Type     : " << storeType.to_string() << "\n";

            newState.EX.rd_mem = loadType.to_ulong();
            // newState.EX.wrt_mem = !((bool) loadType.to_ulong());
            
            newState.EX.wrt_mem = storeType.to_ulong();
            // newState.EX.rd_mem = !((bool) storeType.to_ulong());

            newState.EX.wrt_enable = (storeType.to_ulong() || bType.to_ulong() || jType.to_ulong()) ? 0 : 1;
            
            bitset<5> RReg1 = bitset<5>(right_shift(state.ID.Instr, 21));
            bitset<5> RReg2 = bitset<5>(right_shift(state.ID.Instr, 16));
            cout << "Rs Add.        : " << RReg1.to_string() << "\n";
            cout << "Rt Add.        : " << RReg2.to_string() << "\n";
            newState.EX.Rs = RReg1;
            newState.EX.Rt = RReg2;
            
            bitset<6> functCode = bitset<6>(right_shift(state.ID.Instr, 0));
            cout << "Function Code  : " << functCode.to_string() << " (" << hex << functCode.to_ulong() << ")\n";
            
            bitset<1> ALUop;
            if (OpCode.to_ulong() == 35 || OpCode.to_ulong() == 43 || functCode.to_ulong() == 33 || OpCode.to_ulong() == 5)
            {
                ALUop = bitset<1>(string("1"));
            }
            else
            {
                ALUop = bitset<1>(string("0"));
            }
            cout << "ALU OP Code    : " << ALUop.to_string() << "\n";
            newState.EX.alu_op = ALUop.to_ulong();

            bitset<16> imm = bitset<16>(right_shift(state.ID.Instr, 0));
            cout << "Immediate      : " << imm.to_string() << "\n";
            newState.EX.Imm = imm;
            
            if (iType.to_ulong())
            {
                // If Instruction is I-type, set the Wrt Register Address the value in Rt...
                newState.EX.Wrt_reg_addr = RReg2;
                cout << "Write Reg Add. : " << RReg2.to_string() << "\n";
            }
            else
            {
                // If Instrucion is not I-type, set the Wrt Register Address the value in Instruction (Rd)...
                newState.EX.Wrt_reg_addr = bitset<5>(right_shift(state.ID.Instr, 11));
                cout << "Rd Add.        : " << newState.EX.Wrt_reg_addr.to_string() << "\n";
            }
            
            // Read the data from the registers...
            newState.EX.Read_data1 = myRF.readRF(RReg1);
            newState.EX.Read_data2 = myRF.readRF(RReg2);
            newState.EX.nop = state.ID.nop;
            
            // Check if the registers are equal...
            bool isEqual = ((myRF.readRF(RReg1).to_ulong() == myRF.readRF(RReg2).to_ulong()) ? 1 : 0);

            // Hazard Handling...
            if (state.EX.is_I_type == 1 && state.EX.rd_mem == 1)
            {
                if (state.EX.Rt == bitset<5>(right_shift(state.ID.Instr, 21)) && state.EX.nop == 0 &&
                    bitset<6>(right_shift(state.ID.Instr, 26)) == bitset<6>("000000"))
                {
                    newState.EX.nop = 1;
                }
                if (state.EX.Rt == bitset<5>(right_shift(state.ID.Instr, 16)) && state.EX.nop == 0 &&
                    bitset<6>(right_shift(state.ID.Instr, 26)) == bitset<6>("000000"))
                {
                    newState.EX.nop = 1;
                }
            }

            // Store Word Hazard Handling...
            if (state.EX.is_I_type == 1 && state.EX.rd_mem == 1 &&
                (((bitset<6>(right_shift(state.ID.Instr, 26))).to_ulong() == 43) ? 1 : 0))
            {
                if (state.EX.Rt == bitset<5>(right_shift(state.ID.Instr, 21)))
                {
                    newState.EX.nop = 1;
                }
            }

            // If Instruction is branch type instruction, the write registers address is set to 0s...
            if (bType == bitset<1>("1"))
            {
                newState.EX.Wrt_reg_addr = bitset<5>("00000");
            }

            if (bType == bitset<1>("1") && isEqual == 0)
            {
                cout << "Branch Not Equal... setting PC to PC - 4 in Instruction Decode stage... \n";
                state.IF.PC = bitset<32>(state.IF.PC.to_ulong() - 4);
            }

            cout << "\n";
        }
        else
        {
            newState.EX.nop = state.ID.nop;
        }

        /* --------------------- IF stage --------------------- */
        if (state.IF.nop == 0)
        {
            cout << "| - INSTRUCTION FETCH ---------------- |\n";

            bitset<32> instruction = myInsMem.readInstr(state.IF.PC);
            cout << "Instruction    : " << instruction << "\n";

            newState.ID.Instr = instruction;

            if (instruction.to_string() == string(32, '1'))
            {
                cout << "Halt Instruction!\n";
                state.IF.nop = 1;
            }

            newState.ID.nop = state.IF.nop;

            // Hazards Handling
            if (state.EX.is_I_type == 1 && state.EX.rd_mem == 1)
            {
                if (state.EX.Rt == bitset<5>(right_shift(state.ID.Instr, 21)) && state.EX.nop == 0 &&
                    bitset<6>(right_shift(state.ID.Instr, 26)) == bitset<6>("000000"))
                {
                    newState.ID = state.ID;
                }
                if (state.EX.Rt == bitset<5>(right_shift(state.ID.Instr, 16)) && state.EX.nop == 0 &&
                    bitset<6>(right_shift(state.ID.Instr, 26)) == bitset<6>("000000"))
                {
                    newState.ID = state.ID;
                }
            }
            
            // Store Type Hazard Handling....
            if (state.EX.is_I_type == 1 && state.EX.rd_mem == 1 && ((bitset<6>(right_shift(state.ID.Instr, 26))).to_ulong() == 43 ? 1 : 0))
            {
                if (state.EX.Rt == bitset<5>(right_shift(state.ID.Instr, 21)))
                {
                    newState.ID = state.ID;
                }
            }

            // Branch Hazard Handling...
            if (((bitset<6>(right_shift(state.ID.Instr, 26)).to_ulong() == 5) ? 1 : 0) &&
                (myRF.readRF(bitset<5>(right_shift(state.ID.Instr, 21))).to_ulong() !=
                           myRF.readRF(bitset<5>(right_shift(state.ID.Instr, 16))).to_ulong())
                              ? 1
                              : 0)
            {
                cout << "BNE taken! \n";
                if (state.ID.nop == 0)
                {
                    newState.ID.nop = 1;
                }
            }
            
            cout << "\n";
        }
        else
        {
            newState.ID.nop = state.IF.nop;
        }

        // Halt Instruction Check...
        if (state.IF.nop && state.ID.nop && state.EX.nop && state.MEM.nop && state.WB.nop)
        {
            cout << "| =============== HALT =============== |\n";
            break;
        }

        /* -------------------- PC updation ------------------- */
        if (myInsMem.readInstr(state.IF.PC).to_string() != string(32, '1')) // If Instruction is not a halt instruction...
        {
            // Check if it is a branch instruction and whether the values in the registers are unequal...
            if (((bitset<6>(right_shift(state.ID.Instr, 26)).to_ulong() == 5) ? 1 : 0) &&
                (myRF.readRF(bitset<5>(right_shift(state.ID.Instr, 21))).to_ulong() !=
                           myRF.readRF(bitset<5>(right_shift(state.ID.Instr, 16))).to_ulong()) ? 1 : 0)
            {
                // If the registers are unequal... we are taking the branch if the ID stage nop is 0...
                if (state.ID.nop == 0)
                {
                    cout << "Branch taken... updating PC...\n";
                    newState.IF.PC = bitset<32>(
                        state.IF.PC.to_ulong() + 4 +
                        bitset<32>(sign_extend_30(bitset<16>(right_shift(state.ID.Instr, 0))).to_string() + "00").to_ulong());
                }
                else
                {
                    // Registers are unequal, but we have ID stage nop = 1... so we update PC with PC + 4
                    newState.IF.PC = state.IF.PC.to_ulong() + 4;
                }
            }
            else
            {
                // If it is not a branch instruction, we update the PC with PC = PC + 4
                newState.IF.PC = state.IF.PC.to_ulong() + 4;
            }
        }

        newState.IF.nop = state.IF.nop;

        // Hazard Handling...
        if (state.EX.is_I_type == 1 && state.EX.rd_mem == 1)
        {
            if (state.EX.Rt == bitset<5>(right_shift(state.ID.Instr, 21)) && state.EX.nop == 0 &&
                bitset<6>(right_shift(state.ID.Instr, 26)) == bitset<6>("000000"))
            {
                newState.IF = state.IF;
            }
            if (state.EX.Rt == bitset<5>(right_shift(state.ID.Instr, 16)) && state.EX.nop == 0 &&
                bitset<6>(right_shift(state.ID.Instr, 26)) == bitset<6>("000000"))
            {
                newState.IF = state.IF;
            }
        }
        
        // Store Word Hazard Handling...
        if (state.EX.is_I_type == 1 && state.EX.rd_mem == 1 &&
            ((bitset<6>(right_shift(state.ID.Instr, 26))).to_ulong() == 43) ? 1 : 0)
        {
            if (state.EX.Rt == bitset<5>(right_shift(state.ID.Instr, 21)))
            {
                newState.IF = state.IF;
            }
        }

        // Halt Instruction Check...
        if (state.IF.nop && state.ID.nop && state.EX.nop && state.MEM.nop && state.WB.nop)
        {
            cout << "| =============== HALT =============== |\n";
            break;
        }

        // cout << "State: \n";
        // print_state(state, cycle);
        // cout << "\n";
        // cout << "New State: \n";
        // print_state(newState, cycle);

        printState(newState, cycle); // print states after executing cycle 0, cycle 1, cycle 2 ...
        cycle = cycle + 1;
        state = newState; /*** The end of the cycle and updates the current state with the values calculated in this
                             cycle. csa23 ***/
        cout << "\n";
    }

    myRF.outputRF();           // dump RF;
    myDataMem.outputDataMem(); // dump data mem

    return 0;
}