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

        // Initialise the Python interpreter, if it has not already been
        if (!Py_IsInitialized())
        {
            py::initialize_interpreter();
            scope_ = py::module_::import("__main__").attr("__dict__");

            // Evaluate the Python file to import modules
            py::eval_file(pythonScript_, scope_);
        }
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

    // Take a reference to the face-centre velocity field
    vectorField& velocities = *this;

    // Calculate velocities in Python or directly in OpenFOAM
    if (usePython_)
    {
        const vectorField& C = patch().Cf();

        // Get pointer to the C array
        const Foam::Vector<double>* CData = C.cdata();

        // Get pointer to the velocities array
        Foam::Vector<double>* velocitiesData = velocities.data();

        // Pass the centres array pointer address to Python 
        const long CAddress = reinterpret_cast<long>(CData);
        scope_["CAddress"] = CAddress;

        // Pass the velocities array pointer address to Python 
        const long velocitiesAddress = reinterpret_cast<long>(velocitiesData);
        scope_["velocitiesAddress"] = velocitiesAddress;

        // Pass the size of C field to Python
        scope_["SIZE"] = C.size();

        // Pass the time to Python
        scope_["time"] = db().time().value();

        // Call python script to calculate the face-centre velocities as a function
        // of the face coordinate vectors and the current time
        py::exec("calculate()\n", scope_);

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
