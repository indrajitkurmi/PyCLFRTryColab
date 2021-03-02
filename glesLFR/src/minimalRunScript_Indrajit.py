print('ProgramStarted')
import glesLFR_Indrajit
import json
import numpy as np
import os
import cv2
print('glesLFR Import')

def ReadJsonPosesFiles(PyClassObject,PosesFilePath):
        with open(PosesFilePath) as PoseFile:
            PoseFileData = json.load(PoseFile)
            NoofPoses = len(PoseFileData['images'])
            PoseFileImagesData = PoseFileData['images']
            if len(PoseFileData['images']) > 0:
                PyClassObject.Py_allocate(len(PoseFileData['images']))
                for i in range (0,len(PoseFileData['images'])):
                    ImageName = PoseFileImagesData[i]['imagefile']
                    #Convert List of List to Np array
                    PoseMatrix = PoseFileImagesData[i]['M3x4']
                    PoseMatrixNumpyArray = np.array([], dtype=np.double)
                    for k in range(0,len(PoseMatrix)):
                        PoseMatrixNumpyArray = np.append(PoseMatrixNumpyArray, np.asarray(PoseMatrix[k],dtype=np.double))
                    PyClassObject.namespushback(ImageName)
                    PyClassObject.posespushback(np.array(PoseMatrixNumpyArray))
        return NoofPoses
def RenderInitialize(PyClassObject,PosesFilePath,ImageLocation,ObjModelPath,ObjModelImagePath):
        PyClassObject.RenderInitializeP1()
        NofPoses = ReadJsonPosesFiles(PyClassObject, PosesFilePath)
        print(NofPoses)
        PyClassObject.Py_LoadDemModel(ObjModelPath,ObjModelImagePath)
        for i in range (0,NofPoses):
            ImageName = PyClassObject.GetnamesIndex(i)
            print(ImageName)
            LoadImageName = ImageName.replace('.tiff','.png')
            Image = cv2.imread(os.path.join(ImageLocation,LoadImageName),0)
             # height, width, number of channels in image
            height = Image.shape[0]
            width = Image.shape[1]
            if len(Image.shape) == 2 :
                nrComponents = 1
            else :
                nrComponents = Image.shape[2]
            PyClassObject.LoadImageToC_8bit_1ch(Image)
            PyClassObject.Py_GenrateTextureID()
            PyClassObject.Py_BindImageWithTextureID(i,height,width,nrComponents)
        print('Binding Texture Successful')
        PyClassObject.RenderInitializeP2()
        print('Inititalization Finished')
PosesFilePath = '../data/T20200207F2/thermal_GPS_Corr.json'
ImageLocation = '../data/T20200207F2/thermal_ldr512'
ObjModelPath = '../data/T20200207F2/dem.obj'
ObjModelImagePath = '../data/T20200207F2/dem.png'
FocalLength = 50.815436217896945
PyLFClass = glesLFR_Indrajit.PyLightfieldClass(0)
PyLFClass.Py_setCameraFocalLength(FocalLength)
PyLFClass.Py_setProjectionMatrix()
RenderInitialize(PyLFClass,PosesFilePath,ImageLocation,ObjModelPath,ObjModelImagePath)
ImageReturned1 = PyLFClass.RenderImageOnce('RenderedImage1.png')
cv2.imwrite('Image1InMainApp.png', ImageReturned1)
ImageReturned2 =  PyLFClass.RenderImageOnce('RenderedImage2.png')
cv2.imwrite('Image2InMainApp.png', ImageReturned2)
PyLFClass.TerminateRendererOnceFinished()
#ReadJsonPosesFiles('../data/T20200207F2/thermal_GPS_Corr.json')
print('PY LightFieldClass generated')

#PyLFClass = glesLFR_Indrajit.PyLightfieldClass(0)
#glesLFR_Indrajit.Py_PrintPyLightFieldInstanceInfo(PyLFClass)
#glesLFR_Indrajit.Py_Initiaterender()
#PyLFClass.RenderImageOnce()
#glesLFR_Indrajit.Py_Completerender()

#glesLFR_Indrajit.Py_Initiaterender()
del(PyLFClass)
