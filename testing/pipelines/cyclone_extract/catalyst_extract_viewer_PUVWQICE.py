
from paraview.simple import *
from paraview import coprocessing

import os

# TODO: Parallelize 
# http://www.programcreek.com/python/example/65345/vtk.vtkImageMathematics
# http://www.vtk.org/doc/nightly/html/classvtkThreadedImageAlgorithm.html
# https://blog.kitware.com/a-vtk-pipeline-primer-part-1/
# http://www.paraview.org/ParaView/Doc/Nightly/www/py-doc/paraview.simple.ProgrammableFilter.html
# http://www.paraview.org/ParaView/Doc/Nightly/www/py-doc/paraview.vtk.numpy_interface.dataset_adapter.html
# https://blog.kitware.com/improved-vtk-numpy-integration/
# http://public.kitware.com/pipermail/paraview/2011-August/022529.html
# https://cmake.org/pipermail/paraview/2011-August/022403.html
# https://blog.kitware.com/developing-hdf5-readers-using-vtkpythonalgorithm/
# https://blog.kitware.com/vtkpythonalgorithm-is-great/


#--------------------------------------------------------------
# Code generated from cpstate.py to create the CoProcessor.
# ParaView 5.3.0 64 bits


#--------------------------------------------------------------
# Dynamically determine Filter Scripts directory

if 'INSHIMTU_FILTER_SCRIPT_DIR' in os.environ:
  ScriptDir = os.path.join(os.getcwd(), os.environ['INSHIMTU_FILTER_SCRIPT_DIR'])
else:
  ScriptDir = os.path.join(os.getcwd(), 'filters')


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
      p_raw = coprocessor.CreateProducer(datadescription, 'P')

      u_raw = coprocessor.CreateProducer(datadescription, 'U')
      v_raw = coprocessor.CreateProducer(datadescription, 'V')
      w_raw = coprocessor.CreateProducer(datadescription, 'W')

      qICE_raw = coprocessor.CreateProducer(datadescription, 'QICE')

      # create a new 'Programmable Filter'
      extractCycloneCenter = ProgrammableFilter(Input=p_raw)
      extractCycloneCenter.OutputDataSetType = 'vtkPolyData'
      extractCycloneCenter.Script = "execfile('%s')" % os.path.join(ScriptDir, 'filter_extractCycloneCenter.Script.py')
      extractCycloneCenter.RequestInformationScript = ''
      extractCycloneCenter.RequestUpdateExtentScript = ''
      extractCycloneCenter.PythonPath = ''

      # create a new 'Programmable Filter'
      extractU = ProgrammableFilter(Input=[u_raw, extractCycloneCenter])
      extractU.OutputDataSetType = 'vtkImageData'
      extractU.Script = "execfile('%s')" % os.path.join(ScriptDir, 'filter_extractU.Script.py')
      extractU.RequestInformationScript = 'from paraview import util\nutil.SetOutputWholeExtent(self, self.GetOutput().GetExtent())'
      extractU.RequestUpdateExtentScript = ''
      extractU.PythonPath = ''

      # create a new 'Programmable Filter'
      extractV = ProgrammableFilter(Input=[v_raw, extractCycloneCenter])
      extractV.OutputDataSetType = 'vtkImageData'
      extractV.Script = "execfile('%s')" % os.path.join(ScriptDir, 'filter_extractV.Script.py')
      extractV.RequestInformationScript = 'from paraview import util\nutil.SetOutputWholeExtent(self, self.GetOutput().GetExtent())'
      extractV.RequestUpdateExtentScript = ''
      extractV.PythonPath = ''

      # create a new 'Programmable Filter'
      extractW = ProgrammableFilter(Input=[w_raw, extractCycloneCenter])
      extractW.OutputDataSetType = 'vtkImageData'
      extractW.Script = "execfile('%s')" % os.path.join(ScriptDir, 'filter_extractW.Script.py')
      extractW.RequestInformationScript = 'from paraview import util\nutil.SetOutputWholeExtent(self, self.GetOutput().GetExtent())'
      extractW.RequestUpdateExtentScript = ''
      extractW.PythonPath = ''

      # create a new 'Programmable Filter'
      extractP = ProgrammableFilter(Input=[p_raw, extractCycloneCenter])
      extractP.OutputDataSetType = 'vtkImageData'
      extractP.Script = "execfile('%s')" % os.path.join(ScriptDir, 'filter_extractP.Script.py')
      extractP.RequestInformationScript = 'from paraview import util\nutil.SetOutputWholeExtent(self, self.GetOutput().GetExtent())'
      extractP.RequestUpdateExtentScript = ''
      extractP.PythonPath = ''

      # create a new 'Programmable Filter'
      extractQICE = ProgrammableFilter(Input=[qICE_raw, extractCycloneCenter])
      extractQICE.OutputDataSetType = 'vtkImageData'
      extractQICE.Script = "execfile('%s')" % os.path.join(ScriptDir, 'filter_extractQICE.Script.py')
      extractQICE.RequestInformationScript = 'from paraview import util\nutil.SetOutputWholeExtent(self, self.GetOutput().GetExtent())'
      extractQICE.RequestUpdateExtentScript = ''
      extractQICE.PythonPath = ''

      # create a new 'Programmable Filter'
      computeVelocityMagnitude = ProgrammableFilter(Input=[extractU, extractV, extractW])
      computeVelocityMagnitude.OutputDataSetType = 'vtkImageData'
      computeVelocityMagnitude.Script = "execfile('%s')" % os.path.join(ScriptDir, 'filter_computeVelocityMagnitude.Script.py')
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

