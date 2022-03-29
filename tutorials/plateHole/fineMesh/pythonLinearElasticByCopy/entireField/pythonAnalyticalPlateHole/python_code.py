import numpy as np

E = 200e9 #Young's modulus
v = 0.3 #Poisson's ratio
lame_1 = E * v / ((1 + v) * (1 - 2 * v))
lame_2 = E / (2 * (1 + v))

def predict(strain_tensor):
    stress = np.zeros([strain_tensor.shape[0], 6])
    stress[:, 0] = 2 * lame_2 * strain_tensor[:, 0] \
        + lame_1 * (strain_tensor[:, 0] \
        + strain_tensor[:, 1] + strain_tensor[:, 2])
    stress[:, 1] = 2 * lame_2 * strain_tensor[:, 1] \
        + lame_1 * (strain_tensor[:, 0] \
        + strain_tensor[:, 1] + strain_tensor[:, 2])
    stress[:, 2] = 2 * lame_2 * strain_tensor[:, 2] \
        + lame_1 * (strain_tensor[:, 0] \
        + strain_tensor[:, 1] + strain_tensor[:, 2])
    stress[:, 3] = 2 * lame_2 * strain_tensor[:, 3]
    stress[:, 4] = 2 * lame_2 * strain_tensor[:, 4]
    stress[:, 5] = 2 * lame_2 * strain_tensor[:, 5]
    return stress


