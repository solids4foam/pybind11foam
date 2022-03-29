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
    pythonLaplacianFoam

Description
    Calls python via pybind11 to calculate the T field at each time-step.

Authors
    Philip Cardiff, UCD.
    Simón A. Rodríguez L., UCD.

\*---------------------------------------------------------------------------*/

#include "fvCFD.H"
#include "fvOptions.H"
#include "simpleControl.H"

// pybind and python headers
#include <pybind11/embed.h>
#include <pybind11/numpy.h>
#include <vector>
#include <pybind11/stl.h>
#include <pybind11/eval.h>

namespace py = pybind11;

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

int main(int argc, char *argv[])
{
    #include "setRootCaseLists2.H"

    #include "createTime.H"
    #include "createMesh.H"

    simpleControl simple(mesh);

    #include "createFields.H"
    #include "createPythonObjects.H"

    // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

    Info<< "\nCalculating temperature distribution\n" << endl;

    while (simple.loop(runTime))
    {
        Info<< "Time = " << runTime.timeName() << nl << endl;

        while (simple.correctNonOrthogonal())
        {
            // Take a reference to the internal field
            scalarField& TI = T.primitiveFieldRef();

            // Set boundary cell values to be equal to the boundary values as we
            // will use a finite difference method in the python script
            // These boundary cells will be the boundary nodes in the finite
            // difference code
            forAll(T.boundaryFieldRef(), patchI)
            {
                const labelUList& faceCells =
                    mesh.boundary()[patchI].faceCells();
                forAll(faceCells, faceI)
                {
                    const label cellID = faceCells[faceI];
                    TI[cellID] = T.boundaryFieldRef()[patchI][faceI];
                }
            }

            // Convert T field to C++ std vector
            std::vector<scalar> inputC(TI.size());
            forAll(TI, cellI)
            {
                inputC[cellI] = TI[cellI];
            }

            // Convert std vector to Python Numpy array
            const py::array inputPy = py::cast(inputC);

            // Assign inputs to Python scope
            scope["T"] = inputPy;
            scope["gamma"] =
            (
                DT*runTime.deltaT()
               *Foam::pow(max(mesh.deltaCoeffs()), 2)
            ).value();

            // Call python to calculate T
            py::exec("T = calculate(T, gamma)\n", scope);

            // Retrieve output
            const py::array outputPy = scope["T"];

            // Py to C
            const std::vector<scalar> outputC =
                outputPy.cast<std::vector<scalar>>();

            // C to OpenFOAM
            forAll(TI, cellI)
            {
                TI[cellI] = outputC[cellI];
            }
        }

        #include "write.H"

        Info<< "ExecutionTime = " << runTime.elapsedCpuTime() << " s"
            << "  ClockTime = " << runTime.elapsedClockTime() << " s"
            << nl << endl;
    }

    Info<< "End\n" << endl;

    return 0;
}


// ************************************************************************* //
