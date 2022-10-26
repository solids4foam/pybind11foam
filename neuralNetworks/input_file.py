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
#  This file defines the material properties and other settings
#  for training and saving the neural network.

# Author
#  Simon A. Rodriguez, UCD. All rights reserved
#  Philip Cardiff, UCD. All rights reserved

E = 200e9 #Young's modulus
v = 0.3 #Poisson's ratio
lame_1 = E * v / ((1 + v) * (1 - 2 *v))
lame_2 = E / (2 * (1 + v))
strains_path = 'strains'
stresses_path = 'stresses'
max_abs_value_deformation = 0.00005           
number_strain_sequences = 10000 
splitter = [0.7, 0.2, 0.1] #70% for the training set, 20% validation, 10% test
plots_path = 'Plots'
number_of_epochs = 300