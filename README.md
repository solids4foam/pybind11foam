# README #

### What is this? ###

This repository contains OpenFOAM code and tutorials that show how to use Python interpreters to interact with OpenFOAM. Python support and interoperability between the C++/Python data are provided by the pybind11 library [1]. The code in this repository is based on OpenFOAM-7, however, as noted below, the procedures are expected to compile with similar versions and forks of OpenFOAM.




### [Manual approach] How do I get set up? ###

In addition to an installation of OpenFOAM-7, a Python installation is required. The following Python packages are required, where the versions used to generate the results are:

* Python 3.8.12

* NumPy 1.18.5

* pybind11 2.8.1

* TensorFlow 2.4.0

* pip 21.3.1

* Matplotlib 3.3.1

* scikit-learn 1.0.1

* pandas 1.1.1

* tqdm 4.50.2

These libraries can be installed from the supplied pybind-no-gpu.yml file using the conda software (https://conda.io). Once conda is installed, the Python environment is installed with:

    conda env create -f pybind-no-gpu.yml

The conda environment can be activated with:

    conda activate pybind-no-gpu

Please be aware that the examples may not work with other versions of Python libraries, although they are likely to work with similar versions, e.g. Python 3.8.*.

One of the test cases uses the solids4foam [2] toolbox. To install it, clone it from https://bitbucket.org/philip_cardiff/solids4foam-release/src/master/. You can then revert to the commit which was used for the examples here before compiling solids4foam, although other versions will likely work too:

    cd $FOAM_RUN/..
    git clone https://bitbucket.org/philip_cardiff/solids4foam-release.git
    cd solids4foam-release
    git reset 8b76740d
    ./Allwmake

Once you have solids4foam on your system, set the following environment variable:

    export SOLIDS4FOAM_INST_DIR=<location_of_solids4foam-release>

In addition, two pybind11 environment variables must be defined, for example, as:

    export PYBIND11_INC_DIR=$(python3 -m pybind11 --includes)
    export PYBIND11_LIB_DIR=$(python3 -c 'from distutils import sysconfig; print(sysconfig.get_config_var("LIBDIR"))')

Then, the OpenFOAM code included in the current repository can be compiled with the Allwmake script in the parent folder:

    ./Allwmake

This will install:

**./applications/solvers/**:

* pythonLaplacianFoam: A wrapper OpenFOAM heat transfer solver which invokes a run-time Python script to solve the governing equations using a finite difference approach at each time step. This shows how new solvers could be more quickly prototyped using a combination of OpenFOAM and Python.

* pythonSolids4Foam: This is a version of the solids4Foam [2] solver, where the setRootCase.H header file has been updated as per the description in the paper. This solver is used to demonstrate Python-based mechanical constitutive laws, which calculate a volSymmTensorField stress field using Python.

**./src/**:

* pythonVelocity: An OpenFOAM wrapper boundary condition which uses a Python script - supplied at run-time - to define a temporally-spatially varying velocity profile.
* pythonLinearElasticByCopy: A mechanical law for soldis4Foam that uses pybind11 to call a trained Keras (TensorFlow) neural network mechanical law. Data transfer between OpenFOAM and Python is achieved by copying the data.
* pythonLinearElasticByReference: Same as the previous one, the only difference being that data transfer between OpenFOAM and Python is "by reference", where the address is passed, and the data is not copied.



### [Docker approach] How do I get set up? ###

As an alternative to manually installing the repository and required dependencies, the repository is also available as a public Docker image.

The Docker image can be pulled with:

    docker pull philippic/pybindfoam

Next, create a container from this image:

    docker create --entrypoint /bin/bash --name pybindfoam -it philippic/pybindfoam

Then, start the container running with:

    docker start pybindfoam

Finally, attach to the container with:

    docker attach pybindfoam

When you are done, you can type "exit" to exit the container; this will also stop the container.



### How do I run the cases? ###

To run a particular tutorial, navigate to the parent folder for the specific tutorial and execute the provided Allrun script. For instance, to run the pythonLaplacianFoam tutorial:

    cd tutorials/pythonHotBlock
    ./Allrun

All the tutorials can be run with the following command (this takes ~5 hr on a modern system):

    cd tutorials && ./Allrun

Further details on the tutorials can be found in [tutorials/README.md](tutorials/README.md).

### Running in parallel

To run any of the cases in parallel, pass the argument "parallel" to the Allrun script, e.g.

    ./Allrun parallel

### Running on a GPU

For the solid mechanics test cases, it is possible to use a GPU to evaluate the Keras neural networks. To do that, install and activate the Python environment "pybind-gpu" given by the included file "pybind-gpu.yml". This file installs the GPU version of TensorFlow. When using the GPU, please bear in mind:

* Pass the argument "GPU" to the Allrun scripts, e.g.

        ./Allrun GPU

* The libcudnn.so library must be available in the LD_LIBRARY_PATH. To manually do this, replace the line after "# Expose  GPU" in the Allrun script with the specific location of your libcudnn.so.
* In the cases presented here, it is expected that using a GPU will slow down the calculations as the time required to send data to the GPU is significant relative to the time for the calculations to be performed; however, the GPU approach is left for demonstration purposes.
* For parallel runs with a GPU, the first processor will access the GPU and then block it for the other processors so that you would need one GPU per processor, or the Python code would need to be updated to deal with the mismatch between CPU cores and GPUs.



### Compatible OpenFOAM versions ###

The general pybind11 approach is independent of the OpenFOAM version/fork and is expected to work with all main versions. The included code compiles with the following versions and forks (it will probably work with others too): 

* OpenFOAM-7
* OpenFOAM-9
* OpenFOAM-v2012
* foam-extend-4.1

However, the presented tutorials have only been tested using OpenFOAM-7. 

If desired, the user can make the small changes required to get the cases to work with their particular version of OpenFOAM.

Note that the test cases which use solids4foam (third test case) are limited to the OpenFOAM versions supported by solids4foam. To use OpenFOAM-9, OpenFOAM-v2012 and foam-extend-4.1, you need to use the “nextRelease” branch of solids4foam.




### Who do I talk to? ###

    TO BE ADDED ONCE THE PAPER HAS BEEN ACCEPTED



### References ###

[1]	W. Jakob, J. Rhinelander, and D. Moldovan, “pybind11 – Seamless operability between C++11 and Python.” 2017.

[2]	P. Cardiff et al., “An open-source finite volume toolbox for solid mechanics and fluid-solid interaction simulations,” ArXiv Prepr. ArXiv180810736, 2018.

