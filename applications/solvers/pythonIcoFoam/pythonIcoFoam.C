#ifdef OPENFOAMFOUNDATION
    #include "pythonIcoFoam.foundation.C"
#elif OPENFOAMESI
    #include "pythonIcoFoam.esi.C"
#else
    #include "pythonIcoFoam.foamextend.C"
#endif
