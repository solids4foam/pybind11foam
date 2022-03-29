#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import numpy as np
from numpy import linalg as LA
from distutils.dir_util import mkpath
from functions import *
from input_file import *
from sklearn.preprocessing import MinMaxScaler
import random
from joblib import dump

######################## Check GPU is visible ################################
print("Num GPUs Available: ", len(tf.config.list_physical_devices('GPU')))
print("Num GPUs Available: ", 
      len(tf.config.experimental.list_physical_devices('GPU')))
##############################################################################

########################## seed everything ###################################
seed = 2
random.seed(seed)
np.random.seed(seed)
tf.random.set_seed(seed)
# ##############################################################################

############################ Elastic constants ################################
ce = buildElasticModuli(lame_1, lame_2)
##############################################################################

############## Create the folders for strains and stresses ####################
mkpath(strains_path)
mkpath(stresses_path)
##############################################################################

############## Write the strains/stresses sequences ###########################
writeStrains(strains_path, number_strain_sequences, max_abs_value_deformation)
writeStresses(strains_path, stresses_path, number_strain_sequences, ce)
##############################################################################

#################### Create the scalers #######################################
x_scaler =  MinMaxScaler()
y_scaler  =  MinMaxScaler()
###############################################################################

####################### Create the dataset ####################################   
x_train, y_train, x_validation, y_validation, x_test, y_test = \
createDataset(strains_path, stresses_path, number_strain_sequences, splitter)
###############################################################################    

####################### Normalise the dataset #################################
x_train_normalised, y_train_normalised, x_validation_normalised, \
y_validation_normalised, x_test_normalised, y_test_normalised = \
    normaliseDataset(x_train, y_train, x_validation, y_validation, x_test, 
                     y_test, x_scaler, y_scaler)
###############################################################################

########################### Create the NN #####################################
DNN = createNN()
compileNN(DNN, True)
history = trainNN(DNN, number_of_epochs, x_train_normalised, y_train_normalised, 
        x_validation_normalised, y_validation_normalised)
DNN.summary()
###############################################################################

########################### Perform the predictions ###########################
y_prediction_normalised = DNN.predict(x_test_normalised)
y_prediction = reverseNormaliseSet(y_prediction_normalised, y_scaler)
print("Evaluation score:")
DNN.evaluate(x_test_normalised, y_test_normalised)
###############################################################################

# ########################  Save the model  ###################################
DNN.save("DNN.h5")
# #############################################################################

######################### Plot the results ####################################
plotResults(x_test, y_test, y_prediction, plots_path, history, 10)
###############################################################################

########### Serialise the dataset and the scalers ############################
serialiseDataset(x_train, y_train, x_validation, y_validation, x_test, y_test, 
                 y_prediction)
# Serialise the scalers
dump(x_scaler, 'x_scaler.joblib')
dump(y_scaler, 'y_scaler.joblib')
###############################################################################