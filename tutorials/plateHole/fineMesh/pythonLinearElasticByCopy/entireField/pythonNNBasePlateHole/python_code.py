import os
os.environ['TF_CPP_MIN_LOG_LEVEL'] = '1' # Disable tf warnings
import numpy as np
import tensorflow as tf
from tensorflow import keras
from joblib import load
import sklearn

model = keras.models.load_model("DNN.h5")

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



