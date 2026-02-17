#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>

typedef struct{
    int16_t *vector;
    int8_t size;
} pim_operand;

int init_pim();
int init_operand(pim_operand *op, int8_t elems);
int add(pim_operand a, pim_operand b, pim_operand c);