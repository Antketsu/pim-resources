#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <gem5/m5ops.h>

int main() {
    // Tamaño de la región de memoria
    size_t size = 0x1000000;  // 16 MB



    volatile uint64_t *mem1 = mmap(
        (void *)0x10000000,  
        size,
        PROT_READ | PROT_WRITE,
        MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED,
        -1,
        0
    );

    if (mem1 == MAP_FAILED) {
        perror("Error al mapear la memoria");
        return 1;
    }

    volatile uint64_t *mem2 = mmap(
        (void *)0x20000000,  
        size,
        PROT_READ | PROT_WRITE,
        MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED,
        -1,
        0
    );

    if( mem2 == MAP_FAILED) {
        perror("Error al mapear la memoria");
        return 1;
    }


    printf("Intentando\n");
    m5_exit(0);
    printf("Saliendo\n");

    // Inicializamos memoria
    *mem1 = 0x1122334455667788;
    *mem2 = 0x99AABBCCDDEEFF00;

    // Operaciones de lectura
    volatile uint64_t rd1 = *mem1;
    volatile uint64_t rd2 = *mem2;

    // Comprobar
    printf("Operaciones de lectura (RD):\n");
    printf("0x%lx\tRD\t0x%lx\n", 0x10000000 + 0x0, rd1);
    printf("0x%lx\tRD\t0x%lx\n", 0x20000000 + 0x0, rd2);

    // Desmapear la memoria, al hacer el remapeo en el exit handler peta si lo dejamos
    //munmap((void *)mem, size);

    return 0;
}
