from ctypes import *
from Interfaces import *

class CognitionParams(Structure):
    _fields_=[
        ("resolutionType",c_int),
        ("DTHR",c_double),
        ("ZTHR",c_double),
        ("searchType",c_int),
        ("resolutionSpeed",c_double),
        ("allowedXTrackDeviation",c_double),
        ("XtrackGain",c_double)
    ]

ARR3 = c_double*3

class Cognition():
    def __init__(self,callSign):
        self.lib = CDLL('libCognition.so')
        self.lib.CognitionInit.argtypes = [c_char_p]
        self.lib.CognitionInit.restype = c_void_p
        self.lib.Reset.argtypes = [c_void_p]
        self.lib.ResetFlightPhases.argtypes = [c_void_p]
        self.lib.InputVehicleState.argtypes = [c_void_p,c_double*3,c_double*3,c_double]
        self.lib.InputFlightPlanData.argtypes = [c_void_p,c_char_p,c_double,c_int,c_double*3,c_int,c_double]
        self.lib.InputParameters.argtypes = [c_void_p,POINTER(CognitionParams)]
        self.lib.InputDitchStatus.argtypes = [c_void_p,c_double*3,c_bool]
        self.lib.InputMergeStatus.argtypes = [c_void_p,c_int]
        self.lib.InputTrackBands.argtypes = [c_void_p, POINTER(Bands)]
        self.lib.InputSpeedBands.argtypes = [c_void_p, POINTER(Bands)]
        self.lib.InputAltBands.argtypes = [c_void_p, POINTER(Bands)]
        self.lib.InputVSBands.argtypes = [c_void_p, POINTER(Bands)]
        self.lib.InputGeofenceConflictData.argtypes = [c_void_p, POINTER(GeofenceConflict)]
        self.lib.ReachedWaypoint.argtypes = [c_void_p,c_char_p,c_int]
        self.lib.GetCognitionOutput.argtypes = [c_void_p,POINTER(Command)]
        self.lib.GetCognitionOutput.restype = c_int
        self.lib.StartMission.argtypes = [c_void_p,c_int,c_double]
        self.lib.FlightPhases.argtypes = [c_void_p,c_double]
        self.lib.FlightPhases.restype = c_int

        self.obj = self.lib.CognitionInit(c_char_p(callSign.encode('utf-8')))

    def InputVehicleState(self,pos,vel,hdg):
        cpos = ARR3(*pos)
        cvel = ARR3(*vel)
        self.lib.InputVehicleState(self.obj,cpos,cvel,c_double(hdg))

    def InputFlightplanData(self,planID,scenarioTime,flightplan,eta=False):
        for i,fp in enumerate(flightplan):
            wp = fp[:3]
            wpmetric = 1 if eta else 2
            self.lib.InputFlightPlanData(self.obj,c_char_p(planID.encode('utf-8')),
                                         c_double(scenarioTime),c_int(i),ARR3(*wp),c_int(wpmetric),c_double(fp[3]))


    def InputParameters(self,cogParam):
        self.lib.InputParameters(self.obj,byref(cogParam))

    def InputMergeStatus(self,mgStatus):
        self.lib.InputMergeStatus(self.obj,c_int(mgStatus))

    def InputBands(self,trkBand,gsBand,altBand,vsBand):
        self.lib.InputSpeedBands(self.obj,byref(gsBand))
        self.lib.InputTrackBands(self.obj,byref(trkBand))
        self.lib.InputAltBands(self.obj,byref(altBand))
        self.lib.InputVSBands(self.obj,byref(vsBand))

    def InputGeofenceConflictData(self,gfConflictData):
        self.lib.InputGeofenceConflictData(self.obj,byref(gfConflictData))

    def InputReachedWaypoint(self,planID,reachedID):
        self.lib.ReachedWaypoint(self.obj,c_char_p(planID.encode('utf-8')),c_int(reachedID))

    def InputStartMission(self,startWP,delay):
        self.lib.StartMission(self.obj,c_int(startWP),c_double(delay))

    def RunFlightPhases(self,currTime):
        val = self.lib.FlightPhases(self.obj,c_double(currTime))
        return val

    def GetOutput(self):
        cmd = Command()
        numCommands = self.lib.GetCognitionOutput(self.obj,byref(cmd))
        return (numCommands, cmd)

