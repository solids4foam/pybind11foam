#ifdef OPENFOAMFOUNDATION
    #include "pythonPalIcoFoam.foundation.C"
#elif OPENFOAMESI
    #include "pythonPalIcoFoam.esi.C"
#else
    #include "pythonPalIcoFoam.foamextend.C"
#endif
