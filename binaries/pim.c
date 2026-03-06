#include "pim.h"

uint8_t *pim_region;
uint32_t *crf;
int16_t *srf_m;
uint8_t instr_idx = 0;

size_t pim_size = 0x1000000;  // 16 MB

uint64_t next_addr = 0x20000000;

uint8_t processing_units = 8;


int init_operand(pim_operand *op, uint8_t rows, uint8_t cols){ 
    op->rows = rows;
    op->cols = cols;
    uint32_t elems = rows * cols;
    uint32_t real_elems = SIMD_WIDTH * (elems / SIMD_WIDTH + (elems % SIMD_WIDTH > 0 ? 1 : 0));
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

int init_operand_all_banks(pim_operand_all_banks *op, uint8_t rows, uint8_t cols){ 
    op->rows = rows;
    op->cols = cols;
    uint32_t elems = rows * cols;
    uint32_t elems_per_bank = elems / 8;
    uint32_t ptr = next_addr;
    for(int i = 0; i < processing_units; ++i){
        op->vectors[i] = mmap(
            (void *)ptr,  
            elems_per_bank * 2, // 16 bits per element
            PROT_READ | PROT_WRITE,
            MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED,
            -1,
            0
        );
        ptr += 0x00010000;
        if (op->vectors[i] == MAP_FAILED) {
            printf("Intentando mapear al banco %d\n", i + 1);
            perror("Error al mapear la memoria del banco");
            return 1;
        }
        m5_exit(0);
        memset(op->vectors[i], 0, elems_per_bank * 2);
    }
    next_addr += 0x10000000;
    return 0;
}

void write_add_block(uint8_t op_idx){
    //MOV GRF_A0, BAN0
    crf[instr_idx++] = DATA_INST(3, 1, 3, 0, op_idx, 0);
    //ADD GRF_B0, GRF_A0, BANK1
    crf[instr_idx++] = ALU_INST(4, 2, 1, 4, 0, op_idx, op_idx, 0);
    // MOV BANK0, GRF_B0
    crf[instr_idx++] = DATA_INST(3, 3, 2, 0, 0, op_idx);
}

int add(pim_operand A, pim_operand B, pim_operand C){
        m5_work_begin(0, 0);
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
            crf[instr_idx++] = (1 << 28) | (3 * regs << 11) | (loops - 1);
        }
        for(uint8_t i = 0; tail > 0 && i * SIMD_WIDTH < tail; ++i){
            write_add_block(i);  
        }
        // EXIT
        crf[instr_idx++] = CTL_INST(2, 0 , 0);     

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
        m5_work_end(0, 0);
        return 0;
}

int add_all_banks(pim_operand_all_banks A, pim_operand_all_banks B, pim_operand_all_banks C){
        m5_work_begin(0, 0);
        uint64_t elems = A.rows * A.cols;
        if(elems != B.rows * B.cols || elems != C.rows * C.cols)
            return 1;

        int16_t *v1 = A.vectors[0]; 
        int16_t *v2 = B.vectors[0];
        int16_t *v3 = C.vectors[0];

        uint8_t loops = elems / processing_units / SIMD_WIDTH;
        printf("Loops:%d\n", loops); 
        write_add_block(0);        
        if(loops > 1){
            // JUMP 3, loops
            crf[instr_idx++] = (1 << 28) | (3 << 11) | (loops - 1);
        }
        // EXIT
        crf[instr_idx++] = CTL_INST(2, 0 , 0);     

        // ACTIVATE ALL BANKS MODE
        *(uint8_t *)(pim_region + 4) = 1; // Writing to this address activates the mode for all banks  

        // ACTIVATE PIM MODE
        pim_region[0] = 1; // Writing to this address activates PIM mode
        
        int16_t dummy1, dummy2; //For fake memory access
        for(int i = 0; i < loops; ++i){
                dummy1 = v1[i * SIMD_WIDTH]; // Read to A's address to trigger the MOV instruction
                dummy2 = v2[i * SIMD_WIDTH]; // Read to B's address to trigger the ADD instruction
                v3[i * SIMD_WIDTH] = 0; // Write to C's address to trigger the MOV instruction to write back the result
            if(loops > 1)
                v3[0] = 0; // Some memory access to trigger the execution of JUMP
        }
        v3[0] = 0; // Some memory access to trigger the execution of EXIT
        m5_work_end(0, 0);
        return 0;
}

int matrix_multiplication(pim_operand A, pim_operand B, pim_operand C){
    if(A.cols != B.rows || C.rows != A.rows || C.cols != B.cols)
        return 1;
    uint8_t loops = B.cols / SIMD_WIDTH;
    //MOV GRFB, BANK0 
    crf[instr_idx++] = DATA_INST(3, 2, 3, 0, 0, 0);
    //MAC GRFB, BANK1, SRFM
    crf[instr_idx++] = ALU_INST(7, 2, 3, 4, 0, 0, 0, 0);
    //MOV BANK0 GRFB
    crf[instr_idx++] = DATA_INST(3, 3, 2, 0, 0, 0);
    //JUMP 3, loops
    crf[instr_idx++] = CTL_INST(1, 3, loops - 1);
    //EXIT
    crf[instr_idx++] = CTL_INST(2, 0, 0);

    int16_t dummy;
    for(int k = 0; k < A.rows; ++k){
        for(int i = 0; i < A.cols; ++i){
            srf_m[0] = A.vector[k * A.cols + i];
            pim_region[0] = 1; // Writing to this address activates PIM mode
            int j = 0;        
            do{
                dummy = C.vector[k * C.cols + j * SIMD_WIDTH]; // Read to C's address
                dummy = B.vector[i * B.cols + j * SIMD_WIDTH]; //Read from B
                C.vector[k * C.cols + j * SIMD_WIDTH] = 0; //Write to C
                A.vector[0] = 0; // JUMP
                ++j;
            }while(j < loops);
            A.vector[0] = 0; //EXIT
        }
    }

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
    crf = (uint32_t *)(pim_region + 8); 
    srf_m = (int16_t *)(crf + 32);
    return 0;
}
