import os
os.environ['TF_CPP_MIN_LOG_LEVEL'] = '1' # Disable tf warnings
import numpy as np
import tensorflow as tf
from tensorflow import keras
from joblib import load
import sklearn
from tensorflow.keras import Model
from tensorflow.keras.layers import Dense, Input
import ctypes as C

#Load the keras model from .h5 file
DNN = keras.models.load_model("DNN.h5")

#Load the scalers
x_scaler = load('x_scaler.joblib')
y_scaler = load('y_scaler.joblib')

#Build the new dense model
inputs = Input([None, 6])
dense_layer1 = Dense(units = 20, activation = 'relu')
x1 = dense_layer1 (inputs)
dense_layer2 = Dense(units = 6, activation = 'linear')
x2 = dense_layer2(x1)
model = Model(inputs = inputs, outputs = x2)

#Load the weights from DNN into model
for i in range(len(DNN.layers)):
    model.layers[i+1].set_weights(DNN.layers[i].get_weights())

def predict():
    data_pointer_strain = C.cast(epsilon_address, C.POINTER(C.c_double))
    data_pointer_stress = C.cast(sigma_address, C.POINTER(C.c_double))
    strain_tensor = np.ctypeslib.as_array(data_pointer_strain,shape=(SIZE, 6))
    stress_tensor = np.ctypeslib.as_array(data_pointer_stress,shape=(SIZE, 6))

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
        strain_tensor_scaled.reshape(1,strain_tensor.shape[0],6)
    prediction_scaled = model(strain_tensor_scaled_reshaped, training = False)
    prediction_output_scaled = tf.reshape(prediction_scaled, 
                                          [strain_tensor.shape[0],6])
    stress_tensor[:, :] = y_scaler.inverse_transform(prediction_output_scaled)
    
    #Reorder stress, just like strains
    original_stress_tensor = np.copy(stress_tensor)
    stress_tensor[:, 1] = original_stress_tensor[:, 3]
    stress_tensor[:, 2] = original_stress_tensor[:, 5]
    stress_tensor[:, 3] = original_stress_tensor[:, 1]
    stress_tensor[:, 5] = original_stress_tensor[:, 2]  
