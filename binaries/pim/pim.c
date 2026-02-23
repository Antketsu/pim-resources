#include "pim.h"

int8_t *pim_region;
uint32_t *crf_start;
uint8_t instr_idx = 0;

size_t pim_size = 0x1000000;  // 16 MB

uint64_t next_addr = 0x20000000;

int init_operand(pim_operand *op, uint8_t rows, uint8_t cols){ 
    op->rows = rows;
    op->cols = cols;
    uint8_t elems = rows * cols;
    uint8_t real_elems = SIMD_WIDTH * (elems / SIMD_WIDTH + (elems % SIMD_WIDTH > 0 ? 1 : 0));
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

void write_add_block(uint8_t op_idx){
    //MOV GRF_A0, BANK1
    crf_start[instr_idx++] = DATA_INST(3, 1, 3, 0, op_idx, 0);
    //ADD GRF_B0, GRF_A0, BANK0
    crf_start[instr_idx++] = ALU_INST(4, 2, 1, 3, 0, op_idx, op_idx, 0);
    // MOV BANK0, GRF_B0
    crf_start[instr_idx++] = DATA_INST(3, 3, 2, 0, 0, op_idx);
}


int add(pim_operand A, pim_operand B, pim_operand C){
        uint64_t elems = A.rows * A.cols;
        if(elems != B.rows * B.cols || elems != C.rows * C.cols)
            return 1;

        int16_t *v1 = A.vector;
        int16_t *v2 = B.vector;
        int16_t *v3 = C.vector;

        //We set the maximum to 5 because of the limit of the 32 entries in
        //CRF, this is maximum registers we can use in the worst case
        //where we do 3 instructions for each reg, we have JUMP,
        //a tail of 5 registers and EXIT. 5 * 3 + 1 + 5 * 3 + 1 = 32
        uint8_t regs = elems / SIMD_WIDTH > 5 ? 5 : elems / SIMD_WIDTH;
        uint8_t loops = elems / (SIMD_WIDTH * regs);
        int tail = elems % (SIMD_WIDTH * regs);
        printf("Regs:%d\n", regs);
        printf("Loops:%d\n", loops); 
        printf("Tail:%d\n", tail); 
 
        for(uint8_t i = 0; i < regs; ++i){
            write_add_block(i);        
        }
        if(loops > 1){
            // JUMP 3, loops
            crf_start[instr_idx++] = (1 << 28) | (3 * regs << 11) | (loops - 1);
        }
        for(uint8_t i = 0; tail > 0 && i * SIMD_WIDTH < tail; ++i){
            write_add_block(i);  
        }
        // EXIT
        crf_start[instr_idx++] = CTL_INST(2, 0 , 0);     

        // ACTIVATE PIM MODE
        pim_region[0] = 1; // Writing to this address activates PIM mode
        
        int i = 0, j;
        int16_t dummy1, dummy2; //For fake memory access
        for(; i < loops; ++i){
            for(j = 0; j < regs; ++j){
                dummy1 = v1[i * regs * SIMD_WIDTH + j * SIMD_WIDTH]; // Read to A's address to trigger the MOV instruction
                dummy2 = v2[i * regs * SIMD_WIDTH + j * SIMD_WIDTH]; // Read to B's address to trigger the ADD instruction
                v3[i * regs * SIMD_WIDTH + j * SIMD_WIDTH] = 0; // Write to C's address to trigger the MOV instruction to write back the result
            }
            if(loops > 1)
                v3[0] = 0; // Some memory access to trigger the execution of JUMP
        }
        for(int k = 0; tail > 0 && k * SIMD_WIDTH < tail; ++k){
            dummy1 = v1[(i - 1) * regs * SIMD_WIDTH + j * SIMD_WIDTH]; // Read to A's address to trigger the MOV instruction
            dummy2 = v2[(i - 1) * regs * SIMD_WIDTH + j * SIMD_WIDTH]; // Read to B's address to trigger the ADD instruction
            v3[(i - 1) * regs * SIMD_WIDTH + j * SIMD_WIDTH] = 0; // Write to C's address to trigger the MOV instruction to write back the result
            ++j;
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
    crf_start = (uint32_t *)(pim_region + 4); // CRF starts at offset 2 in the PIM region

    return 0;
}
