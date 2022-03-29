// Lookup the python script name
fileName pythonScript
(
    transportProperties.lookup("pythonScript")
);
pythonScript.expand();

// Initialise the python interpreter
py::initialize_interpreter();
py::object scope = py::module_::import("__main__").attr("__dict__");

// Evaluate python file, e.g. to import modules and define functions
py::eval_file(pythonScript, scope);

