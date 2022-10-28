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

# Author
#  Simon A. Rodriguez, UCD. All rights reserved
#  Philip Cardiff, UCD. All rights reserved

import numpy as np
import ctypes as C

def calculate():
    data_pointer_face_centres = C.cast(CAddress, C.POINTER(C.c_double))
    face_centres = np.ctypeslib.as_array(data_pointer_face_centres, shape = (SIZE, 3))
    data_pointer_velocities = C.cast(velocitiesAddress, C.POINTER(C.c_double))
    velocities = np.ctypeslib.as_array(data_pointer_velocities, shape = (SIZE, 3))

    if face_centres.shape[0] != 0:
        # Initialise result
        result = np.zeros(shape = face_centres.shape)

        # Calculate values using the x coordinates and time
        x = face_centres[:, 0]

        # Update the velocity field
        velocities[:, 0] = np.sin(time * np.pi) * np.sin(x * 40 * np.pi)
