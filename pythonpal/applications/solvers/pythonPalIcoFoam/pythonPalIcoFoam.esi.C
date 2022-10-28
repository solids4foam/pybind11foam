/* License
    This program is part of pythonPal4Foam.

    This program is free software: you can redistribute it and/or modify 
    it under the terms of the GNU General Public License as published 
    by the Free Software Foundation, either version 3 of the License, 
    or (at your option) any later version.

    This program is distributed in the hope that it will be useful, 
    but WITHOUT ANY WARRANTY; without even the implied warranty of 
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 

    See the GNU General Public License for more details. You should have 
    received a copy of the GNU General Public License along with this 
    program. If not, see <https://www.gnu.org/licenses/>. 

   Application
    pythonPalIcoFoam

   Original solver
    icoFoam

   Modified by
    Simon A. Rodriguez, UCD. All rights reserved
    Philip Cardiff, UCD. All rights reserved

   Description
    Transient solver for incompressible, laminar flow of Newtonian fluids.
    The final results are sent to the Python interpreter for the calculation
    of the specific kinetic energy.

\*---------------------------------------------------------------------------*/

#include "fvCFD.H"
#include "pisoControl.H"
#include "pythonPal.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

int main(int argc, char *argv[])
{
    argList::addNote
    (
        "Transient solver for incompressible, laminar flow"
        " of Newtonian fluids."
    );

    #include "addCheckCaseOptions.H"
    #include "setRootCase2.H"
    #include "createTime.H"
    #include "createMesh.H"

    pisoControl piso(mesh);

    #include "createFields.foundation.H"
    #include "initContinuityErrs.H"

    // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //
    pythonPal myPythonPal("python_script.py", true);

    Info<< "\nStarting time loop\n" << endl;

    while (runTime.loop())
    {

        Info<< "Time = " << runTime.timeName() << nl << endl;

        #include "CourantNo.H"

        // Momentum predictor

        fvVectorMatrix UEqn
        (
            fvm::ddt(U)
          + fvm::div(phi, U)
          - fvm::laplacian(nu, U)
        );

        if (piso.momentumPredictor())
        {
            solve(UEqn == -fvc::grad(p));
        }

        // --- PISO loop
        while (piso.correct())
        {
            volScalarField rAU(1.0/UEqn.A());
            volVectorField HbyA(constrainHbyA(rAU*UEqn.H(), U, p));
            surfaceScalarField phiHbyA
            (
                "phiHbyA",
                fvc::flux(HbyA)
              + fvc::interpolate(rAU)*fvc::ddtCorr(U, phi)
            );

            adjustPhi(phiHbyA, U, p);

            // Update the pressure BCs to ensure flux consistency
            constrainPressure(p, U, phiHbyA, rAU);

            // Non-orthogonal pressure corrector loop
            while (piso.correctNonOrthogonal())
            {
                // Pressure corrector

                fvScalarMatrix pEqn
                (
                    fvm::laplacian(rAU, p) == fvc::div(phiHbyA)
                );

                pEqn.setReference(pRefCell, pRefValue);

                pEqn.solve(mesh.solver(p.select(piso.finalInnerIter())));

                if (piso.finalNonOrthogonalIter())
                {
                    phi = phiHbyA - pEqn.flux();
                }
            }

            #include "continuityErrs.H"

            U = HbyA - rAU*fvc::grad(p);
            U.correctBoundaryConditions();
        }

        runTime.write();

        runTime.printExecutionTime(Info);
    }

    Info<< "End\n" << endl;

    //Pass the internal fields to Python side
    myPythonPal.passToPython(U, "U");
    myPythonPal.passToPython(k, "k");

    // Calculate k for the internal field
    myPythonPal.execute("k[:, :] = calculatek(U)");

    // Calculate k for all boundary patches
    forAll(U.boundaryField(), patchI)
    {
        if (U.boundaryField()[patchI].size() != 0)
        {
            // Pass the boundary fields to Python side
            myPythonPal.passToPython(U.boundaryFieldRef()[patchI], "U");
            myPythonPal.passToPython(k.boundaryFieldRef()[patchI], "k");

            // Calculate k for boundary patchI
            myPythonPal.execute("k[:, :] = calculatek(U)");
        }
    }

    k.write();

    return 0;
}


// ************************************************************************* //
