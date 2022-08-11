
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
      # setup the data processing pipelines
      # ----------------------------------------------------------------

      #### disable automatic camera reset on 'Show'
      paraview.simple._DisableFirstRenderCameraReset()

      # create a new 'XML Partitioned Image Data Reader'
      # create a producer from a simulation input
      datafile_QICE = coprocessor.CreateProducer(datadescription, 'QICE')

      # create a new 'Contour'
      contour_QICE = Contour(Input=datafile_QICE)
      contour_QICE.ContourBy = ['POINTS', 'QICE']
      contour_QICE.ComputeScalars = 1
      contour_QICE.Isosurfaces = [5e-05]
      contour_QICE.PointMergeMethod = 'Uniform Binning'

      # create a new 'Decimate'
      decimate_QICE = Decimate(Input=contour_QICE)


      # create a new 'XML Partitioned Image Data Reader'
      # create a producer from a simulation input
      datafile_P = coprocessor.CreateProducer(datadescription, 'P')

      # create a new 'Contour'
      contour_P = Contour(Input=datafile_P)
      contour_P.ContourBy = ['POINTS', 'P']
      contour_P.ComputeScalars = 1
      contour_P.Isosurfaces = [15.0]
      contour_P.PointMergeMethod = 'Uniform Binning'

      # create a new 'Decimate'
      decimate_P = Decimate(Input=contour_P)


      # ----------------------------------------------------------------
      # finally, restore active source
      SetActiveSource(decimate_P)
      # ----------------------------------------------------------------

    return Pipeline()

  class CoProcessor(coprocessing.CoProcessor):
    def CreatePipeline(self, datadescription):
      self.Pipeline = _CreatePipeline(self, datadescription)

  coprocessor = CoProcessor()
  # these are the frequencies at which the coprocessor updates.
  freqs = { 'QICE'   : []
          , 'P'      : [] }
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
    coprocessor.DoLiveVisualization(datadescription, "glendon-00.desktop.vis.kaust.edu.sa", 22222)
