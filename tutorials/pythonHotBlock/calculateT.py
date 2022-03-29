import numpy as np

# This script solves the heat equation using an explicit 2-D finite difference
# discretisation.
# It is assumed that:
# 1 - The mesh comprises a single structured square mesh with an
#     equal number of cells in the x and y directions, where the cell numbers
#     increase row by row from one side to the other.
# 2 - The number of cells in the X and Y directions are equal.
# 3 - The boundary cell values have already been updated

# Calculate the temperature field T using gamma
def calculate(T, gamma):

    # Get number of cells in x and y directions
    N = T.shape[0]
    Nx = np.sqrt(N).astype(int)
    Ny = Nx

    # Use explicit finite difference method to update the non-boundary cells
    for i in range(1, Nx - 1):
        #print(i)
        for j in range(1, Ny - 1):
            T[i * Ny + j] = \
                gamma * (T[i * Ny + j + 1] + T[i * Ny + j - 1] \
                    + T[(i + 1) * Ny + j] + T[(i - 1) * Ny + j] \
                    - 4 * T[i * Ny + j]) + T[i * Ny + j]

    # Return values 
    return T


