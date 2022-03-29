import numpy as np
from joblib import load

#Load the scalers
x_scaler = load('x_scaler.joblib')
y_scaler = load('y_scaler.joblib')

wb = np.load('DNN_weights.npz', allow_pickle = True)
w, b = wb['wb']
w0, w1 = np.array(w[0], dtype = np.float64), np.array(w[1], dtype = np.float64)
b0, b1 = np.array(b[0], dtype = np.float64) , np.array(b[1], dtype = np.float64)

def predict(x):
  x = x_scaler.transform(x)
  x = np.array(x, dtype = np.float64)
  prediction_output_scaled = neural_prediction(x, w0, w1, b0, b1)
  prediction = y_scaler.inverse_transform(prediction_output_scaled)
  return prediction

def neural_prediction(x, w0, w1, b0, b1):
  l0 = x.dot(w0) + b0
  l0 = np.maximum(0, l0)
  l1 = l0.dot(w1) + b1  
  return l1