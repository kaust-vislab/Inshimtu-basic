from paraview import catalyst
from paraview.simple import *
from paraview.catalyst import get_args, get_execute_params
import time
import os

paraview.compatibility.major = 5
paraview.compatibility.minor = 13

#### import the simple module from the paraview
from paraview.simple import *

#---------------------------------------------------------
# Input parameters
#---------------------------------------------------------
# Specify the write frequency
frequency = 1

# Specify the output directory. Ideally, this should be an
# absolute path to avoid confusion.
outputDirectory = "./datasets"

# Specify extractor type to use, if any.
# e.g. for CSV, set extractorType to 'CSV'
extractorType = None

# print values for parameters passed via adaptor (note these don't change,
# and hence must be created as command line params)
args = get_args()
channel_name = "grid"
varsToProcess = []
for arg in args:
    if "channel-name" in arg:
        channel_name = arg.split("=")[1]
    if "variable" in arg:
        varsToProcess.append(arg.split("=")[1])
print("==== executing catalyst_pipeline ====")
print("pipeline args={}".format(get_args()))
print("===================================\n")
print("==== extracting grid and variables of interst ====")
print("channel_name: {}".format(channel_name))
print("varsToProcess: {}".format(varsToProcess))
print("===================================\n")

# registrationName must match the channel name used in the 'CatalystAdaptor'.
producer = TrivialProducer(registrationName=channel_name)
producer.UpdatePipeline()
  
  
# If no vars were passed, try to default to a smart name
if len(varsToProcess) < 1:
    varsToProcess.append(channel_name)

# Process each variable passed independently 
for currentVar in varsToProcess: 
  # create a new 'Pass Arrays' to only process the variable(s) of interest
  passArrays1 = PassArrays(registrationName=currentVar, Input=producer)
  
  # check if we have multiple variables needed per file (checks for a comma delimited string)
  if not currentVar.isalpha():
      varList = [str(var) for var in currentVar.split(',')]
      print("We found a comma in the list the new list is: ", varList)
      
      passArrays1.PointDataArrays = varList
      passArrays1.CellDataArrays = varList
  else:
    passArrays1.PointDataArrays = [currentVar]
    passArrays1.CellDataArrays = [currentVar]

  # example of how to query the pipeline to see what data we are getting
  numPointArrays = passArrays1.GetPointDataInformation().GetNumberOfArrays()
  numCellArrays = passArrays1.GetCellDataInformation().GetNumberOfArrays()
  print("=======================================================")
  print("The catalyst stream contains the following arrays---")
  print("Number of point arrays = {}".format(numPointArrays))
  for i in range(numPointArrays):
      currentArray = passArrays1.GetPointDataInformation().GetArray(i)
      print("Array = {}, name = {}".format(i, currentArray.GetName()))

  print("Number of cell arrays = {}".format(numCellArrays))
  for i in range(numCellArrays):
      currentArray = passArrays1.GetCellDataInformation().GetArray(i)
      print("Array = {}, name = {}".format(i, currentArray.GetName()))
  print("=======================================================")


  def create_extractor(data):
      if extractorType is not None:
          return CreateExtractor(extractorType, data, registrationName=extractorType)

      grid = data.GetClientSideObject().GetOutputDataObject(0)
      if grid.IsA('vtkImageData'):
          return CreateExtractor('VTI', data, registrationName='VTI')
      elif grid.IsA('vtkRectilinearGrid'):
          return CreateExtractor('VTR', data, registrationName='VTR')
      elif grid.IsA('vtkStructuredGrid'):
          return CreateExtractor('VTS', data, registrationName='VTS')
      elif grid.IsA('vtkPolyData'):
          return CreateExtractor('VTP', data, registrationName='VTP')
      elif grid.IsA('vtkUnstructuredGrid'):
          return CreateExtractor('VTU', data, registrationName='VTU')
      elif grid.IsA('vtkUniformGridAMR'):
          return CreateExtractor('VTH', data, registrationName='VTH')
      elif grid.IsA('vtkMultiBlockDataSet'):
          return CreateExtractor('VTM', data, registrationName='VTM')
      elif grid.IsA('vtkPartitionedDataSet'):
          return CreateExtractor('VTPD', data, registrationName='VTPD')
      elif grid.IsA('vtkPartitionedDataSetCollection'):
          return CreateExtractor('VTPC', data, registrationName='VTPC')
      elif  grid.IsA('vtkHyperTreeGrid'):
          return CreateExtractor('HTG', data, registrationName='HTG')
      else:
          raise RuntimeError("Unsupported data type: %s. Check that the adaptor "
                            "is providing channel named %s",
                            grid.GetClassName(), currentVar)


  # Returns extractor type based on data (or you can manually specify
  extractor = create_extractor(passArrays1)

# ------------------------------------------------------------------------------
# Catalyst options
from paraview import catalyst
options = catalyst.Options()
options.ExtractsOutputDirectory = outputDirectory
options.GlobalTrigger.Frequency = frequency
options.EnableCatalystLive = 1
options.CatalystLiveTrigger = 'Time Step'

#--------------------------------------------------------------
# Dynamically determine client
clientport = 22222
clienthost = 'localhost'
if 'SSH_CLIENT' in os.environ:
  clienthost = os.environ['SSH_CLIENT'].split()[0]
if 'INSHIMTU_CLIENT' in os.environ:
  clienthost = os.environ['INSHIMTU_CLIENT']
options.CatalystLiveURL = str(clienthost) + ":" + str(clientport)


def catalyst_execute(info):
    global producer
    producer.UpdatePipeline()

    # get params as example of a parameter changing during the simulation
    params = get_execute_params()
    print("\n===================================")
    print("executing (cycle={}, time={})".format(info.cycle, info.time))
    print("-----")
    print("pipeline parameters:")
    print("\n".join(params))
    print("-----")
    print("bounds:", producer.GetDataInformation().GetBounds())
    #print("v-range:", producer.PointData["v"].GetRange(-1))
    #print("u-range:", producer.PointData["u"].GetRange(-1))
    print("===================================\n")


# ------------------------------------------------------------------------------
if __name__ == '__main__':
    from paraview.simple import SaveExtractsUsingCatalystOptions
    # Code for non in-situ environments; if executing in post-processing
    # i.e. non-Catalyst mode, let's generate extracts using Catalyst options
    SaveExtractsUsingCatalystOptions(options)
