/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | foam-extend: Open Source CFD
   \\    /   O peration     |
    \\  /    A nd           | For copyright notice see file Copyright
     \\/     M anipulation  |
-------------------------------------------------------------------------------
License
    This file is part of foam-extend.

    foam-extend is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation, either version 3 of the License, or (at your
    option) any later version.

    foam-extend is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with foam-extend.  If not, see <http://www.gnu.org/licenses/>.

Class
    pythonLinearElastic

Description
    Mechanical law that uses pybind11 to call a trained Keras (TensorFlow)
    neural network mechanical law.

SourceFiles
    pythonLinearElastic.C

Author
    Simon A. Rodriguez, UCD. All rights reserved
    Philip Cardiff, UCD. All rights reserved

\*---------------------------------------------------------------------------*/

#ifndef pythonLinearElastic_H
#define pythonLinearElastic_H

#include "mechanicalLaw.H"
#include "surfaceFields.H"

// Pybind11 headers
#include <pybind11/embed.h>
#include <pybind11/numpy.h>
#include <vector>
#include <pybind11/stl.h>
#include <pybind11/eval.h>
namespace py = pybind11;

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                         Class pythonLinearElastic Declaration
\*---------------------------------------------------------------------------*/

class pythonLinearElastic
:
    public mechanicalLaw
{
    // Private data

        //- Python interpreter's namespace
        py::object scope_;

        //- Density
        dimensionedScalar rho_;

        //- Implicit stiffness used by the solid model
        //  Assuming convergence is achieved, this will not affect the final answer
        //  Setting it to the equivalent of 2*mu + lambda in elasticity is optimal
        dimensionedScalar impK_;

        //- Total strain field
        volSymmTensorField epsilon_;

    // Private Member Functions

        //- Disallow default bitwise copy construct
        pythonLinearElastic(const pythonLinearElastic&);

        //- Disallow default bitwise assignment
        void operator=(const pythonLinearElastic&);

        //- Update the strain field
        void updateStrain();

        //- Make prediction of stress given the strain
        //  The stress is stored in the python scope
        void predict(const py::array& epsilon);

        void calculateStress
        (
            symmTensorField& sigmaI,
            const symmTensorField& epsilonI
        );

public:

    //- Runtime type information
    TypeName("pythonLinearElastic");

    // Constructors

        //- Construct from dictionary
        pythonLinearElastic
        (
            const word& name,
            const fvMesh& mesh,
            const dictionary& dict,
            const nonLinearGeometry::nonLinearType& nonLinGeom
        );


    // Destructor

        virtual ~pythonLinearElastic();


    // Member Functions

        //- Return density
        virtual tmp<volScalarField> rho() const;

        //- Return the implicit stiffness
        //  This is the diffusivity for the Laplacian term
        virtual tmp<volScalarField> impK() const;

        //- Return the bulk modulus
        virtual tmp<volScalarField> K() const;

        //- Calculate the stress
        virtual void correct(volSymmTensorField& sigma);

        //- Calculate the stress
        virtual void correct(surfaceSymmTensorField& sigma);

        //- Return material residual i.e. a measured of how convergence of
        //  the material model
        virtual scalar residual();
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
