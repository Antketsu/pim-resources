#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>
#include <string.h>
#include <gem5/m5ops.h>

#define SIMD_WIDTH 16
#define CTL_INST(opcode, imm0, imm1) \
    (opcode << 28) | (imm0 << 11) | (imm1)  
#define DATA_INST(opcode, dst, src, relu, dst_idx, src_idx) \
    (opcode << 28) | (dst << 25) | (src << 22) | (relu << 12) | (dst_idx << 8) | (src_idx << 4) | (0)  
#define ALU_INST(opcode, dst, src0, src1, src2, dst_idx, src0_idx, src1_idx) \
    (opcode << 28) | (dst << 25) | (src0 << 22) | (src1 << 19) | (dst_idx << 8) | (src0_idx << 4) | (src1_idx)  

typedef struct{
    int16_t *vector;
    uint8_t rows;
    uint8_t cols;
} pim_operand;

int init_pim();
int init_operand(pim_operand *op, uint8_t rows, uint8_t cols);
int add(pim_operand a, pim_operand b, pim_operand c);