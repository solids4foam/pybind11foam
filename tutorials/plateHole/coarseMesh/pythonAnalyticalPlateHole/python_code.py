import numpy as np
import ctypes as C

E = 200e9 #Young's modulus
v = 0.3 #Poisson's ratio
lame_1 = E * v / ((1 + v) * (1 - 2 * v))
lame_2 = E / (2 * (1 + v))

def predict():
    data_pointer_strain = C.cast(epsilon_address,C.POINTER(C.c_double))
    data_pointer_stress = C.cast(sigma_address,C.POINTER(C.c_double))
    strain_tensor = np.ctypeslib.as_array(data_pointer_strain,shape=(SIZE,6))
    stress_tensor = np.ctypeslib.as_array(data_pointer_stress,shape=(SIZE,6))
    stress_tensor[:, 0] = 2 * lame_2 * strain_tensor[:, 0] \
        + lame_1 * (strain_tensor[:, 0] \
        + strain_tensor[:, 3] + strain_tensor[:, 5])
    stress_tensor[:, 1] = 2 * lame_2 * strain_tensor[:, 1]
    stress_tensor[:, 2] = 2 * lame_2 * strain_tensor[:, 2]
    stress_tensor[:, 3] = 2 * lame_2 * strain_tensor[:, 3] \
        + lame_1 * (strain_tensor[:, 0] \
        + strain_tensor[:, 3] + strain_tensor[:, 5])
    stress_tensor[:, 4] = 2 * lame_2 * strain_tensor[:, 4]
    stress_tensor[:, 5] = 2 * lame_2 * strain_tensor[:, 5] \
        + lame_1 * (strain_tensor[:, 0] \
        + strain_tensor[:, 3] + strain_tensor[:, 5])