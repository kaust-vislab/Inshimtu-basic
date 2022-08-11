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
      renderView1.ViewSize = [736, 711]
      renderView1.AxesGrid = 'GridAxes3DActor'
      renderView1.CenterOfRotation = [331.0, 331.0, 16.5]
      renderView1.StereoType = 0
      renderView1.CameraPosition = [797.9782671456427, 500.11361840542565, -1723.756829791978]
      renderView1.CameraFocalPoint = [331.0000000000002, 330.99999999999994, 16.5000000000002]
      renderView1.CameraViewUp = [-0.021319652945212715, 0.9956198354574075, 0.09103084994693067]
      renderView1.CameraParallelScale = 468.39539920883084
      renderView1.Background = [0.32, 0.34, 0.43]

      # register the view with coprocessor
      # and provide it with information such as the filename to use,
      # how frequently to write the images, etc.
      coprocessor.RegisterView(renderView1,
          filename='image_0_%t.png', freq=1, fittoscreen=0, magnification=1, width=736, height=711, cinema={})
      renderView1.ViewTime = datadescription.GetTime()

      # Create a new 'Render View'
      renderView2 = CreateView('RenderView')
      renderView2.ViewSize = [735, 711]
      renderView2.AxesGrid = 'GridAxes3DActor'
      renderView2.CenterOfRotation = [331.0, 331.0, 16.5]
      renderView2.StereoType = 0
      renderView2.CameraPosition = [331.0, 331.0, 1826.2408520431518]
      renderView2.CameraFocalPoint = [331.0, 331.0, 16.5]
      renderView2.CameraParallelScale = 468.39539920883084
      renderView2.Background = [0.32, 0.34, 0.43]

      # register the view with coprocessor
      # and provide it with information such as the filename to use,
      # how frequently to write the images, etc.
      coprocessor.RegisterView(renderView2,
          filename='image_1_%t.png', freq=1, fittoscreen=0, magnification=1, width=735, height=711, cinema={})
      renderView2.ViewTime = datadescription.GetTime()

      # Create a new 'Render View'
      renderView3 = CreateView('RenderView')
      renderView3.ViewSize = [736, 711]
      renderView3.AxesGrid = 'GridAxes3DActor'
      renderView3.CenterOfRotation = [331.0, 331.0, 16.5]
      renderView3.StereoType = 0
      renderView3.CameraPosition = [331.0, 331.0, 1826.2408520431518]
      renderView3.CameraFocalPoint = [331.0, 331.0, 16.5]
      renderView3.CameraParallelScale = 468.39539920883084
      renderView3.Background = [0.32, 0.34, 0.43]

      # register the view with coprocessor
      # and provide it with information such as the filename to use,
      # how frequently to write the images, etc.
      coprocessor.RegisterView(renderView3,
          filename='image_2_%t.png', freq=1, fittoscreen=0, magnification=1, width=736, height=711, cinema={})
      renderView3.ViewTime = datadescription.GetTime()

      # Create a new 'Render View'
      renderView4 = CreateView('RenderView')
      renderView4.ViewSize = [735, 711]
      renderView4.AxesGrid = 'GridAxes3DActor'
      renderView4.CenterOfRotation = [331.0, 331.0, 16.5]
      renderView4.StereoType = 0
      renderView4.CameraPosition = [384.52295403099487, -197.93513729111103, -1708.3584181096662]
      renderView4.CameraFocalPoint = [330.85735123418533, 332.4097118871088, 21.097072956885334]
      renderView4.CameraViewUp = [0.99955745494628, 0.010947021067652433, 0.027659663610748092]
      renderView4.CameraParallelScale = 468.39539920883084
      renderView4.Background = [0.32, 0.34, 0.43]

      # register the view with coprocessor
      # and provide it with information such as the filename to use,
      # how frequently to write the images, etc.
      coprocessor.RegisterView(renderView4,
          filename='image_3_%t.png', freq=1, fittoscreen=0, magnification=1, width=735, height=711, cinema={})
      renderView4.ViewTime = datadescription.GetTime()

      # ----------------------------------------------------------------
      # setup the data processing pipelines
      # ----------------------------------------------------------------

      # create a new 'XML Rectilinear Grid Reader'
      # create a producer from a simulation input
      #wrfUVWQVAPOR_0000vtr = coprocessor.CreateProducer(datadescription, 'input')
      adaptor_QVAPOR = coprocessor.CreateProducer( datadescription, "QVAPOR" )
      adaptor_U = coprocessor.CreateProducer( datadescription, "U" )
      adaptor_V = coprocessor.CreateProducer( datadescription, "V" )
      adaptor_W = coprocessor.CreateProducer( datadescription, "W" )

      # ----------------------------------------------------------------
      # setup color maps and opacity mapes used in the visualization
      # note: the Get..() functions create a new object, if needed
      # ----------------------------------------------------------------

      # get color transfer function/color map for 'U'
      uLUT = GetColorTransferFunction('U')
      uLUT.RGBPoints = [-9.799333975943515, 0.231373, 0.298039, 0.752941, 13.315200514778791, 0.865003, 0.865003, 0.865003, 36.4297350055011, 0.705882, 0.0156863, 0.14902]
      uLUT.ScalarRangeInitialized = 1.0

      # get opacity transfer function/opacity map for 'U'
      uPWF = GetOpacityTransferFunction('U')
      uPWF.Points = [-9.799333975943515, 0.0, 0.5, 0.0, 36.4297350055011, 1.0, 0.5, 0.0]
      uPWF.ScalarRangeInitialized = 1

      # get color transfer function/color map for 'W'
      wLUT = GetColorTransferFunction('W')
      wLUT.RGBPoints = [-0.7394895354775469, 0.231373, 0.298039, 0.752941, 0.1446254592080528, 0.865003, 0.865003, 0.865003, 1.0287404538936524, 0.705882, 0.0156863, 0.14902]
      wLUT.ScalarRangeInitialized = 1.0

      # get opacity transfer function/opacity map for 'W'
      wPWF = GetOpacityTransferFunction('W')
      wPWF.Points = [-0.7394895354775469, 0.0, 0.5, 0.0, 1.0287404538936524, 1.0, 0.5, 0.0]
      wPWF.ScalarRangeInitialized = 1

      # get color transfer function/color map for 'QVAPOR'
      qVAPORLUT = GetColorTransferFunction('QVAPOR')
      qVAPORLUT.RGBPoints = [9.922088060190383e-07, 0.231373, 0.298039, 0.752941, 0.009841073041067596, 0.865003, 0.865003, 0.865003, 0.019681153873329173, 0.705882, 0.0156863, 0.14902]
      qVAPORLUT.ScalarRangeInitialized = 1.0

      # get opacity transfer function/opacity map for 'QVAPOR'
      qVAPORPWF = GetOpacityTransferFunction('QVAPOR')
      qVAPORPWF.Points = [9.922088060190383e-07, 0.0, 0.5, 0.0, 0.019681153873329173, 1.0, 0.5, 0.0]
      qVAPORPWF.ScalarRangeInitialized = 1

      # get color transfer function/color map for 'V'
      vLUT = GetColorTransferFunction('V')
      vLUT.RGBPoints = [-4.118473565654042, 0.231373, 0.298039, 0.752941, 6.422053020769497, 0.865003, 0.865003, 0.865003, 16.962579607193035, 0.705882, 0.0156863, 0.14902]
      vLUT.ScalarRangeInitialized = 1.0

      # get opacity transfer function/opacity map for 'V'
      vPWF = GetOpacityTransferFunction('V')
      vPWF.Points = [-4.118473565654042, 0.0, 0.5, 0.0, 16.962579607193035, 1.0, 0.5, 0.0]
      vPWF.ScalarRangeInitialized = 1

      # ----------------------------------------------------------------
      # setup the visualization in view 'renderView1'
      # ----------------------------------------------------------------

      # show data from wrfUVWQVAPOR_0000vtr
      wrfUVWQVAPOR_0000vtrDisplay = Show(adaptor_QVAPOR, renderView1)
      # trace defaults for the display properties.
      wrfUVWQVAPOR_0000vtrDisplay.Representation = 'Surface'
      wrfUVWQVAPOR_0000vtrDisplay.ColorArrayName = ['POINTS', 'QVAPOR']
      wrfUVWQVAPOR_0000vtrDisplay.LookupTable = qVAPORLUT
      wrfUVWQVAPOR_0000vtrDisplay.OSPRayScaleArray = 'QVAPOR'
      wrfUVWQVAPOR_0000vtrDisplay.OSPRayScaleFunction = 'PiecewiseFunction'
      wrfUVWQVAPOR_0000vtrDisplay.SelectOrientationVectors = 'None'
      wrfUVWQVAPOR_0000vtrDisplay.ScaleFactor = 66.2
      wrfUVWQVAPOR_0000vtrDisplay.SelectScaleArray = 'QVAPOR'
      wrfUVWQVAPOR_0000vtrDisplay.GlyphType = 'Arrow'
      wrfUVWQVAPOR_0000vtrDisplay.PolarAxes = 'PolarAxesRepresentation'
      wrfUVWQVAPOR_0000vtrDisplay.GaussianRadius = 33.1
      wrfUVWQVAPOR_0000vtrDisplay.SetScaleArray = ['POINTS', 'QVAPOR']
      wrfUVWQVAPOR_0000vtrDisplay.ScaleTransferFunction = 'PiecewiseFunction'
      wrfUVWQVAPOR_0000vtrDisplay.OpacityArray = ['POINTS', 'QVAPOR']
      wrfUVWQVAPOR_0000vtrDisplay.OpacityTransferFunction = 'PiecewiseFunction'

      # show color legend
      wrfUVWQVAPOR_0000vtrDisplay.SetScalarBarVisibility(renderView1, True)

      # setup the color legend parameters for each legend in this view

      # get color legend/bar for qVAPORLUT in view renderView1
      qVAPORLUTColorBar = GetScalarBar(qVAPORLUT, renderView1)
      qVAPORLUTColorBar.Title = 'QVAPOR'
      qVAPORLUTColorBar.ComponentTitle = ''

      # ----------------------------------------------------------------
      # setup the visualization in view 'renderView2'
      # ----------------------------------------------------------------

      # show data from wrfUVWQVAPOR_0000vtr
      wrfUVWQVAPOR_0000vtrDisplay_1 = Show(adaptor_U, renderView2)
      # trace defaults for the display properties.
      wrfUVWQVAPOR_0000vtrDisplay_1.Representation = 'Surface With Edges'
      wrfUVWQVAPOR_0000vtrDisplay_1.ColorArrayName = ['POINTS', 'U']
      wrfUVWQVAPOR_0000vtrDisplay_1.LookupTable = uLUT
      wrfUVWQVAPOR_0000vtrDisplay_1.OSPRayScaleArray = 'QVAPOR'
      wrfUVWQVAPOR_0000vtrDisplay_1.OSPRayScaleFunction = 'PiecewiseFunction'
      wrfUVWQVAPOR_0000vtrDisplay_1.SelectOrientationVectors = 'None'
      wrfUVWQVAPOR_0000vtrDisplay_1.ScaleFactor = 66.2
      wrfUVWQVAPOR_0000vtrDisplay_1.SelectScaleArray = 'QVAPOR'
      wrfUVWQVAPOR_0000vtrDisplay_1.GlyphType = 'Arrow'
      wrfUVWQVAPOR_0000vtrDisplay_1.PolarAxes = 'PolarAxesRepresentation'
      wrfUVWQVAPOR_0000vtrDisplay_1.GaussianRadius = 33.1
      wrfUVWQVAPOR_0000vtrDisplay_1.SetScaleArray = ['POINTS', 'QVAPOR']
      wrfUVWQVAPOR_0000vtrDisplay_1.ScaleTransferFunction = 'PiecewiseFunction'
      wrfUVWQVAPOR_0000vtrDisplay_1.OpacityArray = ['POINTS', 'QVAPOR']
      wrfUVWQVAPOR_0000vtrDisplay_1.OpacityTransferFunction = 'PiecewiseFunction'

      # show color legend
      wrfUVWQVAPOR_0000vtrDisplay_1.SetScalarBarVisibility(renderView2, True)

      # setup the color legend parameters for each legend in this view

      # get color legend/bar for uLUT in view renderView2
      uLUTColorBar = GetScalarBar(uLUT, renderView2)
      uLUTColorBar.Title = 'U'
      uLUTColorBar.ComponentTitle = ''

      # ----------------------------------------------------------------
      # setup the visualization in view 'renderView3'
      # ----------------------------------------------------------------

      # show data from wrfUVWQVAPOR_0000vtr
      wrfUVWQVAPOR_0000vtrDisplay_2 = Show(adaptor_V, renderView3)
      # trace defaults for the display properties.
      wrfUVWQVAPOR_0000vtrDisplay_2.Representation = 'Points'
      wrfUVWQVAPOR_0000vtrDisplay_2.ColorArrayName = ['POINTS', 'V']
      wrfUVWQVAPOR_0000vtrDisplay_2.LookupTable = vLUT
      wrfUVWQVAPOR_0000vtrDisplay_2.OSPRayScaleArray = 'QVAPOR'
      wrfUVWQVAPOR_0000vtrDisplay_2.OSPRayScaleFunction = 'PiecewiseFunction'
      wrfUVWQVAPOR_0000vtrDisplay_2.SelectOrientationVectors = 'None'
      wrfUVWQVAPOR_0000vtrDisplay_2.ScaleFactor = 66.2
      wrfUVWQVAPOR_0000vtrDisplay_2.SelectScaleArray = 'QVAPOR'
      wrfUVWQVAPOR_0000vtrDisplay_2.GlyphType = 'Arrow'
      wrfUVWQVAPOR_0000vtrDisplay_2.PolarAxes = 'PolarAxesRepresentation'
      wrfUVWQVAPOR_0000vtrDisplay_2.GaussianRadius = 33.1
      wrfUVWQVAPOR_0000vtrDisplay_2.SetScaleArray = ['POINTS', 'QVAPOR']
      wrfUVWQVAPOR_0000vtrDisplay_2.ScaleTransferFunction = 'PiecewiseFunction'
      wrfUVWQVAPOR_0000vtrDisplay_2.OpacityArray = ['POINTS', 'QVAPOR']
      wrfUVWQVAPOR_0000vtrDisplay_2.OpacityTransferFunction = 'PiecewiseFunction'

      # show color legend
      wrfUVWQVAPOR_0000vtrDisplay_2.SetScalarBarVisibility(renderView3, True)

      # setup the color legend parameters for each legend in this view

      # get color legend/bar for vLUT in view renderView3
      vLUTColorBar = GetScalarBar(vLUT, renderView3)
      vLUTColorBar.Title = 'V'
      vLUTColorBar.ComponentTitle = ''

      # ----------------------------------------------------------------
      # setup the visualization in view 'renderView4'
      # ----------------------------------------------------------------

      # show data from wrfUVWQVAPOR_0000vtr
      wrfUVWQVAPOR_0000vtrDisplay_3 = Show(adaptor_W, renderView4)
      # trace defaults for the display properties.
      wrfUVWQVAPOR_0000vtrDisplay_3.Representation = 'Surface'
      wrfUVWQVAPOR_0000vtrDisplay_3.ColorArrayName = ['POINTS', 'W']
      wrfUVWQVAPOR_0000vtrDisplay_3.LookupTable = wLUT
      wrfUVWQVAPOR_0000vtrDisplay_3.OSPRayScaleArray = 'QVAPOR'
      wrfUVWQVAPOR_0000vtrDisplay_3.OSPRayScaleFunction = 'PiecewiseFunction'
      wrfUVWQVAPOR_0000vtrDisplay_3.SelectOrientationVectors = 'None'
      wrfUVWQVAPOR_0000vtrDisplay_3.ScaleFactor = 66.2
      wrfUVWQVAPOR_0000vtrDisplay_3.SelectScaleArray = 'QVAPOR'
      wrfUVWQVAPOR_0000vtrDisplay_3.GlyphType = 'Arrow'
      wrfUVWQVAPOR_0000vtrDisplay_3.PolarAxes = 'PolarAxesRepresentation'
      wrfUVWQVAPOR_0000vtrDisplay_3.GaussianRadius = 33.1
      wrfUVWQVAPOR_0000vtrDisplay_3.SetScaleArray = ['POINTS', 'QVAPOR']
      wrfUVWQVAPOR_0000vtrDisplay_3.ScaleTransferFunction = 'PiecewiseFunction'
      wrfUVWQVAPOR_0000vtrDisplay_3.OpacityArray = ['POINTS', 'QVAPOR']
      wrfUVWQVAPOR_0000vtrDisplay_3.OpacityTransferFunction = 'PiecewiseFunction'

      # show color legend
      wrfUVWQVAPOR_0000vtrDisplay_3.SetScalarBarVisibility(renderView4, True)

      # setup the color legend parameters for each legend in this view

      # get color legend/bar for wLUT in view renderView4
      wLUTColorBar = GetScalarBar(wLUT, renderView4)
      wLUTColorBar.Title = 'W'
      wLUTColorBar.ComponentTitle = ''

      # ----------------------------------------------------------------
      # finally, restore active source
      SetActiveSource(adaptor_QVAPOR)
      # ----------------------------------------------------------------
    return Pipeline()

  class CoProcessor(coprocessing.CoProcessor):
    def CreatePipeline(self, datadescription):
      self.Pipeline = _CreatePipeline(self, datadescription)

  coprocessor = CoProcessor()
  # these are the frequencies at which the coprocessor updates.
  freqs = { 'QVAPOR': [1]
          , 'U': [1]
          , 'V': [1]
          , 'W': [1]
          }
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
