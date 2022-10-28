# License
#  This program is part of pythonPal4Foam.

#  This program is free software: you can redistribute it and/or modify 
#  it under the terms of the GNU General Public License as published 
#  by the Free Software Foundation, either version 3 of the License, 
#  or (at your option) any later version.

#  This program is distributed in the hope that it will be useful, 
#  but WITHOUT ANY WARRANTY; without even the implied warranty of 
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 

#  See the GNU General Public License for more details. You should have 
#  received a copy of the GNU General Public License along with this 
#  program. If not, see <https://www.gnu.org/licenses/>. 

# Description
#  This script solves the heat equation using an explicit 2-D finite difference
#  discretisation.
#  It is assumed that:
#  1 - The mesh comprises a single structured square mesh with an
#     equal number of cells in the x and y directions, where the cell numbers
#     increase row by row from one side to the other.
#  2 - The number of cells in the X and Y directions are equal.
#  3 - The boundary cell values have already been updated

# Author
#  Simon A. Rodriguez, UCD. All rights reserved
#  Philip Cardiff, UCD. All rights reserved

import numpy as np
import ctypes as C

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