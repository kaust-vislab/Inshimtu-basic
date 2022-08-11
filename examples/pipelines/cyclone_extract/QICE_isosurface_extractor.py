
from paraview.simple import *
from paraview import coprocessing


#--------------------------------------------------------------
# Code generated from cpstate.py to create the CoProcessor.
# ParaView 5.1.2 64 bits


# ----------------------- CoProcessor definition -----------------------

def CreateCoProcessor():
  def _CreatePipeline(coprocessor, datadescription):
    class Pipeline:
      # state file generated using paraview version 5.1.2

      # ----------------------------------------------------------------
      # setup views used in the visualization
      # ----------------------------------------------------------------

      #### disable automatic camera reset on 'Show'
      paraview.simple._DisableFirstRenderCameraReset()

      # Create a new 'Render View'
      renderView1 = CreateView('RenderView')
      renderView1.ViewSize = [1543, 1453]
      renderView1.InteractionMode = '2D'
      renderView1.AxesGrid = 'GridAxes3DActor'
      renderView1.OrientationAxesVisibility = 0
      renderView1.CenterOfRotation = [549.0, 499.0, 0.0]
      renderView1.StereoType = 0
      renderView1.CameraPosition = [549.0, 499.0, 2000.0]
      renderView1.CameraFocalPoint = [549.0, 499.0, 0.0]
      renderView1.CameraParallelScale = 741.8908275480968
      renderView1.Background = [0.32, 0.34, 0.43]

      # ----------------------------------------------------------------
      # setup the data processing pipelines
      # ----------------------------------------------------------------

      ### extractQRAIN = coprocessor.CreateProducer( datadescription, "QRAIN" )

      ### # create a new 'Contour'
      ### contour1 = Contour(Input=extractQRAIN)
      ### contour1.ContourBy = ['POINTS', 'QRAIN']
      ### contour1.Isosurfaces = [1e-05]
      ### contour1.PointMergeMethod = 'Uniform Binning'

      ### # show data from contour1
      ### contour1Display = Show(contour1, renderView1)
      ### # trace defaults for the display properties.
      ### contour1Display.ColorArrayName = [None, '']
      ### contour1Display.OSPRayScaleFunction = 'PiecewiseFunction'
      ### contour1Display.GlyphType = 'Arrow'

      # create a new 'NetCDF Reader'
      # create a producer from a simulation input
      #qice = coprocessor.CreateProducer(datadescription, 'input')
      extractQICE = coprocessor.CreateProducer( datadescription, "QICE" )

      # create a new 'Contour'
      contour2 = Contour(Input=extractQICE)
      contour2.ContourBy = ['POINTS', 'QICE']
      contour2.Isosurfaces = [1e-05]
      contour2.PointMergeMethod = 'Uniform Binning'

      # show data from contour1
      contour2Display = Show(contour2, renderView1)
      # trace defaults for the display properties.
      contour2Display.ColorArrayName = [None, '']
      contour2Display.OSPRayScaleFunction = 'PiecewiseFunction'
      contour2Display.GlyphType = 'Arrow'
      
      # create a new 'Wavefront OBJ Reader'
      extractedSurfaceTextureSimpleobj = WavefrontOBJReader(FileName='/var/remote/projects/kaust/earthenvironscience/hari/tom_playground/ExtractedSurfaceTextureSimple.obj')
      
      # show data in view
      extractedSurfaceTextureSimpleobjDisplay = Show(extractedSurfaceTextureSimpleobj, renderView1)
      # trace defaults for the display properties.
      extractedSurfaceTextureSimpleobjDisplay.ColorArrayName = [None, '']
      extractedSurfaceTextureSimpleobjDisplay.OSPRayScaleArray = 'Normals'
      extractedSurfaceTextureSimpleobjDisplay.OSPRayScaleFunction = 'PiecewiseFunction'
      extractedSurfaceTextureSimpleobjDisplay.GlyphType = 'Arrow'
      extractedSurfaceTextureSimpleobjDisplay.Orientation = [90.0, 0.0, 0.0]

      extractedSurfaceTextureSimpleobjDisplay = GetDisplayProperties(extractedSurfaceTextureSimpleobj, view=renderView1)
      extractedSurfaceTextureSimpleobjDisplay.SetRepresentationType('Points')
      extractedSurfaceTextureSimpleobjDisplay.AmbientColor = [0.0, 0.0, 0.0]
      
      # register the view with coprocessor
      # and provide it with information such as the filename to use,
      # how frequently to write the images, etc.
      coprocessor.RegisterView(renderView1,
          filename='/home/theusst/temp/inshimtu/image_%t.png', freq=1, fittoscreen=0, magnification=1, width=1543, height=1453, cinema={})
      renderView1.ViewTime = datadescription.GetTime()

      # ----------------------------------------------------------------
      # finally, restore active source
      SetActiveSource(contour2)
      # ----------------------------------------------------------------
    return Pipeline()

  class CoProcessor(coprocessing.CoProcessor):
    def CreatePipeline(self, datadescription):
      self.Pipeline = _CreatePipeline(self, datadescription)

  coprocessor = CoProcessor()
  # these are the frequencies at which the coprocessor updates.
  freqs = {'input': [1]
          , 'QICE': [1]
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


# ---------------------- Data Selection method ----------------------

def RequestDataDescription(datadescription):
    "Callback to populate the request for current timestep"
    global coprocessor
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
    coprocessor.DoLiveVisualization(datadescription, "localhost", 22222)
