import numpy as np

def createViewMatrix(up, forward, camLocation):
    right = cross( forward, up )
    up = cross( right, forward )
    rotMatrix = [ right; up; forward ]
    orientation = rotMatrix'
    translVec = -camLocation * rotMatrix
    viewMatrix = [rotMatrix; translVec]' 
    viewMatrix(4,4) = 1

def createJsonPoseFile(LatitudeList, LongitudeList, AltitudeList, ImageList, basefolder, FileName):
    [x,y,utmzone] = deg2utm(Latitude,Longitude)
    EastPositionWthCenter = x-mean(x)
    NorthPositionWthCenter = y-mean(y)
    AltitutdeinMeters = AltitudeList
    up = [-1,0,0]
    forward = [0,0,-1] 
    ii = 0
    for i in range (0,len(LatitudeList)):
        ii = ii + 1
        thermal_images(ii).imagefile = ImageList[i]
        M = createViewMatrix( up, forward, [EastPositionWthCenter[i],NorthPositionWthCenter[i],AltitutdeinMeters[i] )
        thermal_images(ii).M3x4 = M(1:3,:)
    thermal = struct( 'images', thermal_images );
    writeJSON(thermal,fullfile(basefolder,FileName))

LogFile = 'D:\RESILIO\ANAOS\Flight\Test18Sep\Log\GPSLogFile_Examined.log'
Date, Time, DebugIndex, Latitude,Longitude,Altitude,AngleNick,AngleRoll,CompassHeading,DistancetoTarget,TargetHoldTime = np.genfromtxt(LogFile, skip_header=1, unpack=True, delimiter=' ') 
PathsIdentifier = [*range(3, 23, 2)] 
TableLength = len(Latitude)
CurrentPathCount = 1
NoofPoints = 1
LineLatitude = []
LineLongitude = []
LineAltitude = []
LineCompass = []
for i in range (0,TableLength):
    if Latitude[i] == PathsIdentifier[CurrentPathCount] :
        pass
    else :
        LineLatitude.append(Latitude[i])
        LineLongitude.append(Longitude[i])
        LineAltitude.append(Altitude[i])
        LineCompass.append(CompassHeading[i])
        createJsonPoseFile( Latitude, Longitude, Altitude, ImageList, basefolder, FileName)