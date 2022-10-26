/* License
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

#include "pythonLinearElastic.H"
#include "addToRunTimeSelectionTable.H"
#include "zeroGradientFvPatchFields.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
    defineTypeNameAndDebug(pythonLinearElastic, 0);
    addToRunTimeSelectionTable
    (
        mechanicalLaw, pythonLinearElastic, linGeomMechLaw
    );
}


// * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * * * //

void Foam::pythonLinearElastic::updateStrain()
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
Foam::pythonLinearElastic::pythonLinearElastic
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

    // Check impK is positive
    if (impK_.value() < SMALL)
    {
        FatalErrorIn
        (
            "Foam::pythonLinearElastic::pythonLinearElastic\n"
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

Foam::pythonLinearElastic::~pythonLinearElastic()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

Foam::tmp<Foam::volScalarField> Foam::pythonLinearElastic::rho() const
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


Foam::tmp<Foam::volScalarField> Foam::pythonLinearElastic::impK() const
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

Foam::tmp<Foam::volScalarField> Foam::pythonLinearElastic::K() const
{
    notImplemented("Foam::pythonLinearElastic::K()");

    // Keep the compiler happy
    return impK();
}


void Foam::pythonLinearElastic::calculateStress
(
    symmTensorField& sigma,
    const symmTensorField& epsilon
)
{
    if (sigma.size() != 0)
    {
        // Get pointers to the stress and strain arrays
        symmTensor* sigmaData = sigma.data();
        const symmTensor* epsilonData = epsilon.cdata();

        // Pass the size of the stress and strain fields to Python
        scope_["SIZE"] = sigma.size();

        // Pass the array pointer address to Python 
        const long epsilonAddress = reinterpret_cast<long>(epsilonData);
        const long sigmaAddress = reinterpret_cast<long>(sigmaData);
        scope_["epsilon_address"] = epsilonAddress;
        scope_["sigma_address"] = sigmaAddress;

        // Call the Python predict function to calculate the stress field
        py::exec("predict()", scope_);
    }
}

void Foam::pythonLinearElastic::correct(volSymmTensorField& sigma)
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


void Foam::pythonLinearElastic::correct(surfaceSymmTensorField& sigma)
{
    notImplemented
    (
        "void Foam::pythonLinearElastic::correct(surfaceSymmTensorField&)"
    );
}


Foam::scalar Foam::pythonLinearElastic::residual()
{
    // For nowlinear laws, we can calculate this in some way so that the solid
    // model knows if the law has converged
    return 0.0;
}


// ************************************************************************* //
