/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     | Website:  https://openfoam.org
    \\  /    A nd           | Copyright (C) 2011-2019 OpenFOAM Foundation
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

\*---------------------------------------------------------------------------*/

#include "pythonVelocity.H"
#include "addToRunTimeSelectionTable.H"
#include "volFields.H"
#include "surfaceFields.H"

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::pythonVelocity::pythonVelocity
(
    const fvPatch& p,
    const DimensionedField<vector, volMesh>& iF
)
:
    fixedValueFvPatchVectorField(p, iF),
    usePython_(false),
    pythonScript_("undefined"),
    scope_()
{}


Foam::pythonVelocity::pythonVelocity
(
    const pythonVelocity& ptf,
    const fvPatch& p,
    const DimensionedField<vector, volMesh>& iF,
    const fvPatchFieldMapper& mapper
)
:
    fixedValueFvPatchVectorField(ptf, p, iF, mapper),
    usePython_(ptf.usePython_),
    pythonScript_(ptf.pythonScript_),
    scope_()
{}


Foam::pythonVelocity::pythonVelocity
(
    const fvPatch& p,
    const DimensionedField<vector, volMesh>& iF,
    const dictionary& dict
)
:
    fixedValueFvPatchVectorField(p, iF, dict),
    usePython_(dict.lookupOrDefault<Switch>("usePython", Switch(true))),
    pythonScript_
    (
        usePython_
      ? fileName(dict.lookup("pythonScript"))
      : fileName("undefined")
    ),
    scope_()
{
    if (usePython_)
    {
        // Expand any environmental variables e.g. $FOAM_CASE
        pythonScript_.expand();

        // Initialise the Python interpreter
        py::initialize_interpreter();
        scope_ = py::module_::import("__main__").attr("__dict__");

        // Evaluate the Python file to import modules
        py::eval_file(pythonScript_, scope_);
    }
}


#if OPENFOAMFOUNDATION < 9 || defined(FOAMEXTEND) || defined(OPENFOAMESI)
Foam::pythonVelocity::pythonVelocity
(
    const pythonVelocity& pivpvf
)
:
    fixedValueFvPatchVectorField(pivpvf),
    usePython_(pivpvf.usePython_),
    pythonScript_(pivpvf.pythonScript_),
    scope_()
{}
#endif


Foam::pythonVelocity::pythonVelocity
(
    const pythonVelocity& pivpvf,
    const DimensionedField<vector, volMesh>& iF
)
:
    fixedValueFvPatchVectorField(pivpvf, iF),
    usePython_(pivpvf.usePython_),
    pythonScript_(pivpvf.pythonScript_),
    scope_()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void Foam::pythonVelocity::updateCoeffs()
{
    if (updated())
    {
        return;
    }

    // Face-centre velocity field
    vectorField velocities(patch().size(), vector::zero);

    // Calculate velocities in Python or directly in OpenFOAM
    if (usePython_)
    {
        // Convert the face-centre position vectors to a std::vector
        const vectorField& C = patch().Cf();
        std::vector<std::vector<scalar>> inputC(C.size());
        forAll(C, faceI)
        {
            inputC[faceI] = std::vector<scalar>(3);
            for (int compI = 0; compI < 3; compI++)
            {
                inputC[faceI][compI] = C[faceI][compI];
            }
        }

        // Convert std vector to a C++ NumPy array
        const py::array inputPy = py::cast(inputC);

        // Transfer the C++ NumPy array to a NumPy array in the Python scope
        scope_["face_centres"] = inputPy;
        scope_["time"] = db().time().value();


        // Call python script to calculate the face-centre velocities as a function
        // of the face coordinate vectors and the current time
        py::exec("velocities = calculate(face_centres, time)\n", scope_);


        // Transfer the new velocities back to OpenFOAM

        // Convert the Python velocities to a C++ NumPy array
        const py::array outputPy = scope_["velocities"];

        // Convert the C++ NumPy array to a C++ std vector
        const std::vector<std::vector<scalar>> outputC =
            outputPy.cast<std::vector<std::vector<scalar>>>();

        // Convert the C++ std vector to an OpenFOAM field
        forAll(velocities, faceI)
        {
            for (int compI = 0; compI < 3; compI++)
            {
                velocities[faceI][compI] = outputC[faceI][compI];
            }
        }
    }
    else
    {
        // Perform calculations directly in OpenFOAM

        // X component of the face-centre coordinates
        const scalarField x(patch().Cf().component(vector::X));

        // Time
        const scalar t = db().time().value();

        // Pi
        #ifdef FOAMEXTEND
            const scalar pi = mathematicalConstant::pi;
        #else
            const scalar pi = constant::mathematical::pi;
        #endif

        // Calculate velocity
        velocities.replace(vector::X, Foam::sin(t*pi)*Foam::sin(x*40*pi));
    }

    // Set values on the patch
    fvPatchField<vector>::operator==(velocities);

    fixedValueFvPatchVectorField::updateCoeffs();
}


void Foam::pythonVelocity::write(Ostream& os) const
{
    fvPatchVectorField::write(os);
    os.writeKeyword("usePython")
        << usePython_ << token::END_STATEMENT << nl;
    os.writeKeyword("pythonScript")
        << pythonScript_ << token::END_STATEMENT << nl;

#ifdef OPENFOAMFOUNDATION
    writeEntry(os, "value", *this);
#else
    writeEntry("value", os);
#endif
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{
    makePatchTypeField
    (
        fvPatchVectorField,
        pythonVelocity
    );
}

// ************************************************************************* //
