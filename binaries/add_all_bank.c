#include <stdio.h>
#include <stdint.h>
#include "pim.h"
#include <stdlib.h>
int main(){
    init_pim();
    pim_operand_all_banks A, B, C;
    uint8_t rows = 64;
    uint8_t cols = 64;
    init_operand_all_banks(&A, rows, cols);
    init_operand_all_banks(&B, rows, cols);
    init_operand_all_banks(&C, rows, cols);

    uint32_t elements = rows * cols;
    uint32_t elems_per_bank = elements / 8;


    uint8_t col_idx = 0, row_idx = 0;
    for(int i = 0; i < 8; ++i){
        for (int j = 0; j < elems_per_bank; j++) {
            A.vectors[i][j] = col_idx + row_idx;
            B.vectors[i][j] = 2 * (row_idx + col_idx);
            ++col_idx;
            if(col_idx == cols){
                col_idx = 0;
                ++row_idx;
            }
        }
    }

    if(add_all_banks(A, B, C)){
        printf("Add error\n");
        exit(1);
    }
    
    printf("Resultado de la suma matricial:\n");
    col_idx = 0; row_idx = 0;
    for(int i = 0; i < 8; i++) {
        for(int j = 0; j < elems_per_bank; ++j){
            printf("C[%d][%d] = %d\n", row_idx, col_idx, C.vectors[i][j]);
            ++col_idx;
            if(col_idx == cols){
                col_idx = 0;
                ++row_idx;
            }
        }
    }


}