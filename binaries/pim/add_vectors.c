#include <stdio.h>
#include <stdint.h>
#include "pim.h"
#include <gem5/m5ops.h>
#include <stdlib.h>
int main(){
    init_pim();
    pim_operand A, B, C;
    int size = 32;
    init_operand(&A, size);
    init_operand(&B, size);
    init_operand(&C, size);
    m5_exit(0);

    for (int i = 0; i < size; i++) {
        A.vector[i] = i;
        B.vector[i] = 2 * i;
    }

    if(add(A, B, C)){
        printf("Add error\n");
        exit(1);
    }
    
    printf("Resultado de la suma vectorial:\n");
    for(int i = 0; i < size; i++) {
        printf("C[%d] = %d\n", i, C.vector[i]);
    }


}