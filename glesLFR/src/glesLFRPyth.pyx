cdef extern from "glesLFR.cpp":
    void RendererInit()
    void Completerender()

def C_Initiaterender():
    print('InitiateRender')
    RendererInit()

def C_Completerender():
    Completerender()