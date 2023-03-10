
from paraview.simple import *
from paraview import coprocessing

import os


#--------------------------------------------------------------
# Code generated from cpstate.py to create the CoProcessor.
# ParaView 5.3.0 64 bits


# ----------------------- CoProcessor definition -----------------------

def CreateCoProcessor():
  def _CreatePipeline(coprocessor, datadescription):
    class Pipeline:
      # state file generated using paraview version 5.3.0

      # ----------------------------------------------------------------
      # setup the data processing pipelines
      # ----------------------------------------------------------------

      #### disable automatic camera reset on 'Show'
      paraview.simple._DisableFirstRenderCameraReset()

      # create a new 'Image Reader'
      # create a producer from a simulation input
      u_20151101_170000raw = coprocessor.CreateProducer(datadescription, 'U')

      # create a new 'Image Reader'
      # create a producer from a simulation input
      qICE_20151101_170000raw = coprocessor.CreateProducer(datadescription, 'QICE')
      qGRAUP_20151101_170000raw = coprocessor.CreateProducer(datadescription, 'QGRAUP')
      qRAIN_20151101_170000raw = coprocessor.CreateProducer(datadescription, 'QRAIN')

      # create a new 'Image Reader'
      # create a producer from a simulation input
      p_20151101_170000raw = coprocessor.CreateProducer(datadescription, 'P')

      # create a new 'Programmable Filter'
      extractCycloneCenter = ProgrammableFilter(Input=p_20151101_170000raw)
      extractCycloneCenter.OutputDataSetType = 'vtkPolyData'
      extractCycloneCenter.Script = "import math\ndi = self.GetInput()\nipd = di.GetPointData()\npdi = ipd.GetScalars('P')\npdo = self.GetPolyDataOutput()\nnewPts = vtk.vtkPoints()\n(dimX, dimY, dimZ) = di.GetDimensions()\nminP = pdi.GetTuple1(0)\nminX = 0\nminY = 0\nk = 0\nfor z in range(0, dimZ):\n  for y in range(0, dimY):\n    for x in range(0, dimX):\n      pk = pdi.GetTuple1(k)\n      if (pk < minP):\n        minP = pk\n        minX = x\n        minY = y\n      k = k + 1\nnewPts.InsertPoint(0, minX, minY, 0)\npdo.SetPoints(newPts)"
      extractCycloneCenter.RequestInformationScript = ''
      extractCycloneCenter.RequestUpdateExtentScript = ''
      extractCycloneCenter.PythonPath = ''

      # create a new 'Programmable Filter'
      extractQICE = ProgrammableFilter(Input=[qICE_20151101_170000raw, extractCycloneCenter])
      extractQICE.OutputDataSetType = 'vtkImageData'
      extractQICE.Script = "import vtk\ndi = self.GetInputDataObject(0,0)\nipd = di.GetPointData()\nci = self.GetInputDataObject(0,1)\nido = self.GetImageDataOutput()\ndiExt = di.GetExtent()\noffs = 192\nctrs = ci.GetPoints().GetPoint(0)\nctrX = int(ctrs[0])\nctrY = int(ctrs[1])\noext = [ int(max([int(ctrX-offs),int(diExt[0])])), int(min([int(ctrX+offs),int(diExt[1])]))\n       , int(max([int(ctrY-offs),int(diExt[2])])), int(min([int(ctrY+offs),int(diExt[3])]))\n       , int(diExt[4]), int(diExt[5])]\nvoi = vtk.vtkExtractVOI()\nvoi.SetVOI(oext[0], oext[1], oext[2], oext[3], oext[4], oext[5])\nvoi.SetInputData(di)\nvoi.Update()\nido.ShallowCopy(voi.GetOutput())"
      extractQICE.RequestInformationScript = 'from paraview import util\n\nutil.SetOutputWholeExtent(self, self.GetOutput().GetExtent())\n'
      extractQICE.RequestUpdateExtentScript = ''
      extractQICE.PythonPath = ''

      # create a new 'Programmable Filter'
      extractQGRAUP = ProgrammableFilter(Input=[qGRAUP_20151101_170000raw, extractCycloneCenter])
      extractQGRAUP.OutputDataSetType = 'vtkImageData'
      extractQGRAUP.Script = "import vtk\ndi = self.GetInputDataObject(0,0)\nipd = di.GetPointData()\nci = self.GetInputDataObject(0,1)\nido = self.GetImageDataOutput()\ndiExt = di.GetExtent()\noffs = 192\nctrs = ci.GetPoints().GetPoint(0)\nctrX = int(ctrs[0])\nctrY = int(ctrs[1])\noext = [ int(max([int(ctrX-offs),int(diExt[0])])), int(min([int(ctrX+offs),int(diExt[1])]))\n       , int(max([int(ctrY-offs),int(diExt[2])])), int(min([int(ctrY+offs),int(diExt[3])]))\n       , int(diExt[4]), int(diExt[5])]\nvoi = vtk.vtkExtractVOI()\nvoi.SetVOI(oext[0], oext[1], oext[2], oext[3], oext[4], oext[5])\nvoi.SetInputData(di)\nvoi.Update()\nido.ShallowCopy(voi.GetOutput())"
      extractQGRAUP.RequestInformationScript = 'from paraview import util\n\nutil.SetOutputWholeExtent(self, self.GetOutput().GetExtent())\n'
      extractQGRAUP.RequestUpdateExtentScript = ''
      extractQGRAUP.PythonPath = ''

      # create a new 'Programmable Filter'
      extractQRAIN = ProgrammableFilter(Input=[qRAIN_20151101_170000raw, extractCycloneCenter])
      extractQRAIN.OutputDataSetType = 'vtkImageData'
      extractQRAIN.Script = "import vtk\ndi = self.GetInputDataObject(0,0)\nipd = di.GetPointData()\nci = self.GetInputDataObject(0,1)\nido = self.GetImageDataOutput()\ndiExt = di.GetExtent()\noffs = 192\nctrs = ci.GetPoints().GetPoint(0)\nctrX = int(ctrs[0])\nctrY = int(ctrs[1])\noext = [ int(max([int(ctrX-offs),int(diExt[0])])), int(min([int(ctrX+offs),int(diExt[1])]))\n       , int(max([int(ctrY-offs),int(diExt[2])])), int(min([int(ctrY+offs),int(diExt[3])]))\n       , int(diExt[4]), int(diExt[5])]\nvoi = vtk.vtkExtractVOI()\nvoi.SetVOI(oext[0], oext[1], oext[2], oext[3], oext[4], oext[5])\nvoi.SetInputData(di)\nvoi.Update()\nido.ShallowCopy(voi.GetOutput())"
      extractQRAIN.RequestInformationScript = 'from paraview import util\n\nutil.SetOutputWholeExtent(self, self.GetOutput().GetExtent())\n'
      extractQRAIN.RequestUpdateExtentScript = ''
      extractQRAIN.PythonPath = ''

      # create a new 'Image Reader'
      # create a producer from a simulation input
      w_20151101_170000raw = coprocessor.CreateProducer(datadescription, 'W')

      # create a new 'Programmable Filter'
      extractW = ProgrammableFilter(Input=[w_20151101_170000raw, extractCycloneCenter])
      extractW.OutputDataSetType = 'vtkImageData'
      extractW.Script = "import vtk\ndi = self.GetInputDataObject(0,0)\nipd = di.GetPointData()\nci = self.GetInputDataObject(0,1)\nido = self.GetImageDataOutput()\ndiExt = di.GetExtent()\noffs = 192\nctrs = ci.GetPoints().GetPoint(0)\nctrX = int(ctrs[0])\nctrY = int(ctrs[1])\noext = [ int(max([int(ctrX-offs),int(diExt[0])])), int(min([int(ctrX+offs),int(diExt[1])]))\n       , int(max([int(ctrY-offs),int(diExt[2])])), int(min([int(ctrY+offs),int(diExt[3])]))\n       , int(diExt[4]), int(diExt[5])]\nvoi = vtk.vtkExtractVOI()\nvoi.SetVOI(oext[0], oext[1], oext[2], oext[3], oext[4], oext[5])\nvoi.SetInputData(di)\nvoi.Update()\nido.ShallowCopy(voi.GetOutput())"
      extractW.RequestInformationScript = 'from paraview import util\n\nutil.SetOutputWholeExtent(self, self.GetOutput().GetExtent())\n'
      extractW.RequestUpdateExtentScript = ''
      extractW.PythonPath = ''

      # create a new 'Image Reader'
      # create a producer from a simulation input
      v_20151101_170000raw = coprocessor.CreateProducer(datadescription, 'V')

      # create a new 'Programmable Filter'
      extractV = ProgrammableFilter(Input=[v_20151101_170000raw, extractCycloneCenter])
      extractV.OutputDataSetType = 'vtkImageData'
      extractV.Script = "import vtk\ndi = self.GetInputDataObject(0,0)\nipd = di.GetPointData()\nci = self.GetInputDataObject(0,1)\nido = self.GetImageDataOutput()\ndiExt = di.GetExtent()\noffs = 192\nctrs = ci.GetPoints().GetPoint(0)\nctrX = int(ctrs[0])\nctrY = int(ctrs[1])\noext = [ int(max([int(ctrX-offs),int(diExt[0])])), int(min([int(ctrX+offs),int(diExt[1])]))\n       , int(max([int(ctrY-offs),int(diExt[2])])), int(min([int(ctrY+offs),int(diExt[3])]))\n       , int(diExt[4]), int(diExt[5])]\nvoi = vtk.vtkExtractVOI()\nvoi.SetVOI(oext[0], oext[1], oext[2], oext[3], oext[4], oext[5])\nvoi.SetInputData(di)\nvoi.Update()\nido.ShallowCopy(voi.GetOutput())"
      extractV.RequestInformationScript = 'from paraview import util\n\nutil.SetOutputWholeExtent(self, self.GetOutput().GetExtent())\n'
      extractV.RequestUpdateExtentScript = ''
      extractV.PythonPath = ''

      # create a new 'Programmable Filter'
      extractP = ProgrammableFilter(Input=[p_20151101_170000raw, extractCycloneCenter])
      extractP.OutputDataSetType = 'vtkImageData'
      extractP.Script = "import vtk\ndi = self.GetInputDataObject(0,0)\nipd = di.GetPointData()\nci = self.GetInputDataObject(0,1)\nido = self.GetImageDataOutput()\ndiExt = di.GetExtent()\noffs = 192\nctrs = ci.GetPoints().GetPoint(0)\nctrX = int(ctrs[0])\nctrY = int(ctrs[1])\noext = [ int(max([int(ctrX-offs),int(diExt[0])])), int(min([int(ctrX+offs),int(diExt[1])]))\n       , int(max([int(ctrY-offs),int(diExt[2])])), int(min([int(ctrY+offs),int(diExt[3])]))\n       , int(diExt[4]), int(diExt[5])]\nvoi = vtk.vtkExtractVOI()\nvoi.SetVOI(oext[0], oext[1], oext[2], oext[3], oext[4], oext[5])\nvoi.SetInputData(di)\nvoi.Update()\nido.ShallowCopy(voi.GetOutput())"
      extractP.RequestInformationScript = 'from paraview import util\n\nutil.SetOutputWholeExtent(self, self.GetOutput().GetExtent())\n'
      extractP.RequestUpdateExtentScript = ''
      extractP.PythonPath = ''

      # create a new 'Programmable Filter'
      extractU = ProgrammableFilter(Input=[u_20151101_170000raw, extractCycloneCenter])
      extractU.OutputDataSetType = 'vtkImageData'
      extractU.Script = "import vtk\ndi = self.GetInputDataObject(0,0)\nipd = di.GetPointData()\nci = self.GetInputDataObject(0,1)\nido = self.GetImageDataOutput()\ndiExt = di.GetExtent()\noffs = 192\nctrs = ci.GetPoints().GetPoint(0)\nctrX = int(ctrs[0])\nctrY = int(ctrs[1])\noext = [ int(max([int(ctrX-offs),int(diExt[0])])), int(min([int(ctrX+offs),int(diExt[1])]))\n       , int(max([int(ctrY-offs),int(diExt[2])])), int(min([int(ctrY+offs),int(diExt[3])]))\n       , int(diExt[4]), int(diExt[5])]\nvoi = vtk.vtkExtractVOI()\nvoi.SetVOI(oext[0], oext[1], oext[2], oext[3], oext[4], oext[5])\nvoi.SetInputData(di)\nvoi.Update()\nido.ShallowCopy(voi.GetOutput())"
      extractU.RequestInformationScript = 'from paraview import util\n\nutil.SetOutputWholeExtent(self, self.GetOutput().GetExtent())\n'
      extractU.RequestUpdateExtentScript = ''
      extractU.PythonPath = ''

      # create a new 'Programmable Filter'
      computeVelocityMagnitude = ProgrammableFilter(Input=[extractU, extractV, extractW])
      computeVelocityMagnitude.OutputDataSetType = 'vtkImageData'
      computeVelocityMagnitude.Script = "import vtk\nfrom math import sqrt\ndoU = self.GetInputDataObject(0,0)\ndoV = self.GetInputDataObject(0,1)\ndoW = self.GetInputDataObject(0,2)\ndiU = doU.GetPointData()\ndiV = doV.GetPointData()\ndiW = doW.GetPointData()\nidiU = diU.GetScalars('U')\nidiV = diV.GetScalars('V')\nidiW = diW.GetScalars('W')\nido = self.GetImageDataOutput()\n[minX, maxX, minY, maxY, minZ, maxZ] = doU.GetExtent()\n(dimX, dimY, dimZ) = doU.GetDimensions()\nido.SetExtent([0, maxX - minX, 0, maxY - minY, 0, maxZ - minZ])\nca = vtk.vtkFloatArray()\nca.SetName('vel')\nca.SetNumberOfComponents(1)\nca.SetNumberOfTuples(dimX*dimY*dimZ)\nfor k in range(0, ca.GetNumberOfTuples()):\n  kU = idiU.GetTuple1(k)\n  kV = idiV.GetTuple1(k)\n  kW = idiW.GetTuple1(k)\n  ca.SetValue(k, sqrt(kU*kU + kV*kV +kW*kW))\nido.GetPointData().AddArray(ca)"
      computeVelocityMagnitude.RequestInformationScript = ''
      computeVelocityMagnitude.RequestUpdateExtentScript = ''
      computeVelocityMagnitude.PythonPath = ''


      # TODO: Fix parallelImageDataWriterVelMag, only the first timestep has valid data
      #  vtkImageData (0xcae38d0): Point array vel with 1 components, only has 0 tuples but there are 4040764 points
      #  vtkXMLImageDataWriter (0xcb8bbb0): Input is invalid for piece 0.  Aborting.
      #  Algorithm vtkXMLImageDataWriter(0xc5d1550) returned failure for request: vtkInformation (0xc59f9d0)
      #    Debug: Off
      #    Modified Time: 614687
      #    Reference Count: 1
      #    Registered Events: (none)
      #    Request: REQUEST_DATA
      #    FORWARD_DIRECTION: 0
      #    ALGORITHM_AFTER_FORWARD: 1
      #    FROM_OUTPUT_PORT: -1

      # create Parallel Image Data Writers
      parallelImageDataWriterU = servermanager.writers.XMLPImageDataWriter(Input=extractU)
      parallelImageDataWriterV = servermanager.writers.XMLPImageDataWriter(Input=extractV)
      parallelImageDataWriterW = servermanager.writers.XMLPImageDataWriter(Input=extractW)
      #parallelImageDataWriterVelMag = servermanager.writers.XMLPImageDataWriter(Input=computeVelocityMagnitude)
      parallelImageDataWriterQICE   = servermanager.writers.XMLPImageDataWriter(Input=extractQICE)
      parallelImageDataWriterQGRAUP = servermanager.writers.XMLPImageDataWriter(Input=extractQGRAUP)
      parallelImageDataWriterQRAIN  = servermanager.writers.XMLPImageDataWriter(Input=extractQRAIN)

      # register Writers with coprocessor and initialize
      coprocessor.RegisterWriter(parallelImageDataWriterU, filename='cyclone_U_%t.pvti', freq=1)
      coprocessor.RegisterWriter(parallelImageDataWriterV, filename='cyclone_V_%t.pvti', freq=1)
      coprocessor.RegisterWriter(parallelImageDataWriterW, filename='cyclone_W_%t.pvti', freq=1)
      #coprocessor.RegisterWriter(parallelImageDataWriterVelMag, filename='cyclone_VelocityMagnitude_%t.pvti', freq=1)
      coprocessor.RegisterWriter(parallelImageDataWriterQICE, filename='cyclone_QICE_%t.pvti', freq=1)
      coprocessor.RegisterWriter(parallelImageDataWriterQGRAUP, filename='cyclone_QGRAUP_%t.pvti', freq=1)
      coprocessor.RegisterWriter(parallelImageDataWriterQRAIN, filename='cyclone_QRAIN_%t.pvti', freq=1)


      # ----------------------------------------------------------------
      # finally, restore active source
      SetActiveSource(extractQICE)
      # ----------------------------------------------------------------

    return Pipeline()

  class CoProcessor(coprocessing.CoProcessor):
    def CreatePipeline(self, datadescription):
      self.Pipeline = _CreatePipeline(self, datadescription)

  coprocessor = CoProcessor()
  # these are the frequencies at which the coprocessor updates.
  freqs = {'P': [1], 'U': [1], 'V': [1], 'W': [1], 'QICE': [1], 'QGRAUP': [1], 'QRAIN': [1]}
  coprocessor.SetUpdateFrequencies(freqs)
  return coprocessor

#--------------------------------------------------------------
# Global variables that will hold the pipeline for each timestep
# Creating the CoProcessor object, doesn't actually create the ParaView pipeline.
# It will be automatically setup when coprocessor.UpdateProducers() is called the
# first time.
coprocessor = CreateCoProcessor()

#--------------------------------------------------------------
# Enable Live-Visualizaton with ParaView
coprocessor.EnableLiveVisualization(True, 1)


#--------------------------------------------------------------
# Dynamically determine client
clientport = 22222
clienthost = 'localhost'
if 'SSH_CLIENT' in os.environ:
  clienthost = os.environ['SSH_CLIENT'].split()[0]
if 'INSHIMTU_CLIENT' in os.environ:
  clienthost = os.environ['INSHIMTU_CLIENT']


# ---------------------- Data Selection method ----------------------

def RequestDataDescription(datadescription):
    "Callback to populate the request for current timestep"
    global coprocessor
    datadescription.SetForceOutput(True)
    if datadescription.GetForceOutput() == True:
        # We are just going to request all fields and meshes from the simulation
        # code/adaptor.
        for i in range(datadescription.GetNumberOfInputDescriptions()):
            datadescription.GetInputDescription(i).AllFieldsOn()
            datadescription.GetInputDescription(i).GenerateMeshOn()
        return

    # setup requests for all inputs based on the requirements of the
    # pipeline.
    coprocessor.LoadRequestedData(datadescription)

# ------------------------ Processing method ------------------------

def DoCoProcessing(datadescription):
    "Callback to do co-processing for current timestep"
    global coprocessor

    # Update the coprocessor by providing it the newly generated simulation data.
    # If the pipeline hasn't been setup yet, this will setup the pipeline.
    coprocessor.UpdateProducers(datadescription)

    # Write output data, if appropriate.
    coprocessor.WriteData(datadescription);

    # Write image capture (Last arg: rescale lookup table), if appropriate.
    coprocessor.WriteImages(datadescription, rescale_lookuptable=False)

    # Live Visualization, if enabled.
    coprocessor.DoLiveVisualization(datadescription, clienthost, clientport)

