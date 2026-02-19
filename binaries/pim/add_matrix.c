#include <stdio.h>
#include <stdint.h>
#include "pim.h"
#include <stdlib.h>
int main(){
    init_pim();
    pim_operand A, B, C;
    uint8_t rows = 4;
    uint8_t cols = 16 * 4 + 2;
    init_operand(&A, rows, cols);
    init_operand(&B, rows, cols);
    init_operand(&C, rows, cols);

    for(int i = 0; i < rows; ++i){
        for (int j = 0; j < cols; j++) {
            A.vector[i * cols + j] = j + i;
            B.vector[i * cols + j] = 2 * (i + 1) * j;
        }
    }

    if(add(A, B, C)){
        printf("Add error\n");
        exit(1);
    }
    
    printf("Resultado de la suma matricial:\n");
    for(int i = 0; i < rows; i++) {
        for(int j = 0; j < cols; ++j)
        printf("C[%d][%d] = %d\n", i, j, C.vector[i * cols + j]);
    }


}