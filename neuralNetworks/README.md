# README #

### What is this? ###

This folder contains the files to set up/train/save the neural network used in the hole in a plate problem. If you want to run the code, first activate the conda environment:

    conda activate pybind-no-gpu

or , if you want to use the GPU, use:

    conda activate pybind-gpu

Please be aware of the comments on using GPU presented in the main README file; however, neural network training is much faster using a GPU (order of magnitude).

Then, run the code

    python main.py

At run time, additional files such as the serialised versions of the training, validation and test sets, strain and stress samples, and others, are generated. In addition, a few stress/strain plots corresponding to predictions on the test set are generated.

