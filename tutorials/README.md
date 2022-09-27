# README #

### What is this? ###

This folder contains the files associated to the Python-based OpenFOAM solvers/components presented in the paper:

a) pythonCavity: In this case, a velocity profile boundary condition is created, where a Python script calculates the patch velocities as a function of spatial coordinates and time, as explained in section 3.1. The proposed pythonVelocity boundary condition has been verified on the classic cavity tutorial case, where it is used to set the velocity on the upper moving wall patch.

b) pythonHotBlock: This case demonstrates one of Python's main advantages over C++: fast prototyping. To do this, as an example, the classic laplacianFoam solver is modified such that a run-time selectable Python script is passed mesh, material and time information and is expected to calculate the temperature field, as explained in section 3.2.

c) plateHole: This demonstrates how to perform field calculations using the embedded Python interpreter; specifically, this case shows how this can be done using machine learning models implemented in TensorFlow/Keras, scikit-learn, and Python in general. A solid mechanics problem is chosen, where the stress tensor at each cell is calculated as a function of the displacement gradient via a run-time selectable constitutive mechanical law, as explained in section 3.3.
plateHole has several subfolders for running the case on the different meshes. Each specific case has 3 sub-folders, each one representing one of the linear Hookean law implementations as described in section 3.3. 
If you want to run the code that creates/trains/serialises the neural network, please check [../neuralNetworks/README.md](../neuralNetworks/README.md)
