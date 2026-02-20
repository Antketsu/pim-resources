#include <stdio.h>
#include <stdint.h>
#include "pim.h"
#include <stdlib.h>
int main(){
    init_pim();
    pim_operand A, B, C;
    int cols = 16 * 10 + 4;
    init_operand(&A, 1, cols);
    init_operand(&B, 1, cols);
    init_operand(&C, 1, cols);


    for (int i = 0; i < cols; i++) {
        A.vector[i] = i;
        B.vector[i] = 2 * i;
    }

    if(add(A, B, C)){
        printf("Add error\n");
        exit(1);
    }
    
    printf("Resultado de la suma vectorial:\n");
    for(int i = 0; i < cols; i++) {
        printf("C[%d] = %d\n", i, C.vector[i]);
    }

}