#include <stdio.h>
#include <stdint.h>
#include <gem5/m5ops.h>

int main() {
    uint8_t rows = 64;
    uint8_t cols = 64;
    int16_t A[rows * cols];
    int16_t B[rows * cols];
    int16_t C[rows * cols];

    for(int i = 0; i < rows; ++i){
        for (int j = 0; j < cols; j++) {
            A[i * cols + j] = j + i;
            B[i * cols + j] = 2 * (i + j);
        }
    }

    for(int i = 0; i < rows; ++i){
        for (int j = 0; j < cols; j++) {
            C[i * cols + j] = A[i * cols + j] + B[i * cols + j];
        }
    }
    m5_work_begin(0, 0);
    for(int i = 0; i < rows; i++) {
        for(int j = 0; j < cols; ++j)
        printf("C[%d][%d] = %d\n", i, j, C[i * cols + j]);
    }
    m5_work_end(0, 0);
    return 0;
}