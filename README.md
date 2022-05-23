# README #

### What is this? ###

This repository contains sample OpenFOAM solvers and tutorials that show how to use embedded Python interpreters to interact with OpenFOAM solver. Python support and interoperability between the C++/Python data is provided by the pybind11 library [1]. The code in this repository is based on OpenFOAM-7.


### How do I get set up? ###

In addition to an installation of OpenFOAM-7, a Python installation is required. The following Python packages are required, where the versions used to generate the results are shown:

* Python 3.8.12

* NumPy 1.18.5

* pybind11 2.8.1

* TensorFlow 2.4.0

* pip 21.3.1

* Numba 0.54.1

* Matplotlib 3.3.1

* scikit-learn 1.0.1

* pandas 1.1.1

* tqdm 4.50.2

These libraries can be installed from the supplied pybind-no-gpu.yml file using the conda software (https://conda.io). Once conda is installed, the Python environment is installed with:

    conda env create -f pybind-no-gpu.yml

The conda enviroment can be activated with:

    conda activate pybind-no-gpu

Please be aware that the examples may not work with other versions of Python libraries, although they are likely to work with other similar versions, e.g. Python 3.8.*.

One of the test cases uses the solids4foam [2] toolbox. To install it, clone it from https://bitbucket.org/philip_cardiff/solids4foam-release/src/master/. You can then revert to the commit which was used for the examples here before compiling solids4foam, although other versions will probably work too:

    cd $FOAM_RUN/..
    git clone https://bitbucket.org/philip_cardiff/solids4foam-release.git
    cd solids4foam-release
    git reset 8b76740d
    ./Allwmake

Once you have solids4foam on your system, set the following environment variables:

    export SOLIDS4FOAM_INST_DIR=<location_of_solids4foam-release>
    export PYBIND11_INC_DIR=$(python3 -m pybind11 --includes)
    export PYBIND11_LIB_DIR=$(python -c 'from distutils import sysconfig; print(sysconfig.get_config_var("LIBDIR"))')

Then, the OpenFOAM code included in the current repository can be compiled with the Allwmake script in the parent folder:

    ./Allwmake

This will install:

**./applications/solvers/**:

* pythonLaplacianFoam: A wrapper OpenFOAM heat transfer solver which invokes a run-time Python script to solve the governing equations at each time step. This shows how new solvers could be more quickly prototyped using a combination OpenFOAM and Python

* pythonSolids4Foam: This is a version of the solids4Foam [2] solver, where the setRootCase.H header file has been updated as per the description in the paper. This solver is used to demonstrate Python-based mechanical constitutive laws which calculate a volSymmTensorField stress field using Python.

**./src/**:

* pythonVelocity: An OpenFOAM wrapper boundary condition which uses a Python script - supplied at run-time - to define a temporally-spatially varying velocity profile.

* pythonLinearElasticByCopy: Mechanical law that uses pybind11 to call a trained Keras (TensorFlow) neural network mechanical law. OpenFOAM/Python data transfer implicitly copies the data between OpenFOAM and Python.

* pythonLinearElasticByReference: Same as the previous one with the only difference being that data transfer between OpenFOAM and Python is by reference, where the address is passed and the data is not copied.



### How do I run the cases? ###

To run a particular tutorial, navigate to the parent folder for the specific tutorial and execute the provided Allrun script. For instance, to run the pythonLaplacianFoam tutorial:

    cd tutorials/pythonHotBlock
    ./Allrun

All the tutorials can be run with the following command:

    cd tutorials && ./Allrun

Further details on the tutorials can be found in [tutorials/README.md](tutorials/README.md).


It is also possible to run the solid mechanics test cases using a GPU, via the provided AllrunGPU scripts. To do that, install and activate the Python environment pybind-gpu given by the attached file pybind-gpu.yml. This file installs the GPU version of TensorFlow. When using the GPU, please bear in mind:

* The libcudnn.so library must be available in the LD_LIBRARY_PATH. To manually do this, replace the line after # Expose  GPU in AllrunGPU with the specific location to your libcudnn.so
* In the cases presented here, it is expected that using a GPU will actually slow down the calculations as the time required to send data to the GPU is significant relative to the time for the calculations to be performed.
* For parallel runs, the first processor will take access the GPU and then block it for the other processors, so you would need one GPU per processor or alternatively the Python code would need to be updated to deal with the mismatch between CPU cores and GPUs.

Although not been demonstrated in article, the solid mechanics test cases can be run in parallel via the standard OpenFOAM MPI approach without any changes; in that case, each processor has its own local copy of the Python interpreter. To do that, use the provided ./AllrunParallel script.


### Docker notes ###

Alternatively, It is possible to run the code via Docker, as explained next:

First, pull the Docker image:

    docker pull philippic/pybindfoam

Next, create a container from this image:

    docker create --entrypoint /bin/bash --name pybindfoam -it philippic/pybindfoam

Then, start the container running with:
    
    docker start pybindfoam

Finally, attach to the container with:
    
    docker attach pybindfoam

Then when you are done, you can type "exit" to exit the container. This will also stop the container.


### Who do I talk to? ###

    simon.rodriguezluzardo@ucdconnect.ie
    philip.cardiff@ucd.ie
    

### References ###
[1]	W. Jakob, J. Rhinelander, and D. Moldovan, “pybind11 – Seamless operability between C++11 and Python.” 2017.

[2]	P. Cardiff et al., “An open-source finite volume toolbox for solid mechanics and fluid-solid interaction simulations,” ArXiv Prepr. ArXiv180810736, 2018.


