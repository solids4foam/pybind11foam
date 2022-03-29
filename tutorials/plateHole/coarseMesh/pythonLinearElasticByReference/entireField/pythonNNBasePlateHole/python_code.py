import os
os.environ['TF_CPP_MIN_LOG_LEVEL'] = '1' # Disable tf warnings
import numpy as np
import tensorflow as tf
from tensorflow import keras
from joblib import load
import sklearn
import ctypes as C

model = keras.models.load_model("DNN.h5")

#Load the scalers
x_scaler = load('x_scaler.joblib')
y_scaler = load('y_scaler.joblib')

def predict():
    data_pointer_strain = C.cast(epsilon_address, C.POINTER(C.c_double))
    data_pointer_stress = C.cast(sigma_address, C.POINTER(C.c_double))
    strain_tensor = np.ctypeslib.as_array(data_pointer_strain, 
                                          shape = (SIZE, 6))
    stress_tensor = np.ctypeslib.as_array(data_pointer_stress, 
                                          shape = (SIZE, 6))

    #Reorder the components of strain to match the order in the
    #trained NN
    #Order NN: e_xx, e_yy, e_zz, e_xy, e_yz, e_zx
    #Order OpenFOAM: e_xx, e__xy, e_xz, e_yy, e_yz, e_zz
    original_strain_tensor = np.copy(strain_tensor)
    strain_tensor[:, 1] = original_strain_tensor[:, 3]
    strain_tensor[:, 2] = original_strain_tensor[:, 5]
    strain_tensor[:, 3] = original_strain_tensor[:, 1]
    strain_tensor[:, 5] = original_strain_tensor[:, 2]
    strain_tensor_scaled = x_scaler.transform(strain_tensor)
    strain_tensor_scaled_reshaped = \
        strain_tensor_scaled.reshape(1, strain_tensor.shape[0], 6)
    prediction_scaled = model.predict(strain_tensor_scaled_reshaped)
    prediction_output_scaled = \
        prediction_scaled.reshape(strain_tensor.shape[0], 6)
    stress_tensor[:, :] = y_scaler.inverse_transform(prediction_output_scaled)

    #Reorder stress, just like strains
    original_stress_tensor = np.copy(stress_tensor)
    stress_tensor[:, 1] = original_stress_tensor[:, 3]
    stress_tensor[:, 2] = original_stress_tensor[:, 5]
    stress_tensor[:, 3] = original_stress_tensor[:, 1]
    stress_tensor[:, 5] = original_stress_tensor[:, 2]  
