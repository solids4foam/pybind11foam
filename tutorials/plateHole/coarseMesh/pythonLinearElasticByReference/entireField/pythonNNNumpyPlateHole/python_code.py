import numpy as np
from joblib import load
import ctypes as C

#Load the scalers
x_scaler = load('x_scaler.joblib')
y_scaler = load('y_scaler.joblib')

wb = np.load('DNN_weights.npz', allow_pickle=True)
w, b = wb['wb']
w0, w1 = np.array(w[0], dtype=np.float64), np.array(w[1], dtype=np.float64)
b0, b1 = np.array(b[0], dtype=np.float64) , np.array(b[1], dtype=np.float64)

def predict():
  data_pointer_strain = C.cast(epsilon_address, C.POINTER(C.c_double))
  data_pointer_stress = C.cast(sigma_address, C.POINTER(C.c_double))
  strain_tensor = np.ctypeslib.as_array(data_pointer_strain,shape=(SIZE,6))
  stress_tensor = np.ctypeslib.as_array(data_pointer_stress,shape=(SIZE,6))

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
  prediction_output_scaled = neural_prediction(strain_tensor_scaled, w0, w1, b0, b1)
  stress_tensor[:, :] = y_scaler.inverse_transform(prediction_output_scaled)

  #Reorder stress, just like strains
  original_stress_tensor = np.copy(stress_tensor)
  stress_tensor[:, 1] = original_stress_tensor[:, 3]
  stress_tensor[:, 2] = original_stress_tensor[:, 5]
  stress_tensor[:, 3] = original_stress_tensor[:, 1]
  stress_tensor[:, 5] = original_stress_tensor[:, 2]  

def neural_prediction(x, w0, w1, b0, b1):
  l0 = x.dot(w0) + b0
  l0 = np.maximum(0, l0)
  l1 = l0.dot(w1) + b1  
  return l1