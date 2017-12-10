
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

if 'INSHIMTU_WRITE_OUTPUT_DIR' in os.environ:
  OutputDir = os.path.join(os.getcwd(), os.environ['INSHIMTU_WRITE_OUTPUT_DIR'])
else:
  OutputDir = os.path.join(os.getcwd(), 'output')


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
      extractCycloneCenter.Script = "execfile('%s')" % os.path.join(ScriptDir, 'filter_extractCycloneCenter.Script.py')
      extractCycloneCenter.RequestInformationScript = ''
      extractCycloneCenter.RequestUpdateExtentScript = ''
      extractCycloneCenter.PythonPath = ''

      # create a new 'Programmable Filter'
      extractQICE = ProgrammableFilter(Input=[qICE_20151101_170000raw, extractCycloneCenter])
      extractQICE.OutputDataSetType = 'vtkImageData'
      extractQICE.Script = "execfile('%s')" % os.path.join(ScriptDir, 'filter_extractQICE.Script.py')
      extractQICE.RequestInformationScript = 'from paraview import util\nutil.SetOutputWholeExtent(self, self.GetOutput().GetExtent())'
      extractQICE.RequestUpdateExtentScript = ''
      extractQICE.PythonPath = ''

      # create a new 'Programmable Filter'
      extractQGRAUP = ProgrammableFilter(Input=[qGRAUP_20151101_170000raw, extractCycloneCenter])
      extractQGRAUP.OutputDataSetType = 'vtkImageData'
      extractQGRAUP.Script = "execfile('%s')" % os.path.join(ScriptDir, 'filter_extractQGRAUP.Script.py')
      extractQGRAUP.RequestInformationScript = 'from paraview import util\nutil.SetOutputWholeExtent(self, self.GetOutput().GetExtent())'
      extractQGRAUP.RequestUpdateExtentScript = ''
      extractQGRAUP.PythonPath = ''

      # create a new 'Programmable Filter'
      extractQRAIN = ProgrammableFilter(Input=[qRAIN_20151101_170000raw, extractCycloneCenter])
      extractQRAIN.OutputDataSetType = 'vtkImageData'
      extractQRAIN.Script = "execfile('%s')" % os.path.join(ScriptDir, 'filter_extractQRAIN.Script.py')
      extractQRAIN.RequestInformationScript = 'from paraview import util\nutil.SetOutputWholeExtent(self, self.GetOutput().GetExtent())'
      extractQRAIN.RequestUpdateExtentScript = ''
      extractQRAIN.PythonPath = ''

      # create a new 'Image Reader'
      # create a producer from a simulation input
      w_20151101_170000raw = coprocessor.CreateProducer(datadescription, 'W')

      # create a new 'Programmable Filter'
      extractW = ProgrammableFilter(Input=[w_20151101_170000raw, extractCycloneCenter])
      extractW.OutputDataSetType = 'vtkImageData'
      extractW.Script = "execfile('%s')" % os.path.join(ScriptDir, 'filter_extractW.Script.py')
      extractW.RequestInformationScript = 'from paraview import util\nutil.SetOutputWholeExtent(self, self.GetOutput().GetExtent())'
      extractW.RequestUpdateExtentScript = ''
      extractW.PythonPath = ''

      # create a new 'Image Reader'
      # create a producer from a simulation input
      v_20151101_170000raw = coprocessor.CreateProducer(datadescription, 'V')

      # create a new 'Programmable Filter'
      extractV = ProgrammableFilter(Input=[v_20151101_170000raw, extractCycloneCenter])
      extractV.OutputDataSetType = 'vtkImageData'
      extractV.Script = "execfile('%s')" % os.path.join(ScriptDir, 'filter_extractV.Script.py')
      extractV.RequestInformationScript = 'from paraview import util\nutil.SetOutputWholeExtent(self, self.GetOutput().GetExtent())'
      extractV.RequestUpdateExtentScript = ''
      extractV.PythonPath = ''

      # create a new 'Programmable Filter'
      extractP = ProgrammableFilter(Input=[p_20151101_170000raw, extractCycloneCenter])
      extractP.OutputDataSetType = 'vtkImageData'
      extractP.Script = "execfile('%s')" % os.path.join(ScriptDir, 'filter_extractP.Script.py')
      extractP.RequestInformationScript = 'from paraview import util\nutil.SetOutputWholeExtent(self, self.GetOutput().GetExtent())'
      extractP.RequestUpdateExtentScript = ''
      extractP.PythonPath = ''

      # create a new 'Programmable Filter'
      extractU = ProgrammableFilter(Input=[u_20151101_170000raw, extractCycloneCenter])
      extractU.OutputDataSetType = 'vtkImageData'
      extractU.Script = "execfile('%s')" % os.path.join(ScriptDir, 'filter_extractU.Script.py')
      extractU.RequestInformationScript = 'from paraview import util\nutil.SetOutputWholeExtent(self, self.GetOutput().GetExtent())'
      extractU.RequestUpdateExtentScript = ''
      extractU.PythonPath = ''

      # create a new 'Programmable Filter'
      computeVelocityMagnitude = ProgrammableFilter(Input=[extractU, extractV, extractW])
      computeVelocityMagnitude.OutputDataSetType = 'vtkImageData'
      computeVelocityMagnitude.Script = "execfile('%s')" % os.path.join(ScriptDir, 'filter_computeVelocityMagnitude.Script.py')
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
      coprocessor.RegisterWriter(parallelImageDataWriterU, filename=os.path.join(OutputDir, 'cyclone_U_%t.pvti'), freq=1)
      coprocessor.RegisterWriter(parallelImageDataWriterV, filename=os.path.join(OutputDir, 'cyclone_V_%t.pvti'), freq=1)
      coprocessor.RegisterWriter(parallelImageDataWriterW, filename=os.path.join(OutputDir, 'cyclone_W_%t.pvti'), freq=1)
      #coprocessor.RegisterWriter(parallelImageDataWriterVelMag, filename=os.path.join(OutputDir, 'cyclone_VelocityMagnitude_%t.pvti'), freq=1)
      coprocessor.RegisterWriter(parallelImageDataWriterQICE, filename=os.path.join(OutputDir, 'cyclone_QICE_%t.pvti'), freq=1)
      coprocessor.RegisterWriter(parallelImageDataWriterQGRAUP, filename=os.path.join(OutputDir, 'cyclone_QGRAUP_%t.pvti'), freq=1)
      coprocessor.RegisterWriter(parallelImageDataWriterQRAIN, filename=os.path.join(OutputDir, 'cyclone_QRAIN_%t.pvti'), freq=1)


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
coprocessor.EnableLiveVisualization(False, 1)


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

