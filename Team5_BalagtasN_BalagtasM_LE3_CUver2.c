/*
 * Members:
 * Nino Angelo Balagtas
 * Mac Dylan Philippe Balagtas
 */

#include <stdio.h>
#include <stdint.h>

#define MEM_SIZE 2048 // 2^11 memory cells (11-bit addressable)
#define IO_SIZE 32    // 2^5 I/O buffer size (5-bit addressable)

// Memory & I/O Buffers
uint8_t dataMemory[MEM_SIZE];
uint8_t ioBuffer[IO_SIZE];

// Global Bus Variables
uint16_t ADDR;       // 11-bit Address Bus
uint8_t BUS;         // 8-bit Data Bus
uint8_t CONTROL;     // 5-bit Control Signals

// Global Control Signals
uint8_t IOM;         // I/O or Memory select (1 for Memory, 0 for I/O)
uint8_t RW;          // Read/Write signal (1 for Write, 0 for Read)
uint8_t OE;          // Output Enable signal

// Function prototypes
void MainMemory(void);
void IOMemory(void);

// Control Unit function
int CU(void)
{
    uint16_t PC = 0x000; // Program Counter
    uint16_t IR;         // Instruction Register
    uint16_t MAR;        // Memory Address Register
    uint8_t MBR;         // Memory Buffer Register
    uint16_t IOAR;       // I/O Address Register
    uint8_t IOBR;        // I/O Buffer Register

    // Local control signals
    uint8_t Fetch = 0;
    uint8_t Memory = 0;
    uint8_t IO = 0;
    uint8_t Increment = 0;

    while (1)
    {
        printf("\n*****************************\n");
        printf("PC: 0x%03X\n", PC);
        printf("Fetching instruction...\n");

        /* setting external control signals for fetch */
        CONTROL = 0x00;  // Clear control signals
        IOM = 1;         // Main Memory access
        RW = 0;          // read operation (fetch)
        OE = 1;          // allow data movement to/from memory

        /* Fetching Instruction (2 cycle) */
        Fetch = 1;       // set local control signal Fetch to 1
        IO = 0;
        Memory = 0;

        /* fetching the upper byte */
        ADDR = PC;
        MainMemory();    // fetch upper byte
        if(Fetch == 1)
        {
            IR = (int)BUS;   // load instruction to IR
            IR = IR << 8;    // shift IR 8 bits to the left
            PC++;           // points to the lower byte
            ADDR = PC;      // update address bus
        }

        /* fetching the lower byte */
        MainMemory();    // fetch lower byte
        if(Fetch == 1)
        {
            IR = IR | BUS;   // load the instruction on bus to lower 8 bits of IR
            PC++;           // points to the next instruction
        }

        printf("IR: 0x%04X\n", IR);

        // Decode instruction
        uint8_t inst_code = IR >> 11;   // get 5-bit instruction code
        uint16_t operand = IR & 0x07FF; // get 11-bit operand

        printf("Instruction Code: 0x%02X\n", inst_code);
        printf("Operand: 0x%03X\n", operand);

        // Execute instruction
        CONTROL = inst_code;  // Set control signals for current instruction
        Fetch = 0;           // Clear fetch signal

        switch (inst_code)
        {
        case 0x06: // WB - Write data to MBR
            printf("Instruction: WB\n");
            printf("Loading data to MBR...\n");
            MBR = operand & 0xFF;
            printf("MBR: 0x%02X\n", MBR);
            break;

        case 0x01: // WM - Write to Memory
            printf("Instruction: WM\n");
            printf("Writing data to memory...\n");
            MAR = operand;
            Memory = 1;
            IO = 0;
            IOM = 1;
            RW = 1;
            OE = 1;
            ADDR = MAR;
            if(Memory)
                BUS = MBR;
            MainMemory();
            printf("Data written to memory at address 0x%03X\n", MAR);
            break;

        case 0x02: // RM - Read from Memory
            printf("Instruction: RM\n");
            printf("Reading data from memory...\n");
            MAR = operand;
            Memory = 1;
            IO = 0;
            IOM = 1;
            RW = 0;
            OE = 1;
            ADDR = MAR;
            MainMemory();
            if(Memory)
                MBR = BUS;
            printf("MBR: 0x%02X\n", MBR);
            break;

        case 0x03: // BR - Branch
            printf("Instruction: BR\n");
            printf("Branch to 0x%03X on next cycle.\n", operand);
            PC = operand;
            break;

        case 0x07: // WIB - Write data to IOBR
            printf("Instruction: WIB\n");
            printf("Loading data to IOBR...\n");
            IOBR = operand & 0xFF;
            printf("IOBR: 0x%02X\n", IOBR);
            break;

        case 0x05: // WIO - Write to I/O Buffer
            printf("Instruction: WIO\n");
            printf("Writing to IO buffer...\n");
            IOAR = operand;
            Memory = 0;
            IO = 1;
            IOM = 0;
            RW = 1;
            OE = 1;
            ADDR = IOAR;
            if(IO)
                BUS = IOBR;
            IOMemory();
            printf("Data written to I/O buffer at address 0x%02X\n", IOAR);
            break;

        case 0x1F: // EOP - End of Program
            printf("Instruction: EOP\n");
            printf("End of program.\n");
            printf("\nFinal Bus States:\n");
            printf("BUS: 0x%02X\n", BUS);
            printf("ADDR: 0x%03X\n", ADDR);
            printf("CONTROL: 0x%02X\n", CONTROL);
            printf("IOM: %d, R/W: %d, OE: %d\n", IOM, RW, OE);
            printf("\n*****************************\n");
            return 1;

        default:
            printf("Unknown instruction code 0x%02X\n", inst_code);
            printf("\n*****************************\n");
            return 0;
        }

        // Display bus and control signal states after each instruction
        printf("\nBus States:\n");
        printf("BUS: 0x%02X\n", BUS);
        printf("ADDR: 0x%03X\n", ADDR);
        printf("CONTROL: 0x%02X\n", CONTROL);
        printf("IOM: %d, R/W: %d, OE: %d\n", IOM, RW, OE);
    }
}

void MainMemory(void)
{
    if(IOM == 1)
    {
        if(RW == 0 && OE == 1)      // memory read
            BUS = dataMemory[ADDR];
        else if(RW == 1 && OE == 1)  // memory write
            dataMemory[ADDR] = BUS;
    }
}

void IOMemory(void)
{
    if(IOM == 0)
    {
        if(RW == 0 && OE == 1)      // I/O read
            BUS = ioBuffer[ADDR];
        else if(RW == 1 && OE == 1)  // I/O write
            ioBuffer[ADDR] = BUS;
    }
}

// initialization by populating th data memory with the instructions
void initMemory(void)
{
    dataMemory[0x000] = 0x30; // WB 0xFF
    dataMemory[0x001] = 0xFF;
    dataMemory[0x002] = 0x0C; // WM 0x400
    dataMemory[0x003] = 0x00;
    dataMemory[0x004] = 0x14; // RM 0x400
    dataMemory[0x005] = 0x00;
    dataMemory[0x006] = 0x19; // BR 0x12A
    dataMemory[0x007] = 0x2A;
    dataMemory[0x12A] = 0x38; // WIB 0x05
    dataMemory[0x12B] = 0x05;
    dataMemory[0x12C] = 0x28; // WIO 0x0A
    dataMemory[0x12D] = 0x0A;
    dataMemory[0x12E] = 0xF8; // EOP
    dataMemory[0x12F] = 0x00;
}

int main(void)
{
    printf("Initializing Main Memory...\n");
    initMemory();
    if (CU() == 1)
    {
        printf("Program ran successfully!\n");
    }
    else
    {
        printf("Error encountered, program terminated!\n");
    }
    return 0;
}
