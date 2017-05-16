from paraview.simple import *

from paraview import coprocessing

import os

#--------------------------------------------------------------
# Code generated from cpstate.py to create the CoProcessor.


# ----------------------- CoProcessor definition -----------------------

def CreateCoProcessor():
  def _CreatePipeline(coprocessor, datadescription):
    class Pipeline:
      # NOTE: only include producers for variables that belong to a single description on the simulation side

      #adaptor_input = coprocessor.CreateProducer( datadescription, "input" )
      adaptor_QVAPOR = coprocessor.CreateProducer( datadescription, "QVAPOR" )
      adaptor_U = coprocessor.CreateProducer( datadescription, "U" )
      adaptor_V = coprocessor.CreateProducer( datadescription, "V" )
      adaptor_W = coprocessor.CreateProducer( datadescription, "W" )

    return Pipeline()

  class CoProcessor(coprocessing.CoProcessor):
    def CreatePipeline(self, datadescription):
      self.Pipeline = _CreatePipeline(self, datadescription)

  coprocessor = CoProcessor()
  freqs = { 'input': [1]
          , 'QVAPOR': [1]
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
coprocessor.EnableLiveVisualization(True, frequency = 1)


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

    # setup requests for all inputs based on the requirements of the pipeline.
    coprocessor.LoadRequestedData(datadescription)

# ------------------------ Processing method ------------------------

def DoCoProcessing(datadescription):
    "Callback to do co-processing for current timestep"
    global coprocessor

    # Update the coprocessor by providing it the newly generated simulation data.
    # If the pipeline hasn't been setup yet, this will setup the pipeline.
    coprocessor.UpdateProducers(datadescription)

    # Dynamically determine client
    clienthost = 'localhost'
    if 'SSH_CLIENT' in os.environ:
      clienthost = os.environ['SSH_CLIENT'].split()[0]

    # Live Visualization, if enabled.
    coprocessor.DoLiveVisualization(datadescription, clienthost, 22222)

