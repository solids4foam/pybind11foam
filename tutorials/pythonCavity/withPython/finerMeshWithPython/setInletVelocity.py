import numpy as np

def calculate(face_centres, time):
    # Initialise result
    result = np.zeros(shape = face_centres.shape)

    # Calculate values using the x coordinates and time
    x = face_centres[:, 0]
    result[:,0] = np.sin(time * np.pi) * np.sin(x * 40 * np.pi)

    return result