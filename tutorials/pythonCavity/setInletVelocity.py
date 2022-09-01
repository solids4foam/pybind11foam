import numpy as np
import ctypes as C

def calculate():
    data_pointer_face_centres = C.cast(CAddress, C.POINTER(C.c_double))
    face_centres = np.ctypeslib.as_array(data_pointer_face_centres, shape = (SIZE, 3))
    data_pointer_velocities = C.cast(velocitiesAddress, C.POINTER(C.c_double))
    velocities = np.ctypeslib.as_array(data_pointer_velocities, shape = (SIZE, 3))

    # Return if the array size is zero
    if face_centres.shape[0] == 0:
        return face_centres

    # Initialise result
    result = np.zeros(shape = face_centres.shape)

    # Calculate values using the x coordinates and time
    x = face_centres[:, 0]

    # Update the velocity field
    velocities[:, 0] = np.sin(time * np.pi) * np.sin(x * 40 * np.pi)
