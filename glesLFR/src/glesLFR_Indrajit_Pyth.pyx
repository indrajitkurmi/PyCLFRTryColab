from libcpp.vector cimport vector
from libcpp.string cimport string
import numpy as np
cimport numpy as np
import json
import os
import cv2

'''
cdef extern from "glesLFR_Indrajit.cpp":
    void RendererInit()
    void Completerender()
    void TerminateRenderer()
    void LoadDemModel(string ModelFile, string Modelimage)
    void PrintPyLightFieldInstanceInfo(Lightfield *PyLF)

def Py_Initiaterender():
    print('InitiateRender')
    RendererInit()

def Py_Completerender():
    Completerender()

def Py_TerminateRenderer():
    TerminateRenderer()

def Py_LoadDemModel(string OBJModelPath,string OBJModelImagePath):
    LoadDemModel(OBJModelPath.encode(),OBJModelImagePath.encode())

def Py_PrintPyLightFieldInstanceInfo(PyLightfieldClass PyLF):
    PrintPyLightFieldInstanceInfo(PyLF.thisptr)
'''
cdef extern from "glesLFR_Indrajit.cpp": # defines the source C++ file
    cdef cppclass Lightfield:
        vector[string] pynames
        vector[vector[double]] vectorposes
        unsigned char *image_to_bind
        float camerafocallength;
        vector[unsigned int] ogl_textures

        Lightfield(int preallocate) except +
        void allocate(int allocate)
        void GenrateTextureID();
        void BindImageWithTextureID(unsigned int index,unsigned int width, unsigned int height, unsigned int nrComponents)
        void RendererInitializationPart1()
        void RendererInitializationPart2()
        void RenderOnce(float *RenderedImage)
        void LoadDemModel(string ObjFile, unsigned int width, unsigned int height, unsigned int nrComponents)
        void LFRTerminateRenderer()
        void SetProjectionMatrix()

cdef class PyLightfieldClass: # defines a python wrapper to the C++ class
    cdef Lightfield* thisptr # thisptr is a pointer that will hold to the instance of the C++ class
    cdef unsigned char *PyCheckArray
    def __cinit__(self, int preallocatedvalue): # defines the python wrapper class' init function
        self.thisptr = new Lightfield(preallocatedvalue) # creates an instance of the C++ class and puts allocates the pointer to this
    def __dealloc__(self): # defines the python wrapper class' deallocation function (python destructor)
        del self.thisptr # destroys the reference to the C++ instance (which calls the C++ class destructor
    def Py_allocate(self, int Py_NameSize):
        return self.thisptr.allocate(Py_NameSize)
    def Py_setCameraFocalLength(self, float camerafocallength):
        self.thisptr.camerafocallength = camerafocallength
    def Py_setProjectionMatrix(self):
        self.thisptr.SetProjectionMatrix()
    def namespushback(self, namestring):
        self.thisptr.pynames.push_back(namestring.encode())
    def posespushback(self, np.ndarray[double, ndim=1, mode="c"] InputVector not None):
        cdef vector[double] InputVectorC = InputVector
        self.thisptr.vectorposes.push_back(InputVectorC)
    def namesIndex(self, index, namestring):
        self.thisptr.pynames[index] = namestring.encode()
    def posesIndex(self, index, np.ndarray[double, ndim=1, mode="c"] InputVector not None):
        cdef vector[double] InputVectorC = InputVector
        self.thisptr.vectorposes[index] = InputVectorC
    def LoadImageToC_8bit_1ch(self,np.ndarray[unsigned char, ndim=2, mode="c"] InputVector not None):
        cdef unsigned char[:,::1] InputVectorC = InputVector
        self.PyCheckArray = &InputVectorC[0,0]
        self.thisptr.image_to_bind = self.PyCheckArray
    def LoadImageToC_8bit_3ch(self,np.ndarray[unsigned char, ndim=3, mode="c"] InputVector not None):
        cdef unsigned char[:,:,::1] InputVectorC = InputVector
        self.PyCheckArray = &InputVectorC[0,0,0]
        self.thisptr.image_to_bind = self.PyCheckArray
    def LoadImageToC_16bit_1ch(self,np.ndarray[unsigned char, ndim=3, mode="c"] InputVector not None):
        cdef unsigned char[:,:,::1] InputVectorC = InputVector
        self.PyCheckArray = &InputVectorC[0,0,0]
        self.thisptr.image_to_bind = self.PyCheckArray
    def LoadImageToC_16bit_3ch(self,np.ndarray[unsigned char, ndim=3, mode="c"] InputVector not None):
        cdef unsigned char[:,:,::1] InputVectorC = InputVector
        self.PyCheckArray = &InputVectorC[0,0,0]
        self.thisptr.image_to_bind = self.PyCheckArray
    def RenderInitializeP1(self):
        self.thisptr.RendererInitializationPart1()
    def Py_GenrateTextureID(self):
        self.thisptr.GenrateTextureID()
    def Py_BindImageWithTextureID(self,index, width, height, nrComponents):
        self.thisptr.BindImageWithTextureID(index, width, height, nrComponents)
    def RenderInitializeP2(self):
        self.thisptr.RendererInitializationPart2()
    def Py_LoadDemModel(self,ObjModelPath, OBJModelImageName):
        DemImage = cv2.imread(OBJModelImageName,0)
        self.LoadImageToC_8bit_1ch(DemImage)
        # height, width, number of channels in image
        height = DemImage.shape[0]
        width = DemImage.shape[1]
        #print(len(DemImage.shape))
        if len(DemImage.shape) == 2 :
            nrComponents = 1
        else :
            nrComponents = DemImage.shape[2]          
        self.thisptr.LoadDemModel(ObjModelPath.encode(),width, height, nrComponents)
    def GetnamesIndex(self, index):
        namestring = self.thisptr.pynames[index].decode()
        return namestring
    def RenderImageOnce(self, ImageName):
        print('Rendering Scene')
        cdef np.ndarray[float, ndim=3, mode='c'] a
        a = np.zeros((512,512,4), dtype=np.float32)
        self.thisptr.RenderOnce(&a[0,0,0])
        print('Rendering Finished')
        cv2.imwrite(ImageName, a)
        print('Image Written')
        return a
    def TerminateRendererOnceFinished(self):
        self.thisptr.LFRTerminateRenderer()




    
        

