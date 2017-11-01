
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

      # create a new 'Image Reader'
      # create a producer from a simulation input
      p_20151101_170000raw = coprocessor.CreateProducer(datadescription, 'P')

      # create a new 'Programmable Filter'
      extractCycloneCenter = ProgrammableFilter(Input=p_20151101_170000raw)
      extractCycloneCenter.OutputDataSetType = 'vtkPolyData'
      extractCycloneCenter.Script = "di = inputs[0]\npdi = di.PointData['P']\npdo = self.GetPolyDataOutput()\nnewPts = vtk.vtkPoints()\n(dimX, dimY, dimZ) = di.GetDimensions()\nminIdx = pdi.argmin()\ni = minIdx % (dimX * dimY)\nminX = i % dimX\nminY = i // dimX\nnewPts.InsertPoint(0, minX, minY, 0)\npdo.SetPoints(newPts)"
      extractCycloneCenter.RequestInformationScript = ''
      extractCycloneCenter.RequestUpdateExtentScript = ''
      extractCycloneCenter.PythonPath = ''

      # create a new 'Programmable Filter'
      extractQICE = ProgrammableFilter(Input=[qICE_20151101_170000raw, extractCycloneCenter])
      extractQICE.OutputDataSetType = 'vtkImageData'
      extractQICE.Script = "import vtk\ndi = inputs[0]\ngi = self.GetInput()\nci = inputs[1]\nidi = di.PointData['QICE']\nido = self.GetImageDataOutput()\ndiExt = di.GetExtent()\noffs = 64\nctrs = ci.GetPoints()\nctrX = int(ctrs[0][0])\nctrY = int(ctrs[0][1])\noext = [ int(max([int(ctrX-offs),int(diExt[0])])), int(min([int(ctrX+offs),int(diExt[1])]))\n       , int(max([int(ctrY-offs),int(diExt[2])])), int(min([int(ctrY+offs),int(diExt[3])]))\n       , int(diExt[4]), int(diExt[5])]\nvoi = vtk.vtkExtractVOI()\nvoi.SetVOI(oext[0], oext[1], oext[2], oext[3], oext[4], oext[5])\nvoi.SetInputData(gi)\nvoi.Update()\nido.ShallowCopy(voi.GetOutput())"
      extractQICE.RequestInformationScript = 'from paraview import util\n\nutil.SetOutputWholeExtent(self, self.GetOutput().GetExtent())\n'
      extractQICE.RequestUpdateExtentScript = ''
      extractQICE.PythonPath = ''

      # create a new 'Image Reader'
      # create a producer from a simulation input
      w_20151101_170000raw = coprocessor.CreateProducer(datadescription, 'W')

      # create a new 'Programmable Filter'
      extractW = ProgrammableFilter(Input=[w_20151101_170000raw, extractCycloneCenter])
      extractW.OutputDataSetType = 'vtkImageData'
      extractW.Script = "import vtk\ndi = inputs[0]\ngi = self.GetInput()\nci = inputs[1]\nidi = di.PointData['W']\nido = self.GetImageDataOutput()\ndiExt = di.GetExtent()\noffs = 64\nctrs = ci.GetPoints()\nctrX = int(ctrs[0][0])\nctrY = int(ctrs[0][1])\noext = [ int(max([int(ctrX-offs),int(diExt[0])])), int(min([int(ctrX+offs),int(diExt[1])]))\n       , int(max([int(ctrY-offs),int(diExt[2])])), int(min([int(ctrY+offs),int(diExt[3])]))\n       , int(diExt[4]), int(diExt[5])]\nvoi = vtk.vtkExtractVOI()\nvoi.SetVOI(oext[0], oext[1], oext[2], oext[3], oext[4], oext[5])\nvoi.SetInputData(gi)\nvoi.Update()\nido.ShallowCopy(voi.GetOutput())"
      extractW.RequestInformationScript = 'from paraview import util\n\nutil.SetOutputWholeExtent(self, self.GetOutput().GetExtent())\n'
      extractW.RequestUpdateExtentScript = ''
      extractW.PythonPath = ''

      # create a new 'Image Reader'
      # create a producer from a simulation input
      v_20151101_170000raw = coprocessor.CreateProducer(datadescription, 'V')

      # create a new 'Programmable Filter'
      extractV = ProgrammableFilter(Input=[v_20151101_170000raw, extractCycloneCenter])
      extractV.OutputDataSetType = 'vtkImageData'
      extractV.Script = "import vtk\ndi = inputs[0]\ngi = self.GetInput()\nci = inputs[1]\nidi = di.PointData['V']\nido = self.GetImageDataOutput()\ndiExt = di.GetExtent()\noffs = 64\nctrs = ci.GetPoints()\nctrX = int(ctrs[0][0])\nctrY = int(ctrs[0][1])\noext = [ int(max([int(ctrX-offs),int(diExt[0])])), int(min([int(ctrX+offs),int(diExt[1])]))\n       , int(max([int(ctrY-offs),int(diExt[2])])), int(min([int(ctrY+offs),int(diExt[3])]))\n       , int(diExt[4]), int(diExt[5])]\nvoi = vtk.vtkExtractVOI()\nvoi.SetVOI(oext[0], oext[1], oext[2], oext[3], oext[4], oext[5])\nvoi.SetInputData(gi)\nvoi.Update()\nido.ShallowCopy(voi.GetOutput())"
      extractV.RequestInformationScript = 'from paraview import util\n\nutil.SetOutputWholeExtent(self, self.GetOutput().GetExtent())\n'
      extractV.RequestUpdateExtentScript = ''
      extractV.PythonPath = ''

      # create a new 'Programmable Filter'
      extractP = ProgrammableFilter(Input=[p_20151101_170000raw, extractCycloneCenter])
      extractP.OutputDataSetType = 'vtkImageData'
      extractP.Script = "import vtk\ndi = inputs[0]\ngi = self.GetInput()\nci = inputs[1]\nidi = di.PointData['P']\nido = self.GetImageDataOutput()\ndiExt = di.GetExtent()\noffs = 64\nctrs = ci.GetPoints()\nctrX = int(ctrs[0][0])\nctrY = int(ctrs[0][1])\noext = [ int(max([int(ctrX-offs),int(diExt[0])])), int(min([int(ctrX+offs),int(diExt[1])]))\n       , int(max([int(ctrY-offs),int(diExt[2])])), int(min([int(ctrY+offs),int(diExt[3])]))\n       , int(diExt[4]), int(diExt[5])]\nvoi = vtk.vtkExtractVOI()\nvoi.SetVOI(oext[0], oext[1], oext[2], oext[3], oext[4], oext[5])\nvoi.SetInputData(gi)\nvoi.Update()\nido.ShallowCopy(voi.GetOutput())"
      extractP.RequestInformationScript = 'from paraview import util\n\nutil.SetOutputWholeExtent(self, self.GetOutput().GetExtent())\n'
      extractP.RequestUpdateExtentScript = ''
      extractP.PythonPath = ''

      # create a new 'Programmable Filter'
      extractU = ProgrammableFilter(Input=[u_20151101_170000raw, extractCycloneCenter])
      extractU.OutputDataSetType = 'vtkImageData'
      extractU.Script = "import vtk\ndi = inputs[0]\ngi = self.GetInput()\nci = inputs[1]\nidi = di.PointData['U']\nido = self.GetImageDataOutput()\ndiExt = di.GetExtent()\noffs = 64\nctrs = ci.GetPoints()\nctrX = int(ctrs[0][0])\nctrY = int(ctrs[0][1])\noext = [ int(max([int(ctrX-offs),int(diExt[0])])), int(min([int(ctrX+offs),int(diExt[1])]))\n       , int(max([int(ctrY-offs),int(diExt[2])])), int(min([int(ctrY+offs),int(diExt[3])]))\n       , int(diExt[4]), int(diExt[5])]\nvoi = vtk.vtkExtractVOI()\nvoi.SetVOI(oext[0], oext[1], oext[2], oext[3], oext[4], oext[5])\nvoi.SetInputData(gi)\nvoi.Update()\nido.ShallowCopy(voi.GetOutput())"
      extractU.RequestInformationScript = 'from paraview import util\n\nutil.SetOutputWholeExtent(self, self.GetOutput().GetExtent())\n'
      extractU.RequestUpdateExtentScript = ''
      extractU.PythonPath = ''

      # create a new 'Programmable Filter'
      computeVelocityMagnitude = ProgrammableFilter(Input=[extractU, extractV, extractW])
      computeVelocityMagnitude.OutputDataSetType = 'vtkImageData'
      computeVelocityMagnitude.Script = "from math import sqrt\ndiU = inputs[0]\ndiV = inputs[1]\ndiW = inputs[2]\nidiU = diU.PointData['U']\nidiV = diV.PointData['V']\nidiW = diW.PointData['W']\nido = self.GetImageDataOutput()\n[minX, maxX, minY, maxY, minZ, maxZ] = diU.GetExtent()\n(dimX, dimY, dimZ) = diU.GetDimensions()\nido.SetExtent([0, maxX - minX, 0, maxY - minY, 0, maxZ - minZ])\nca = vtk.vtkFloatArray()\nca.SetName('vel')\nca.SetNumberOfComponents(1)\nca.SetNumberOfTuples(dimX*dimY*dimZ)\nfor k in range(0, ca.GetNumberOfTuples()):\n  ca.SetValue(k, sqrt(idiU[k]*idiU[k] + idiV[k]*idiV[k] +idiW[k]*idiW[k]))\nido.GetPointData().AddArray(ca)"
      computeVelocityMagnitude.RequestInformationScript = ''
      computeVelocityMagnitude.RequestUpdateExtentScript = ''
      computeVelocityMagnitude.PythonPath = ''

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
  freqs = {'P': [1], 'U': [1], 'V': [1], 'W': [1], 'QICE': [1]}
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

