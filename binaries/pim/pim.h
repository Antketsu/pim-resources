#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>
#include <string.h>
#include <gem5/m5ops.h>
typedef struct{
    int16_t *vector;
    uint8_t rows;
    uint8_t cols;
} pim_operand;

int init_pim();
int init_operand(pim_operand *op, uint8_t rows, uint8_t cols);
int add(pim_operand a, pim_operand b, pim_operand c);