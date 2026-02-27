import numpy as np

# Inicialización con listas de comprensión
A = np.array([[2 * i + j for j in range(16)] for i in range(16)], dtype=np.int16)  # 1x16
B = np.array([[i + j for j in range(64)] for i in range(16)], dtype=np.int16)  # 16x16

# Multiplicación de matrices
C = np.matmul(A, B)

# Mostrar resultados
for i in range(16):
    for j in range(64):
        print(f"C[{i}][{j}] = {C[i][j]}")