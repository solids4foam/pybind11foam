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

   Authors
    Simon A. Rodriguez, UCD. All rights reserved
    Philip Cardiff, UCD. All rights reserved

   Description
    Header-only library for running Python codes in OpenFOAM.

\*---------------------------------------------------------------------------*/

#ifndef pythonPal_H
#define pythonPal_H

#include "Field.H"
#include <pybind11/embed.h>
#include <pybind11/eval.h>

namespace py = pybind11;

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{
/*---------------------------------------------------------------------------* \                                                                                                                                                   Class pythonPal Declaration
\*---------------------------------------------------------------------------*/

class pythonPal
{
    // Private data

        //- Address of Python script
        fileName pythonScript_;

        //- Store Python interpretor scope
        py::object scope_;

        //- Debug switch for Info statement
        const bool debug_;


public:

    // Constructors

        // Construct from Python script name
        pythonPal(const fileName& pythonScript, const bool& debug = true)
        :
            pythonScript_(pythonScript),
            scope_(),
            debug_(debug)
        {
            // Initialise the Python interpretor, if it does not already exist
            if (!Py_IsInitialized())
            {
                if (debug_)
                {
                    InfoIn("pythonPal(...)")
                        << "Initialising Python interpretor" << endl; 
                }
                py::initialize_interpreter();
            }

            // Initialise the Python scope
            if (debug_)
            {
                InfoIn("pythonPal(...)")
                    << "Creating Python scope" << endl; 
            }
            scope_ = py::module_::import("__main__").attr("__dict__");

            // Load NumPy
            py::exec("import numpy as np", scope_);

            // Load ctypes
            py::exec("import ctypes as C", scope_);

            // Expand any environment variables in the Python script address
            pythonScript_.expand();

            // Load the Python script
            if (debug_)
            {
                InfoIn("pythonPal(...)")
                    << "Loading the Python script: " << pythonScript_ << endl;
            }
            py::eval_file(pythonScript_, scope_);
        }

    // Destructor

        virtual ~pythonPal()
        {}


    // Member Functions

        //- Pass OpenFOAM List/Field by reference to Python
        template<class T>
        void passToPython
        (
            List<T>& myList, const std::string& fieldNameInPython
        ) const
        {
            if (debug_)
            {
                InfoIn("passToPython(...)")
                    << "Passing field to Python: " << fieldNameInPython << endl;
            }

            // Pass the field size to Python
            scope_["SIZE"] = myList.size();

            // Pass the field name to Python
            scope_["name"] = fieldNameInPython;

            // Retrieve the number of components of T type
            const int dim = pTraits<T>::nComponents;

            // Pass the dimension (number of components) to Python
            scope_["dim"] = dim;

            // Pass the raw data pointer address to Python
            const T* myListData = myList.cdata();
            const long myListAddress = reinterpret_cast<long>(myListData);
            scope_["myList_address"] = myListAddress;

            // Create the NumPy array in Python
            py::exec
            (
                "data_pointer_myList = "
                "C.cast(myList_address, C.POINTER(C.c_double))",
                scope_
            );
            py::exec
            (
                "exec(name + ' = np.ctypeslib.as_array"
                "(data_pointer_myList, shape = (SIZE, dim))')",
                scope_
            );
        }

        //- Execute Python command
        void execute(const word& command) const
        {
            if (debug_)
            {
                InfoIn("execute(const word& command)")
                    << "Executing Python command: " << command << endl;
            }
            py::exec(command, scope_);
        }

};

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
