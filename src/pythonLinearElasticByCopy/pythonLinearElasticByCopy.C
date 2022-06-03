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

\*---------------------------------------------------------------------------*/

#include "pythonLinearElasticByCopy.H"
#include "addToRunTimeSelectionTable.H"
#include "zeroGradientFvPatchFields.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
    defineTypeNameAndDebug(pythonLinearElasticByCopy, 0);
    addToRunTimeSelectionTable
    (
        mechanicalLaw, pythonLinearElasticByCopy, linGeomMechLaw
    );
}


// * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * * * //

void Foam::pythonLinearElasticByCopy::updateStrain()
{
    if (incremental())
    {
        // Lookup gradient of displacement increment
        const volTensorField& gradDD =
            mesh().lookupObject<volTensorField>("grad(DD)");

        // Calculate the total strain
        epsilon_ = epsilon_.oldTime() + symm(gradDD);
    }
    else
    {
        // Lookup gradient of displacement
        const volTensorField& gradD =
            mesh().lookupObject<volTensorField>("grad(D)");

        // Calculate the total strain
        epsilon_ = symm(gradD);
    }
}

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

// Construct from dictionary
Foam::pythonLinearElasticByCopy::pythonLinearElasticByCopy
(
    const word& name,
    const fvMesh& mesh,
    const dictionary& dict,
    const nonLinearGeometry::nonLinearType& nonLinGeom
)
:
    mechanicalLaw(name, mesh, dict, nonLinGeom),
    scope_(),
    rho_(dict.lookup("rho")),
    impK_(dict.lookup("implicitStiffness")),
    epsilon_
    (
        IOobject
        (
            "epsilon",
            mesh.time().timeName(),
            mesh,
            IOobject::NO_READ,
            IOobject::NO_WRITE
        ),
        mesh,
        dimensionedSymmTensor("zero", dimless, symmTensor::zero)
    )
{
    // Create python interpreter
    py::initialize_interpreter();
    scope_ = py::module_::import("__main__").attr("__dict__");

    // Load the python file and evaluate it
    const word pythonMod =
        dict.lookupOrDefault<word>("pythonModule", "python_code.py");
    py::eval_file(pythonMod, scope_);

    // Cell-by-cell or entire field?
    sendToPython = word(dict.lookup("sendToPython"));

    // Check impK is positive
    if (impK_.value() < SMALL)
    {
        FatalErrorIn
        (
            "Foam::pythonLinearElasticByCopy::pythonLinearElasticByCopy\n"
            "(\n"
            "    const word& name,\n"
            "    const fvMesh& mesh,\n"
            "    const dictionary& dict\n"
            ")"
        )   << "The implicitStiffness should be positive!"
            << abort(FatalError);
    }

    // Store the old time
    epsilon_.oldTime();
}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

Foam::pythonLinearElasticByCopy::~pythonLinearElasticByCopy()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

Foam::tmp<Foam::volScalarField> Foam::pythonLinearElasticByCopy::rho() const
{
    tmp<volScalarField> tresult
    (
        new volScalarField
        (
            IOobject
            (
                "rho",
                mesh().time().timeName(),
                mesh(),
                IOobject::NO_READ,
                IOobject::NO_WRITE
            ),
            mesh(),
            rho_,
            zeroGradientFvPatchScalarField::typeName
        )
    );

#ifdef OPENFOAMESIORFOUNDATION
    tresult.ref().correctBoundaryConditions();
#else
    tresult().correctBoundaryConditions();
#endif

    return tresult;
}


Foam::tmp<Foam::volScalarField> Foam::pythonLinearElasticByCopy::impK() const
{
    return tmp<volScalarField>
    (
        new volScalarField
        (
            IOobject
            (
                "impK",
                mesh().time().timeName(),
                mesh(),
                IOobject::NO_READ,
                IOobject::NO_WRITE
            ),
            mesh(),
            impK_
        )
    );
}


Foam::tmp<Foam::volScalarField> Foam::pythonLinearElasticByCopy::K() const
{
    notImplemented("Foam::pythonLinearElasticByCopy::K()");

    // Keep the compiler happy
    return impK();
}


void Foam::pythonLinearElasticByCopy::calculateStress
(
    symmTensorField& sigma,
    const symmTensorField& epsilon
)
{
    if (sendToPython.match("entireField"))
    {
        if (epsilon.size() != 0)
        {
            // Transfer strain to Python
            std::vector<std::vector<scalar>> inputC(epsilon.size());
            forAll(epsilon, cellI)
            { 
                const symmTensor& in = epsilon[cellI];
                inputC[cellI] =
                {
                    in.xx(), in.yy(), in.zz(),
                    in.xy(), in.yz(), in.xz()
                };
            }
            const py::array inputPy = py::cast(inputC);
            scope_["strains"] = inputPy;

            // Calculate stress via Python
            py::exec("stress = predict(strains)\n");

            // Transfer stress back to OpenFOAM
            const py::array& outputPy = scope_["stress"];
            const std::vector<std::vector<scalar>>& outputC =
                outputPy.cast<std::vector<std::vector<scalar>>>();
            forAll(sigma, cellI)
            {
                sigma[cellI].xx() = outputC[cellI][0];
                sigma[cellI].yy() = outputC[cellI][1];
                sigma[cellI].zz() = outputC[cellI][2];
                sigma[cellI].xy() = outputC[cellI][3];
                sigma[cellI].yz() = outputC[cellI][4];
                sigma[cellI].xz() = outputC[cellI][5];
            }
        }
    }
    else
    {
        // Loop over all cells
        forAll(sigma, cellI)
        {
            // Transfer strain to Python
            const symmTensor& in = epsilon[cellI];
            const std::vector<std::vector<scalar>> inputC =
            {
                {
                    in.xx(), in.yy(), in.zz(),
                    in.xy(), in.yz(), in.xz()
                }
            };
            const py::array inputPy = py::cast(inputC);
            scope_["strains"] = inputPy;

            // Calculate stress via Python
            py::exec("stress = predict(strains)\n", scope_);

            // Transfer stress back to OpenFOAM
            const py::array outputPy = scope_["stress"];
            const std::vector<std::vector<scalar>> outputC =
                outputPy.cast<std::vector<std::vector<scalar>>>();
            sigma[cellI].xx() = outputC[0][0];
            sigma[cellI].yy() = outputC[0][1];
            sigma[cellI].zz() = outputC[0][2];
            sigma[cellI].xy() = outputC[0][3];
            sigma[cellI].yz() = outputC[0][4];
            sigma[cellI].xz() = outputC[0][5];
        }
    }
}


void Foam::pythonLinearElasticByCopy::correct(volSymmTensorField& sigma)
{
    // Update strain volSymmTensorField (epsilon)
    updateStrain();

    // Take references for brevity and efficiency
    #ifdef FOAMEXTEND
        symmTensorField& sigmaI = sigma.internalField();
    #else
        symmTensorField& sigmaI = sigma.primitiveFieldRef();
    #endif
    const symmTensorField& epsilonI = epsilon_.internalField();

    // Calculate stress in the internal field
    calculateStress(sigmaI, epsilonI);

    // Loop over all boundary patches
    forAll(sigma.boundaryField(), patchI)
    {
        // Take references for brevity and efficiency
        #ifdef FOAMEXTEND
            symmTensorField& sigmaP = sigma.boundaryField()[patchI];
        #else
            symmTensorField& sigmaP = sigma.boundaryFieldRef()[patchI];
        #endif
        const symmTensorField&  epsilonP = epsilon_.boundaryField()[patchI];

        // Calculate stress on the boundary patch
        calculateStress(sigmaP, epsilonP);
    }
}


void Foam::pythonLinearElasticByCopy::correct(surfaceSymmTensorField& sigma)
{
    notImplemented
    (
        "void Foam::pythonLinearElasticByCopy::correct(surfaceSymmTensorField&)"
    );
}


Foam::scalar Foam::pythonLinearElasticByCopy::residual()
{
    // For nowlinear laws, we can calculate this in some way so that the solid
    // model knows if the law has converged
    return 0.0;
}


// ************************************************************************* //
