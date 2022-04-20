#------------------------------------------------------------------------------#
# Dockerfile to create a docker image for the article "A general approach for  #
# running Python codes in OpenFOAM using an embedded pybind11 Python           #
# interpreter".                                                                #
#                                                                              #
# The image contains OpenFOAM-7 and conda with the required packages and       #
# environmental variaables.                                                    #
#                                                                              #
# Authors: to be added                                                         #
# Date: April 2022                                                             #
#------------------------------------------------------------------------------#

# Use available OpenFOAM-7 image as starting point
FROM philippic/openfoam7-graphical-apps.gfortran:latest

# File Author / Maintainer
# MAINTAINER NAME_TO_BE_ADDED EMAIL_TO_BE_ADDED

# Change default shell to bash
SHELL ["/bin/bash", "-c"]

# Change to root user
USER root

# Update the repository sources list
RUN apt update && apt install -y sudo


#
# Install conda
#

# Install the public GPG key to trusted store
RUN curl https://repo.anaconda.com/pkgs/misc/gpgkeys/anaconda.asc | gpg --dearmor > conda.gpg
RUN install -o root -g root -m 644 conda.gpg /usr/share/keyrings/conda-archive-keyring.gpg

# Check whether fingerprint is correct (will output an error message otherwise)
RUN gpg --keyring /usr/share/keyrings/conda-archive-keyring.gpg --no-default-keyring --fingerprint 34161F5BF5EB1D4BFBBB8F0A8AEB4F8B29D82806

# Add conda Debian repo
RUN echo "deb [arch=amd64 signed-by=/usr/share/keyrings/conda-archive-keyring.gpg] https://repo.anaconda.com/pkgs/misc/debrepo/conda stable main" > /etc/apt/sources.list.d/conda.list

# Add conda load script to bashrc
RUN echo "source /opt/conda/etc/profile.d/conda.sh" >> /home/openfoam/.bashrc
RUN echo "conda activate pybind-no-gpu" >> /home/openfoam/.bashrc

# Install conda
RUN apt update && apt install -y conda


#
# Install conda environment
#

# Copy conda environment file
COPY pybind-no-gpu.yml /home/openfoam

# Change working directory
WORKDIR /home/openfoam

# Create enviroment
RUN source /opt/conda/etc/profile.d/conda.sh \
    && conda env create -f pybind-no-gpu.yml


#
# Install solids4foam
#

# Change to the user "openfoam"
USER openfoam

# Download
RUN git clone https://bitbucket.org/philip_cardiff/solids4foam-release.git

# Change working directory
WORKDIR /home/openfoam/solids4foam-release

# Source bash and build
RUN source /opt/openfoam7/etc/bashrc \
    && git reset 8b76740d && ./Allwmake

# Set environmental variable
RUN export SOLIDS4FOAM_INST_DIR=/home/openfoam/solids4foam-release
RUN echo "export SOLIDS4FOAM_INST_DIR=/home/openfoam/solids4foam-release" \
    >> /home/openfoam/.bashrc


#
# Build pybindfoam
#

# Change working directory
WORKDIR /home/openfoam/

# Copy files
COPY applications /home/openfoam/applications
COPY src /home/openfoam/src
COPY tutorials /home/openfoam/tutorials
COPY Allwmake /home/openfoam
COPY README.md /home/openfoam

# Switch to the root and change the ownership of files to the user "openfoam"
USER root
RUN chown -R openfoam: /home/openfoam
USER openfoam

# Add pybind11 variable definitions to the bash  
RUN echo "export PYBIND11_INC_DIR='-I/opt/conda/envs/pybind-no-gpu/include/python3.8 -I/opt/conda/envs/pybind-no-gpu/lib/python3.8/site-packages/pybind11/include'" \
    >> /home/openfoam/.bashrc \
    && echo "export PYBIND11_LIB_DIR=/opt/conda/envs/pybind-no-gpu/lib" \
    >> /home/openfoam/.bashrc \
    && echo "export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$PYBIND11_LIB_DIR" \
    >> /home/openfoam/.bashrc

# Source bash, load conda environment and build
RUN source /home/openfoam/.bashrc \
    && source /opt/conda/etc/profile.d/conda.sh \
    && conda activate pybind-no-gpu \
    && source /opt/openfoam7/etc/bashrc \
    && export SOLIDS4FOAM_INST_DIR=/home/openfoam/solids4foam-release \
    && export PYBIND11_INC_DIR="-I/opt/conda/envs/pybind-no-gpu/include/python3.8 -I/opt/conda/envs/pybind-no-gpu/lib/python3.8/site-packages/pybind11/include" \
    && export PYBIND11_LIB_DIR="/opt/conda/envs/pybind-no-gpu/lib" \
    && ./Allwmake