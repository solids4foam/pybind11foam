import os
os.environ['TF_CPP_MIN_LOG_LEVEL'] = '1' # Disable tf warnings
import numpy as np
import tensorflow as tf
from tensorflow import keras
from joblib import load
import sklearn
from tensorflow.keras import Model
from tensorflow.keras.layers import Dense, Input

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

def predict(x_new):
    x_new_scaled = x_scaler.transform(x_new)
    x_new_scaled_reshaped = x_new_scaled.reshape(1, x_new.shape[0], 6)
    prediction_scaled = model(x_new_scaled_reshaped, training = False)
    prediction_output_scaled = tf.reshape(prediction_scaled, [x_new.shape[0],6])
    prediction = y_scaler.inverse_transform(prediction_output_scaled)
    return prediction
