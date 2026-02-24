#include <stdio.h>
#include <stdint.h>
#include "pim.h"
#include <stdlib.h>
int main(){
    init_pim();
    pim_operand A, B, C;
    init_operand(&A, 1, 16);
    init_operand(&B, 16, 16);
    init_operand(&C, 1, 16);

    for(int i = 0; i < A.cols; ++i){
        A.vector[i] = 2 * i;
    }

    for(int i = 0; i < B.rows; ++i){
        for (int j = 0; j < B.cols; j++) {
            B.vector[i * B.cols + j] = i + j;
        }
    }

    if(matrix_multiplication(A, B, C)){
        printf("Matrix multiplication error\n");
        exit(1);
    }
    
    printf("Resultado de la multiplicación vector-matriz:\n");
    for(int j = 0; j < C.cols; ++j)
    printf("C[%d] = %d\n", j, C.vector[j]);

}