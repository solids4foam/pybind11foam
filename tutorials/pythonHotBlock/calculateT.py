import numpy as np
import ctypes as C

# This script solves the heat equation using an explicit 2-D finite difference
# discretisation.
# It is assumed that:
# 1 - The mesh comprises a single structured square mesh with an
#     equal number of cells in the x and y directions, where the cell numbers
#     increase row by row from one side to the other.
# 2 - The number of cells in the X and Y directions are equal.
# 3 - The boundary cell values have already been updated

# Calculate the temperature field T using gamma
def calculate():

    data_pointer_T = C.cast(T_address, C.POINTER(C.c_double))
    T = np.ctypeslib.as_array(data_pointer_T, shape=(SIZE, 1))

    # Get number of cells in x and y directions
    Nx = np.sqrt(SIZE).astype(int)
    Ny = Nx

    # Reshape T to 2-D array
    T2d = np.reshape(T, (Nx, Ny)).copy()
    newT2d = T2d.copy()

    # Use explicit finite difference method to update the non-boundary cells
    newT2d[1:-1, 1:-1] = (gamma*(T2d[2:, 1:-1] + T2d[:-2, 1:-1] + T2d[1:-1, 2:]
                                 + T2d[1:-1, :-2] - 4*T2d[1:-1, 1:-1])
                          + T2d[1:-1, 1:-1])

    T[:, :] = np.reshape(newT2d, (SIZE, 1))