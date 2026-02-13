#include <stdio.h>
#include <stdint.h>
#include <sys/mman.h>
#include <gem5/m5ops.h>
#define PIM 0x10000000
#define VECTOR_A 0x20000000
#define VECTOR_B 0x30000000
#define VECTOR_C 0x40000000

int main(){
    // Tamaño de la región de memoria
    size_t size = 0x1000000;  // 16 MB

    int8_t *pim_region = mmap(
        (void *)0x10000000,  
        size,
        PROT_READ | PROT_WRITE,
        MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED,
        -1,
        0
    );

    if (pim_region == MAP_FAILED) {
        perror("Error al mapear la región PIM");
        return 1;
    }
    printf("Región PIM mapeada correctamente\n");

    int16_t *A = mmap(
        (void *)0x20000000,  
        size,
        PROT_READ | PROT_WRITE,
        MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED,
        -1,
        0
    );
    int16_t *B = mmap(
        (void *)0x30000000,  
        size,
        PROT_READ | PROT_WRITE,
        MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED,
        -1,
        0
    );
    int16_t *C = mmap(
        (void *)0x40000000,  
        size,
        PROT_READ | PROT_WRITE,
        MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED,
        -1,
        0
    );

    if (A == MAP_FAILED || B == MAP_FAILED || C == MAP_FAILED) {
        perror("Error al mapear la memoria");
        return 1;
    }
    printf("Memoria mapeada correctamente\n");

    m5_exit(0); // Trigger exit event to map memory in the simulated process

    // Inicializamos vectores A y B
    for (int i = 0; i < 16; i++) {
        A[i] = i;
        B[i] = 2 * i;
    }
    printf("Vectores A y B inicializados\n");

    uint32_t *crf_start = (uint32_t *)(pim_region + 2); // CRF starts at offset 2 in the PIM region
    //MOV GRF_A0, BANK0
    printf("Configurando instrucciones CRF...\n");
    crf_start[0] = (3 << 28) | (1 << 25) | (3 << 22) | (0 << 19) | (0 << 8) |
                   (0 << 4) | (0);
    printf("CRF para MOV GRF_A0, BANK0 configurado\n"); 
    //ADD GRF_B0, GRF_A0, BANK0
    crf_start[1] = (4 << 28) | (2 << 25) | (1 << 22) | (3 << 19) | (0 << 8) |
                   (0 << 4) | (0);
    printf("CRF para ADD GRF_B0, GRF_A0, BANK0 configurado\n");
    // MOV BANK0, GRF_B0
    crf_start[2] = (3 << 28) | (3 << 25) | (2 << 22) | (0 << 19) | (0 << 8) |
                   (0 << 4) | (0);
    printf("CRF para MOV BANK0, GRF_B0 configurado\n");
    // EXIT
    crf_start[3] = (2 << 28) | (0 << 25) | (0 << 22) | (0 << 19) | (0 << 8) |
                   (0 << 4) | (0);
    printf("CRF para EXIT configurado\n");
    // ACTIVATE PIM MODE
    pim_region[0] = 1; // Writing to this address activates PIM mode 
    printf("PIM mode activado\n");
    int16_t dummy1, dummy2;
    dummy1 = A[0]; // Read to A's address to trigger the MOV instruction
    dummy2 = B[0]; // Read to B's address to trigger the ADD instruction
    C[0] = 0; // Write to C's address to trigger the MOV instruction to write back the result
    C[0] = 0; // Some memory access to trigger the execution of EXIT
    printf("Resultado de la suma vectorial:\n");
    for(int i = 0; i < 16; i++) {
        printf("C[%d] = %d\n", i, C[i]);
    }

    return 0;
}