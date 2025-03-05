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

// addr bus 11 bit, data bus 8 bit, control bus 5 bit, io memory, read/write, ouput enable
uint8_t ADDR, BUS, CONTROL, IOM, RW, OE;

void IOMemory(void)
{
    if (IOM == 0)
    {
        if (RW == 0 && OE == 1) // memory read
            BUS = ioBuffer[ADDR];
        else if (RW == 1 && OE == 1) // memory write
            ioBuffer[ADDR] = BUS;
    }
}

void MainMemory(void)
{
    if (IOM == 1)
    {
        if (RW == 0 && OE == 1) // Read operation
        {
            printf("DEBUG: Reading Memory at ADDR=0x%03X, Value=0x%02X\n", ADDR, dataMemory[ADDR]);
            BUS = dataMemory[ADDR];
        }
        else if (RW == 1 && OE == 1) // Write operation
        {
            printf("DEBUG: Writing 0x%02X to Memory at ADDR=0x%03X\n", BUS, ADDR);
            dataMemory[ADDR] = BUS;
        }
    }
}

void printBinary(uint32_t value, int bits)
{
    for (int i = bits - 1; i >= 0; i--)
        printf("%d", (value >> i) & 1);
}

void displayInfo(uint16_t PC, uint16_t MAR, uint16_t IOAR, uint8_t IOBR, uint8_t CONTROL)
{
    printf("\n--- Current System State ---\n");

    printf("BUS     : 0x%02X  | Binary: ", BUS);
    printBinary(BUS, 8);
    printf("\n");

    printf("ADDR    : 0x%03X | Binary: ", ADDR);
    printBinary(ADDR, 11);
    printf("\n");

    printf("PC      : 0x%03X | Binary: ", PC);
    printBinary(PC, 11);
    printf("\n");

    printf("MAR     : 0x%03X | Binary: ", MAR);
    printBinary(MAR, 11);
    printf("\n");

    printf("IOAR    : 0x%03X | Binary: ", IOAR);
    printBinary(IOAR, 11);
    printf("\n");

    if (IOAR < IO_SIZE)
    {
        printf("IOR     : 0x%02X  | Binary: ", ioBuffer[IOAR]);
        printBinary(ioBuffer[IOAR], 8);
        printf("\n");
    }
    else
    {
        printf("IOR     : INVALID (IOAR out of range)\n");
    }

    printf("IOBR    : 0x%02X  | Binary: ", IOBR);
    printBinary(IOBR, 8);
    printf("\n");

    printf("CONTROL : 0x%02X  | Binary: ", CONTROL);
    printBinary(CONTROL, 5);
    printf("\n");
}

// Control Unit function
int CU(void)
{
    uint16_t PC = 0x000, IR, MAR, IOAR = 0x000, operand; // Program Counter, inst reg, memory addr reg, io addr reg, operand
    uint8_t MBR, IOBR, Fetch, IO, Memory, inst_code;     // mem buffer reg, io buffer reg, instruction code

    while (1)
    {
        printf("\n*****************************\n");
        printf("PC: 0x%03X\n", PC);
        printf("Fetching instruction...\n");

        printf("DEBUG: Fetching memory at PC=0x%03X: 0x%02X 0x%02X\n", PC, dataMemory[PC], dataMemory[PC + 1]);
        // Fetch instruction using big endian
        CONTROL = inst_code;
        Fetch = 1;
        IOM = 1; // Main Memory access
        RW = 0;  // Read operation
        OE = 1;  // Allow data movement
        IO = 0;
        Memory = 0;

        /* Fetching the upper byte */
        ADDR = PC;
        MainMemory(); // Fetch upper byte
        if (Fetch == 1)
        {
            IR = (uint16_t)BUS; // Load upper byte
            IR = IR << 8;       // Shift left
            PC++;               // Move to lower byte
        }

        /* Fetching the lower byte */
        ADDR = PC;
        MainMemory(); // Fetch lower byte
        if (Fetch == 1)
        {
            IR = IR | BUS; // Merge lower byte
            PC++;          // Move to next instruction
        }

        printf("IR: 0x%04X\n", IR);

        // Decode instruction
        inst_code = IR >> 11;  // get 5-bit instruction code
        operand = IR & 0x07FF; // get 11-bit operand

        printf("Instruction Code: 0x%02X\n", inst_code);
        printf("Operand: 0x%03X\n", operand);

        // Execute instruction
        switch (inst_code)
        {
        case 0x06: // WB - Write data to MBR
            printf("Instruction: WB\n");
            printf("Loading data to MBR...\n");
            MBR = operand & 0xFF;
            printf("MBR : 0x%02X\n", MBR);
            break;

        case 0x01: // WM - Write to Memory
            printf("Instruction: WM\n");
            printf("Writing data to memory...\n");

            MAR = operand; // load the operand to MAR (address)

            /* setting local control signals */
            Fetch = 0;
            Memory = 1; // accessing memory
            IO = 0;     // Not an I/O operation

            /* setting external control signals */
            CONTROL = inst_code; // setting the control signals
            IOM = 1;             // Main Memory access
            RW = 1;              // write operation
            OE = 1;              // allow data movement to/from memory

            ADDR = MAR; // load MAR to Address Bus
            if (Memory)
                BUS = MBR; // MBR owns the Bus since control signal Memory is 1

            MainMemory(); // Write data in Data Bus to memory
            displayInfo(PC, MAR, IOAR, IOBR, CONTROL);

            break;

        case 0x02: // RM - Read from Memory
            printf("Instruction: RM\n");
            printf("Reading data from memory...\n");

            MAR = operand; // Load operand into Memory Address Register

            Fetch = 0;
            Memory = 1; // Accessing main memory
            IO = 0;     // Not an I/O operation

            CONTROL = inst_code; // Set control signals
            IOM = 1;             // Main Memory access
            RW = 0;              // Read operation
            OE = 1;              // Allow data movement

            ADDR = MAR; // Load MAR to Address Bus

            MainMemory(); // Read data from memory
            MBR = BUS;    // Store the retrieved data in MBR

            printf("MBR : 0x%02X\n", MBR);
            displayInfo(PC, MAR, IOAR, IOBR, CONTROL);

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
            printf("IOBR : 0x%02X\n", IOBR);
            break;

        case 0x05: // WIO - Write to I/O Buffer
            printf("Instruction: WIO\n");
            printf("Writing to IO buffer...\n");

            IOAR = operand; // Load operand (I/O Address Register)

            Fetch = 0;
            Memory = 0; // Not a memory operation
            IO = 1;     // This is an I/O operation

            CONTROL = inst_code; // Set control signals
            IOM = 0;             // I/O Memory access
            RW = 1;              // Write operation
            OE = 1;              // Allow data movement

            ADDR = IOAR; // Load IOAR to Address Bus
            BUS = IOBR;  // Load data from IOBR to BUS

            IOMemory(); // Perform I/O buffer write operation
            displayInfo(PC, MAR, IOAR, IOBR, CONTROL);

            break;

        case 0x1F: // EOP - End of Program
            printf("Instruction: EOP\n");
            printf("End of program.\n");
            printf("\n*****************************\n");
            return 1;
        default:
            printf("Unknown instruction code 0x%02X\n", inst_code);
            printf("\n*****************************\n");
            return 0;
        }
    }
}

/* initialization by populating th data memory with the instructions
 * this are hard coded instructions
 * instructions are 16 bit so we do big endian to get first then shift left to get 2nd which is done in CU()
 */
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

    printf("Memory at 0x12A: 0x%02X 0x%02X\n", dataMemory[0x12A], dataMemory[0x12B]);
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
