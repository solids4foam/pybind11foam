# -*- coding: utf-8 -*-

E = 200e9 #Young's modulus
v = 0.3 #Poisson's ratio
lame_1 = E * v / ((1 + v) * (1 - 2 *v))
lame_2 = E / (2 * (1 + v))
strains_path = 'strains'
stresses_path = 'stresses'
max_abs_value_deformation = 0.00005#0.0001#0.00025#0.05           
number_strain_sequences = 10000 #1000
splitter = [0.7, 0.2, 0.1] #70% for the training set, 20% validation, 10% test
plots_path = 'Plots'
number_of_epochs = 300#200