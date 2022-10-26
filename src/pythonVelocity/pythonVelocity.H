/* License
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

Class
    Foam::pythonVelocity

Description
    This velocity inlet boundary condition is calls a python script to set
    the velocity profile via python using pybind11.

    The address of the python script is passed as an argument.

Usage
    Example of the boundary condition specification:
    \verbatim
    <patchName>
    {
        type            pythonVelocity;
        pythonScript    "$FOAM_CASE/myPythonScript.py";
        value           uniform 0;
    }
    \endverbatim

SourceFiles
    pythonVelocity.C

Author
    Philip Cardiff, UCD.
    Simón A. Rodríguez L., UCD.

\*---------------------------------------------------------------------------*/

#ifndef pythonVelocity_H
#define pythonVelocity_H

#include "fvPatchFields.H"
#include "fixedValueFvPatchFields.H"

// pybind and python headers
#include <pybind11/embed.h>
#include <pybind11/eval.h>
namespace py = pybind11;
using namespace pybind11::literals;

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
           Class pythonVelocity Declaration
\*---------------------------------------------------------------------------*/

class pythonVelocity
:
    public fixedValueFvPatchVectorField
{
    // Private Data

        //- Flag to enable the use of Python
        //  Default to true
        const Switch usePython_;

        //- Python script name
        fileName pythonScript_;

        //- pybind11: Python interpreter's namespace
        py::object scope_;

public:

    //- Runtime type information
    TypeName("pythonVelocity");


    // Constructors

        //- Construct from patch and internal field
        pythonVelocity
        (
            const fvPatch&,
            const DimensionedField<vector, volMesh>&
        );

        //- Construct from patch, internal field and dictionary
        pythonVelocity
        (
            const fvPatch&,
            const DimensionedField<vector, volMesh>&,
            const dictionary&
        );

        //- Construct by mapping given pythonVelocity
        //  onto a new patch
        pythonVelocity
        (
            const pythonVelocity&,
            const fvPatch&,
            const DimensionedField<vector, volMesh>&,
            const fvPatchFieldMapper&
        );

        //- Copy constructor
        pythonVelocity
        (
            const pythonVelocity&
        );

#if OPENFOAMFOUNDATION < 9 || defined(FOAMEXTEND) || defined(OPENFOAMESI)
        //- Construct and return a clone
        virtual tmp<fvPatchVectorField> clone() const
        {
            return tmp<fvPatchVectorField>
            (
                new pythonVelocity(*this)
            );
        }
#endif

        //- Copy constructor setting internal field reference
        pythonVelocity
        (
            const pythonVelocity&,
            const DimensionedField<vector, volMesh>&
        );

        //- Construct and return a clone setting internal field reference
        virtual tmp<fvPatchVectorField> clone
        (
            const DimensionedField<vector, volMesh>& iF
        ) const
        {
            return tmp<fvPatchVectorField>
            (
                new pythonVelocity(*this, iF)
            );
        }


    // Member Functions


        //- Update the coefficients associated with the patch field
        virtual void updateCoeffs();

        //- Write
        virtual void write(Ostream&) const;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
