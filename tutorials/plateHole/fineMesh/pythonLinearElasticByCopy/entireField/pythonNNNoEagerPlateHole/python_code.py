import os
os.environ['TF_CPP_MIN_LOG_LEVEL'] = '1' # Disable tf warnings
import numpy as np
import tensorflow as tf
from tensorflow import keras
from tensorflow.keras.optimizers import Adam
from tensorflow.keras import Sequential, Model
from tensorflow.keras.layers import GRU, Dense, Input
from joblib import load
import sklearn


# #################### Disables eager execution ################################
tf.compat.v1.disable_eager_execution()
print("tf.executing_eagerly() is", tf.executing_eagerly()) # False 
# ##############################################################################

model = Sequential()
model.add(Dense(units = 20, kernel_initializer = 'he_normal', 
                activation = 'relu', input_shape = (None, 6)))
model.add(Dense(units = 6, kernel_initializer = 'he_normal', 
                activation = 'linear'))
model.compile(optimizer = Adam(lr = 0.01), loss = 'mse')
model.load_weights("DNN.h5")


#Load the scalers
x_scaler = load('x_scaler.joblib')
y_scaler = load('y_scaler.joblib')

def predict(x_new):
    x_new_scaled = x_scaler.transform(x_new)
    x_new_scaled_reshaped = x_new_scaled.reshape(1,x_new.shape[0], 6)
    prediction_scaled = model.predict(x_new_scaled_reshaped)
    prediction_output_scaled = prediction_scaled.reshape(x_new.shape[0], 6)
    prediction = y_scaler.inverse_transform(prediction_output_scaled)
    return prediction