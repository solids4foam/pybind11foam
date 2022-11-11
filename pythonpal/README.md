# README #

### What is this? ###

This section contains OpenFOAM code and tutorials that show how to use Python interpreters to interact with OpenFOAM via pythonPal. pythonPal offers Python support and interoperability between the C++/Python data based on the pybind11 library [1]. The code in this repository is based on OpenFOAM-9, however, as noted below, the procedures are expected to compile with similar versions and forks of OpenFOAM.

### [Manual approach] How do I get set up? ###

In addition to an installation of OpenFOAM-9, a Python installation is required. The following libraries are required to run the tutorials:

* Python 3.8.12

* NumPy 1.18.5

* pybind11 2.8.1

These libraries can be installed from the supplied pythonpal-no-gpu.yml file using the conda software (https://conda.io). Once conda is installed, the Python environment is installed with:

    conda env create -f pythonPal-no-gpu.yml

The conda environment can be activated with:

    conda activate pythonPal-no-gpu

Please be aware that the examples may not work with other versions of Python libraries, although they are likely to work with similar versions, e.g. Python 3.8.*.

In addition, two pybind11 environment variables must be defined, for example, as:

    export PYBIND11_INC_DIR=$(python3 -m pybind11 --includes)
    export PYBIND11_LIB_DIR=$(python3 -c 'from distutils import sysconfig; print(sysconfig.get_config_var("LIBDIR"))')

Once those environment variables have been defined, the OpenFOAM code included in the current section can be compiled with the Allwmake script in the parent folder:

    ./Allwmake

If the Allwmake script gives the error “libpython or lpython not found” then please manually update the LD_LIBRARY_PATH environment variable with "export LD_LIBRARY_PATH=$PYBIND11_LIB_DIR:$LD_LIBRARY_PATH” and run the “./Allwmake” command again.

This will install:

**./applications/solvers/**:

* pythonPalIcoFoam: This is a version of icoFoam that, once the simulation has converged, passes the velocity field to Python where the specific kinetic energy (k) is calculated. Then, OpenFOAM prints the k field.

### How do I run the cases? ###

Currently, there is only one tutorial case which solves the cavity case using pythonPalIcoFoam, as explained in the paper. To run it:

    cd tutorials/pythonPalIcoFoam/cavity
    ./Allrun

### Compatible OpenFOAM versions ###

pythonPal is independent of the OpenFOAM version/fork and is expected to work with all main versions. The included code compiles with the following versions and forks (it will probably work with others too): 

* OpenFOAM-9
* OpenFOAM-v2012
* foam-extend-4.1

The presented tutorial has been tested using these three versions. 

If desired, the user can make the small changes required to get the cases to work with their particular version of OpenFOAM.


### Who do I talk to? ###

    Simon Rodriguez
    simon.rodriguezluzardo@ucdconnect.ie
    https://www.linkedin.com/in/simonrodriguezl/

    Philip Cardiff
    philip.cardiff@ucd.ie
    https://www.linkedin.com/in/philipcardiff/


### References ###

[1]	W. Jakob, J. Rhinelander, and D. Moldovan, “pybind11 – Seamless operability between C++11 and Python.” 2017.