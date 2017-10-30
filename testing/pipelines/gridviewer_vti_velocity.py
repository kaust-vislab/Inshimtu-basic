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

      adaptor_input = coprocessor.CreateProducer( datadescription, "velocity" )

    return Pipeline()

  class CoProcessor(coprocessing.CoProcessor):
    def CreatePipeline(self, datadescription):
      self.Pipeline = _CreatePipeline(self, datadescription)

  coprocessor = CoProcessor()
  freqs = { 'velocity': [1]
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

    if datadescription.GetForceOutput():
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
    global clienthost
    global clientport

    # Update the coprocessor by providing it the newly generated simulation data.
    # If the pipeline hasn't been setup yet, this will setup the pipeline.
    coprocessor.UpdateProducers(datadescription)

    # Live Visualization, if enabled.
    coprocessor.DoLiveVisualization(datadescription, clienthost, clientport)

