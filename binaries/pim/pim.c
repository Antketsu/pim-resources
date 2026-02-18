#include "pim.h"

int8_t *pim_region;

size_t pim_size = 0x1000000;  // 16 MB

uint64_t next_addr = 0x20000000;

int init_operand(pim_operand *op, int8_t elems){ 
    op->size = elems;
    uint8_t real_elems = 16 * (elems / 16 + (elems % 16 > 0 ? 1 : 0));
    op->vector = mmap(
        (void *)next_addr,  
        real_elems * 2, // 16 bits per element
        PROT_READ | PROT_WRITE,
        MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED,
        -1,
        0
    );
    if (op->vector == MAP_FAILED) {
        perror("Error al mapear la memoria");
        return 1;
    }
    m5_exit(0);
    memset(op->vector, 0, real_elems);
    next_addr += 0x10000000;
    return 0;
}

int add(pim_operand A, pim_operand B, pim_operand C){
        if(A.size != B.size || A. size != C.size)
            return 1;
        int16_t *v1 = A.vector;
        int16_t *v2 = B.vector;
        int16_t *v3 = C.vector;
        uint16_t loops = A.size / 16;
        uint8_t tail = A.size % 16;
        uint32_t *crf_start = (uint32_t *)(pim_region + 2); // CRF starts at offset 2 in the PIM region
        //MOV GRF_A0, BANK1
        crf_start[0] = (3 << 28) | (1 << 25) | (3 << 22) | (0 << 19) | (0 << 8) |
                    (0 << 4) | (0);
        //ADD GRF_B0, GRF_A0, BANK0
        crf_start[1] = (4 << 28) | (2 << 25) | (1 << 22) | (3 << 19) | (0 << 8) |
                    (0 << 4) | (0);
        // MOV BANK0, GRF_B0
        crf_start[2] = (3 << 28) | (3 << 25) | (2 << 22) | (0 << 19) | (0 << 8) |
                    (0 << 4) | (0);
        // JUMP 3, loops
        crf_start [3] = (1 << 28) | (3 << 11) | (loops - 1);
        if(tail){
            //MOV GRF_A0, BANK1
            crf_start[4] = (3 << 28) | (1 << 25) | (3 << 22) | (0 << 19) | (0 << 8) |
            (0 << 4) | (0);
            //ADD GRF_B0, GRF_A0, BANK0
            crf_start[5] = (4 << 28) | (2 << 25) | (1 << 22) | (3 << 19) | (0 << 8) |
            (0 << 4) | (0);
            // MOV BANK0, GRF_B0
            crf_start[6] = (3 << 28) | (3 << 25) | (2 << 22) | (0 << 19) | (0 << 8) |
            (0 << 4) | (0);
            // EXIT
            crf_start[7] = (2 << 28) | (0 << 25) | (0 << 22) | (0 << 19) | (0 << 8) |
                        (0 << 4) | (0);
        }
        else{
            // EXIT
            crf_start[4] = (2 << 28) | (0 << 25) | (0 << 22) | (0 << 19) | (0 << 8) |
                        (0 << 4) | (0);
        }
        // ACTIVATE PIM MODE
        pim_region[0] = 1; // Writing to this address activates PIM mode
        int i = 0;
        for(; i < loops; ++i){
            int16_t dummy1, dummy2;
            dummy1 = v1[i * 16]; // Read to A's address to trigger the MOV instruction
            dummy2 = v2[i * 16]; // Read to B's address to trigger the ADD instruction
            v3[i * 16] = 0; // Write to C's address to trigger the MOV instruction to write back the result
            v3[0] = 0; // Some memory access to trigger the execution of JUMP
        } 
        if(tail){
            int16_t dummy1, dummy2;
            dummy1 = v1[i * 16]; // Read to A's address to trigger the MOV instruction
            dummy2 = v2[i * 16]; // Read to B's address to trigger the ADD instruction
            v3[i * 16] = 0; // Write to C's address to trigger the MOV instruction to write back the result
        }
        v3[0] = 0; // Some memory access to trigger the execution of EXIT
        return 0;
}

int init_pim(){
    pim_region = mmap(
        (void *)0x10000000,  
        pim_size,
        PROT_READ | PROT_WRITE,
        MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED,
        -1,
        0
    );

    if (pim_region == MAP_FAILED) {
        perror("Error al mapear la región PIM");
        return 1;
    }
    m5_exit(0);
    return 0;
}
