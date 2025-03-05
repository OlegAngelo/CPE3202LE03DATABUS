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

// Control Unit function
int CU(void)
{
    uint16_t PC = 0x000; // Program Counter
    uint16_t IR;         // Instruction Register
    uint16_t MAR;        // Memory Address Register
    uint8_t MBR;         // Memory Buffer Register
    uint16_t IOAR;       // I/O Address Register
    uint8_t IOBR;        // I/O Buffer Register

    while (1)
    {
        printf("\n*****************************\n");
        printf("PC: 0x%03X\n", PC);
        printf("Fetching instruction...\n");

        // Fetch instruction
        IR = dataMemory[PC] << 8; // get upper byte from memory pointed to by PC & move the byte to the correct position
        PC++;                     // point to the address of the lower byte
        IR |= dataMemory[PC];     // get lower byte from memory pointed to by PC
        PC++;                     // points to the next instruction

        printf("IR: 0x%04X\n", IR);

        // Decode instruction
        uint8_t inst_code = IR >> 11;   // get 5-bit instruction code
        uint16_t operand = IR & 0x07FF; // get 11-bit operand

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
            MAR = operand;
            dataMemory[MAR] = MBR;
            break;

        case 0x02: // RM - Read from Memory
            printf("Instruction: RM\n");
            printf("Reading data from memory...\n");
            MAR = operand;
            MBR = dataMemory[MAR];
            printf("MBR : 0x%02X\n", MBR);
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
            IOAR = operand;
            ioBuffer[IOAR] = IOBR;
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
