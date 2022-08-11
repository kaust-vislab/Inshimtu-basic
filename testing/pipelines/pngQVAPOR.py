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
      # setup views used in the visualization
      # ----------------------------------------------------------------

      #### disable automatic camera reset on 'Show'
      paraview.simple._DisableFirstRenderCameraReset()

      # Create a new 'Render View'
      renderView1 = CreateView('RenderView')
      renderView1.ViewSize = [1480, 1452]
      renderView1.AxesGrid = 'GridAxes3DActor'
      renderView1.CenterOfRotation = [5.0, 4.5, 1.5]
      renderView1.StereoType = 0
      renderView1.CameraPosition = [0.6183627970877066, -0.7463673189222227, 24.06883815584594]
      renderView1.CameraFocalPoint = [5.0, 4.5, 1.5000000000000007]
      renderView1.CameraViewUp = [0.005860598183340073, -0.9742625779818028, -0.22533992662904875]
      renderView1.CameraParallelScale = 6.103277807866851
      renderView1.Background = [0.32, 0.34, 0.43]

      # register the view with coprocessor
      # and provide it with information such as the filename to use,
      # how frequently to write the images, etc.
      coprocessor.RegisterView(renderView1,
          filename='image_%t.png', freq=1, fittoscreen=0, magnification=1, width=1480, height=1452, cinema={})
      renderView1.ViewTime = datadescription.GetTime()

      # ----------------------------------------------------------------
      # setup the data processing pipelines
      # ----------------------------------------------------------------

      # create a new 'XML Partitioned Image Data Reader'
      # create a producer from a simulation input
      dataoutfile_QVAPOR_0pvti = coprocessor.CreateProducer(datadescription, 'QVAPOR')

      # ----------------------------------------------------------------
      # setup color maps and opacity mapes used in the visualization
      # note: the Get..() functions create a new object, if needed
      # ----------------------------------------------------------------

      # get color transfer function/color map for 'QVAPOR'
      qVAPORLUT = GetColorTransferFunction('QVAPOR')
      qVAPORLUT.RGBPoints = [1.0, 0.231373, 0.298039, 0.752941, 8.5, 0.865003, 0.865003, 0.865003, 16.0, 0.705882, 0.0156863, 0.14902]
      qVAPORLUT.ScalarRangeInitialized = 1.0

      # get opacity transfer function/opacity map for 'QVAPOR'
      qVAPORPWF = GetOpacityTransferFunction('QVAPOR')
      qVAPORPWF.Points = [1.0, 0.0, 0.5, 0.0, 16.0, 1.0, 0.5, 0.0]
      qVAPORPWF.ScalarRangeInitialized = 1

      # ----------------------------------------------------------------
      # setup the visualization in view 'renderView1'
      # ----------------------------------------------------------------

      # show data from dataoutfile_QVAPOR_0pvti
      dataoutfile_QVAPOR_0pvtiDisplay = Show(dataoutfile_QVAPOR_0pvti, renderView1)
      # trace defaults for the display properties.
      dataoutfile_QVAPOR_0pvtiDisplay.Representation = 'Volume'
      dataoutfile_QVAPOR_0pvtiDisplay.ColorArrayName = ['CELLS', 'QVAPOR']
      dataoutfile_QVAPOR_0pvtiDisplay.LookupTable = qVAPORLUT
      dataoutfile_QVAPOR_0pvtiDisplay.OSPRayScaleArray = 'QVAPOR'
      dataoutfile_QVAPOR_0pvtiDisplay.OSPRayScaleFunction = 'PiecewiseFunction'
      dataoutfile_QVAPOR_0pvtiDisplay.SelectOrientationVectors = 'None'
      dataoutfile_QVAPOR_0pvtiDisplay.ScaleFactor = 0.9
      dataoutfile_QVAPOR_0pvtiDisplay.SelectScaleArray = 'QVAPOR'
      dataoutfile_QVAPOR_0pvtiDisplay.GlyphType = 'Arrow'
      dataoutfile_QVAPOR_0pvtiDisplay.PolarAxes = 'PolarAxesRepresentation'
      dataoutfile_QVAPOR_0pvtiDisplay.ScalarOpacityUnitDistance = 2.3288363443082876
      dataoutfile_QVAPOR_0pvtiDisplay.Slice = 1

      # ----------------------------------------------------------------
      # finally, restore active source
      SetActiveSource(dataoutfile_QVAPOR_0pvti)
      # ----------------------------------------------------------------
    return Pipeline()

  class CoProcessor(coprocessing.CoProcessor):
    def CreatePipeline(self, datadescription):
      self.Pipeline = _CreatePipeline(self, datadescription)

  coprocessor = CoProcessor()
  # these are the frequencies at which the coprocessor updates.
  freqs = {'QVAPOR': [1]}
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

    # TODO: Fix update issue 
    #   If output is not forced, Catalyst fails to request processing, even
    #   if ParaView client is connected.
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
