# License
#  This program is part of pythonPal4Foam.

#  This program is free software: you can redistribute it and/or modify 
#  it under the terms of the GNU General Public License as published 
#  by the Free Software Foundation, either version 3 of the License, 
#  or (at your option) any later version.

#  This program is distributed in the hope that it will be useful, 
#  but WITHOUT ANY WARRANTY; without even the implied warranty of 
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 

#  See the GNU General Public License for more details. You should have 
#  received a copy of the GNU General Public License along with this 
#  program. If not, see <https://www.gnu.org/licenses/>. 

# Description
#  This file has the definitions of some functions that are used when 
#  setting up and training the neural network.

# Author
#  Simon A. Rodriguez, UCD. All rights reserved
#  Philip Cardiff, UCD. All rights reserved


################### Required imports ##########################
import pickle
from tqdm import tqdm
import random
from distutils.dir_util import mkpath
import numpy as np
from numpy import linalg as LA
import pandas as pd
import matplotlib.pyplot as plt
import tensorflow as tf
from tensorflow import keras
from tensorflow.keras import Sequential, Model
from tensorflow.keras.layers import GRU, Dense, Input
from tensorflow.keras.callbacks import ModelCheckpoint
from tensorflow.keras.optimizers import Adam
###############################################################

def plotResults(x, y, y_pred, plots_path, history, number_of_plots):
    places = np.arange(number_of_plots)
    mkpath(plots_path)
    labels = ["sigma_xx", "sigma_yy", "sigma_zz", "sigma_xy", 
              "sigma_xz",  "sigma_yz"]
    labels_pred = ["sigma_xx_pred", "sigma_yy_pred", "sigma_zz_pred", 
                 "sigma_xy_pred", "sigma_xz_pred", "sigma_yz_pred"]
    labels_colors = ["dimgray", "steelblue", "forestgreen", 
                     "indianred", "saddlebrown", "purple",  "gold"]
    labels_colors_predictions = ["lightgray", "deepskyblue", "lime", 
                                 "lightcoral", "peru", "darkviolet", 
                                 "yellow"]
    coord_x = np.array(range(0, x.shape[1] + 1), dtype = float)
    for i in places:
        coord_y = np.zeros(2)            
        coord_y_pred = np.zeros(2)
        plt.figure(figsize=(15, 10))
        ax = plt.subplot(111)
        plot_name  = plots_path + '/test_sample_number_' + str(i) + '.png'
        for j in range(6):
            coord_y[1] = y[i, :, j]
            coord_y_pred[1] = y_pred[i, :, j]   
            plt.plot(coord_x, coord_y, label = labels[j], color = 
                     labels_colors[j])
            plt.plot(coord_x, coord_y_pred, linestyle='dashed', label = 
                     labels_pred[j], color = labels_colors[j])                                       
        box = ax.get_position() #
        ax.set_position([box.x0, box.y0, box.width * 0.8, box.height])
        # Put a legend to the right of the current axis
        ax.legend(loc='center left', bbox_to_anchor=(1, 0.5))
        plt.xlabel("time steps")
        plt.ylabel("Sigma")
        plt.grid(True)
        plt.savefig(plot_name)
        plt.close()
    pd.DataFrame(history.history).plot(figsize=(15, 10))
    plot_name = plots_path + '/Convergence history.png'
    plt.grid(True)
    plt.legend()
    plt.yscale("log")
    plt.xlabel("Epoch")
    plt.ylabel("Loss")
    plt.savefig(plot_name)
    
def trainNN(model, number_of_epochs, x_train, y_train, x_validation, 
            y_validation):
    history = model.fit(x_train, y_train, epochs = number_of_epochs,
                              validation_data = (x_validation, y_validation))
    return history

def createNN():
    model = Sequential()
    model.add(Dense(units = 20, kernel_initializer = 'he_normal', 
                    activation = 'relu', input_shape = (None, 6)))
    model.add(Dense(units = 6, kernel_initializer = 'he_normal', 
                    activation = 'linear'))
    return model

def compileNN(model, slow):
    if (slow):
        opt = tf.keras.optimizers.Adam(
            learning_rate=0.01,
            beta_1=0.99,
            beta_2=0.999999, 
            epsilon=1e-08, 
            amsgrad=True, 
        )
        model.compile(optimizer=opt, loss='mse')
    else:
        model.compile(optimizer=Adam(lr = 0.01), loss='mse')
    
def normaliseDataset(x_train, y_train, x_validation, y_validation, x_test, 
                     y_test, x_scaler, y_scaler):
    master_x_train = x_train.reshape(x_train.shape[0] * x_train.shape[1],
                                     x_train.shape[2])
    master_y_train = y_train.reshape(y_train.shape[0] * y_train.shape[1], 
                                     y_train.shape[2])
    ########## Fitting scalers #####################################
    x_scaler.fit(master_x_train)
    y_scaler.fit(master_y_train)    
    ################################################################
    x_train_normalised = np.zeros(x_train.shape)
    y_train_normalised = np.zeros(y_train.shape)
    for i in range(0, x_train.shape[0]):
      x_train_normalised[i,:,:] =  x_scaler.transform(x_train[i,:,:])
      y_train_normalised[i,:,:] = y_scaler.transform(y_train[i,:,:])
    x_validation_normalised = np.zeros(x_validation.shape)
    y_validation_normalised = np.zeros(y_validation.shape)
    for i in range(0, x_validation.shape[0]):
      x_validation_normalised[i,:,:] = x_scaler.transform(x_validation[i,:,:])
      y_validation_normalised[i,:,:] = y_scaler.transform(y_validation[i,:,:])
    x_test_normalised = np.zeros(x_test.shape)
    y_test_normalised = np.zeros(y_test.shape)      
    for i in range(0, x_test.shape[0]):
      x_test_normalised[i,:,:] = x_scaler.transform(x_test[i,:,:])
      y_test_normalised[i,:,:] = y_scaler.transform(y_test[i,:,:])
    return x_train_normalised, y_train_normalised, x_validation_normalised, \
           y_validation_normalised, x_test_normalised, y_test_normalised
            
def reverseNormaliseSet(array, scaler):
    denormalised = np.zeros(array.shape)
    for i in range(array.shape[0]):
        denormalised[i,:,:] = scaler.inverse_transform(array[i,:,:])     
    return denormalised

def serialiseDataset(x_train, y_train, x_validation, y_validation, x_test, 
                     y_test, y_prediction):
    with open('x_training.pkl', 'wb') as f:
        pickle.dump(x_train, f)
    f.close()
    with open('y_training.pkl', 'wb') as f:
        pickle.dump(y_train, f)
    f.close()
    with open('x_validation.pkl', 'wb') as f:
        pickle.dump(x_validation, f)
    f.close()
    with open('y_validation.pkl', 'wb') as f:
        pickle.dump(y_validation, f)
    f.close()
    with open('x_test.pkl', 'wb') as f:
        pickle.dump(x_test, f)
    f.close()
    with open('y_test.pkl', 'wb') as f:
        pickle.dump(y_test, f)
    f.close()
    with open('y_prediction.pkl', 'wb') as f:
        pickle.dump(y_prediction, f)
    f.close()

def createDataset(strains_path, stresses_path, number_strain_sequences, 
             splitter):
    x_master_array = np.zeros([number_strain_sequences, 1, 6])
    y_master_array = np.zeros([number_strain_sequences, 1, 6])
    ############## Populating the master sets ########################
    for i in tqdm(range(0, number_strain_sequences)):
        x_master_array[i,:,:] = np.loadtxt(strains_path + \
                                           '/%i.txt' %(i)).reshape(1, 6)
        y_master_array[i,:,:] = np.loadtxt(stresses_path + \
                                           '/%i.txt' %(i)).reshape(1, 6)
    ##################################################################
    ####### Selection of samples for each set ########################
    total_indices = range(0, number_strain_sequences)
    training_indices = random.sample(total_indices, 
                                     round(splitter[0] * \
                                           number_strain_sequences))
    possible_validation_indices = list(set(total_indices) - \
                                       set(training_indices))
    validation_indices = random.sample(possible_validation_indices, 
                                          round(splitter[1]\
                                        * number_strain_sequences))
    test_indices = list(set(total_indices) - set(training_indices) - \
                        set(validation_indices))
    ##################################################################

    ################ Creation of the sets ############################
    i = 0
    x_train = np.zeros([len(training_indices), 1, 6])
    y_train = np.zeros([len(training_indices), 1, 6])       
    for j in training_indices:
        x_train[i,:,:] = x_master_array[j,:,:]
        y_train[i,:,:] = y_master_array[j,:,:]
        i = i + 1
    i = 0 
    x_validation = np.zeros([len(validation_indices), 1, 6])
    y_validation = np.zeros([len(validation_indices), 1, 6]) 
    for j in validation_indices:
        x_validation[i,:,:] = x_master_array[j,:,:]
        y_validation[i,:,:] = y_master_array[j,:,:]
        i = i + 1
    i = 0
    x_test = np.zeros([len(test_indices), 1, 6])
    y_test = np.zeros([len(test_indices), 1, 6]) 
    for j in test_indices:
        x_test[i,:,:] = x_master_array[j,:,:]
        y_test[i,:,:] = y_master_array[j,:,:]
        i = i + 1
    #################################################################
    return x_train, y_train, x_validation, y_validation, x_test, y_test
    

def writeStresses(strains_path, stresses_path, number_strain_sequences, ce):
    for i in tqdm(range(0, number_strain_sequences)):
        with open(strains_path+'/%i.txt' %(i), 'r') as f:
            strains = np.loadtxt(strains_path +'/%i.txt' %(i)).reshape(6, 1)
            stresses = ce.dot(strains)
        np.savetxt('./' + stresses_path + '/' + '%i.txt' %(i), stresses,  
                   delimiter = ' ')

def writeStrains(strains_path, number_strain_sequences, 
                 max_abs_value_deformation):
    print("Calculating strains")
    mu, sigma = 0, 1 #mean and variance

    #Inner function
    def calculate_eq_strain(strains):
        trace = (np.sum(strains[:, 0:3], axis = 1)/3).reshape(strains.shape[0], 
                                                              1)         
        dev_strains = strains
        dev_strains[:, 0:3] = dev_strains[:, 0:3] - trace
        eq_strains = (((2/3)*np.sum((dev_strains*dev_strains), axis = 1))**0.5
                      ).reshape(strains.shape[0],1)
        return eq_strains
    
    for i in tqdm(range(0, number_strain_sequences)):
        strains = np.random.normal(mu, sigma, size = (1, 6))
        strains  = strains * 2 * max_abs_value_deformation - \
            max_abs_value_deformation
        eq_strains = calculate_eq_strain(np.copy(strains))
        max_eq_strain = max(abs(eq_strains))
        if (max_eq_strain > max_abs_value_deformation): 
            strains = strains * (max_abs_value_deformation/max_eq_strain)  
        eq_strains = calculate_eq_strain(np.copy(strains))
        np.savetxt('./' + strains_path + '/' + '%i.txt' %(i), strains, 
                                delimiter = ' ')
        
def buildElasticModuli(lame_1, lame_2):
    ce = np.eye(6)
    ce = 2 * lame_2 * ce
    for i in range(0, 3):
        ce[i, 0:3] = lame_1
        ce[i, i] = ce[i, i] + 2 * lame_2
    return ce