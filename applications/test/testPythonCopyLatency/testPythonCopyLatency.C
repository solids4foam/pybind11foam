/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     | Website:  https://openfoam.org
    \\  /    A nd           | Copyright (C) 2011-2018 OpenFOAM Foundation
     \\/     M anipulation  |
-------------------------------------------------------------------------------
License
    This file is part of OpenFOAM.

    OpenFOAM is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    OpenFOAM is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with OpenFOAM.  If not, see <http://www.gnu.org/licenses/>.

Application
    testPythonCopyLatency

Description
    Simple utility to test the latency of copying data between OpenFOAM and
    Python via pybind11.

    This utility needs to find system/controlDict to create the time object but
    no other data from the case is used.

Authors
    Philip Cardiff, UCD.
    Simón A. Rodríguez L., UCD.

\*---------------------------------------------------------------------------*/

#include "argList.H"
#include "Field.H"
#include "Time.H"
#include <pybind11/embed.h>
#include <pybind11/numpy.h>
#include <vector>
#include <pybind11/stl.h>
#include <pybind11/eval.h>

namespace py = pybind11;

using namespace Foam;

// * * * * * * * * * * * * * * * * Heloer Functions  * * * * * * * * * * * * //

void printElapsedTime
(
    const Time& runTime, scalar& elapsedCpuTime, scalar& elapsedClockTime
)
{
    elapsedCpuTime = runTime.elapsedCpuTime() - elapsedCpuTime;
    elapsedClockTime = runTime.elapsedClockTime() - elapsedClockTime;

    Info<< "CPU time taken = " << elapsedCpuTime << nl
        << "Clock time taken = " << elapsedClockTime << nl << endl;

    // Reset elapsed time values
    elapsedCpuTime = runTime.elapsedCpuTime();
    elapsedClockTime = runTime.elapsedClockTime();
}


// * * * * * * * * * * * * * * * * Main Function * * * * * * * * * * * * * * //
int main(int argc, char *argv[])
{
    argList::addOption("nIterations", "int", "Default is 1e6");
    #include "setRootCase2.H"

    const int nIter(args.optionLookupOrDefault("nIterations", 1e6));

    #include "createTime.H"

    Info<< "nIterations = " << nIter << nl << endl;

    // Initialise the python interpreter
    py::initialize_interpreter();
    py::object scope = py::module_::import("__main__").attr("__dict__");

    // Create bool and bool vector
    Info<< "Creating bool, int, and std::vector<bool> " << nl << endl;
    const bool x = true;
    const int a = 0;
    std::vector<bool> xVec(0);

    // Store time before performing copies
    scalar elapsedCpuTime = runTime.elapsedCpuTime();
    scalar elapsedClockTime = runTime.elapsedClockTime();


    // For reference, copy the bool within OpenFOAM
    Info<< "Copy the bool from OpenFOAM to OpenFOAM " << endl;
    for (int i = 0; i < nIter; i++)
    {
        const bool unused = x;
    }
    printElapsedTime(runTime, elapsedCpuTime, elapsedClockTime);


    // For reference, copy the int within OpenFOAM
    Info<< "Copy the int from OpenFOAM to OpenFOAM " << endl;
    for (int i = 0; i < nIter; i++)
    {
        const int unused = a;
    }
    printElapsedTime(runTime, elapsedCpuTime, elapsedClockTime);


    // For reference, copy the bool vector within OpenFOAM
    Info<< "Copy the std::vector<bool> from OpenFOAM to OpenFOAM " << endl;
    for (int i = 0; i < nIter; i++)
    {
        const std::vector<bool> unused = xVec;
    }
    printElapsedTime(runTime, elapsedCpuTime, elapsedClockTime);


    // Copy the bool from OpenFOAM to Python
    Info<< "Copy the bool from OpenFOAM to Python" << endl;
    for (int i = 0; i < nIter; i++)
    {
        scope["x"] = x;
    }
    printElapsedTime(runTime, elapsedCpuTime, elapsedClockTime);


    // Copy the int from OpenFOAM to Python
    Info<< "Copy the int from OpenFOAM to Python" << endl;
    for (int i = 0; i < nIter; i++)
    {
        scope["a"] = a;
    }
    printElapsedTime(runTime, elapsedCpuTime, elapsedClockTime);


    // Copy the bool vector from OpenFOAM to Python
    Info<< "Copy the std::vector<bool> from OpenFOAM to Python" << endl;
    for (int i = 0; i < nIter; i++)
    {
        const py::array xpy = py::cast(xVec);
        scope["xVec"] = xpy;
    }
    printElapsedTime(runTime, elapsedCpuTime, elapsedClockTime);


    // Copy the bool from Python to OpenFOAM
    Info<< "Copy the bool from Python to OpenFOAM" << endl;
    for (int i = 0; i < nIter; i++)
    {
        // Copy the bool from Python to OpenFOAM
        const bool unused = scope["x"].cast<bool>();
    }
    printElapsedTime(runTime, elapsedCpuTime, elapsedClockTime);


    // Copy the int from Python to OpenFOAM
    Info<< "Copy the int from Python to OpenFOAM" << endl;
    for (int i = 0; i < nIter; i++)
    {
        const bool unused = scope["a"].cast<int>();
    }
    printElapsedTime(runTime, elapsedCpuTime, elapsedClockTime);


    // Copy the bool vector from Python to OpenFOAM
    Info<< "Copy the std::vector<bool> from Python to OpenFOAM" << endl;
    for (int i = 0; i < nIter; i++)
    {
        const std::vector<bool> y = scope["xVec"].cast<std::vector<bool>>();
    }
    printElapsedTime(runTime, elapsedCpuTime, elapsedClockTime);


    Info<< nl << "End\n" << endl;

    return 0;
}


// ************************************************************************* //
