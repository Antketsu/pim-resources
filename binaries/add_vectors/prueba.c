#include <stdio.h>
#include <stdint.h>

int main() {
    uint32_t raw_instr = 0x32c00000;
    printf("Raw instruction: 0x%x\n", raw_instr);

    uint32_t mask = ~(uint32_t)0x0FE3FFFFF; // Literal de 32 bits
    printf("Mask for SRC0 operand: 0x%x\n", mask);

    uint32_t operand = (raw_instr & mask);
    printf("No shift: 0x%x\n", operand);

    operand = operand >> 22;
    printf("SRC0 operand: 0x%x\n", operand);

    return 0;
}
