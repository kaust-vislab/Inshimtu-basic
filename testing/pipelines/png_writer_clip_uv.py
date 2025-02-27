from paraview import catalyst
from paraview.simple import *
from paraview.catalyst import get_args, get_execute_params
import time
import os

paraview.compatibility.major = 5
paraview.compatibility.minor = 13

#### import the simple module from the paraview
from paraview.simple import *
#### disable automatic camera reset on 'Show'
paraview.simple._DisableFirstRenderCameraReset()

# Specify the output directory. Ideally, this should be an
# absolute path to avoid confusion.
outputDirectory = "./images"

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
print("executing catalyst_pipeline")
print("===================================")
print("pipeline args={}".format(get_args()))
print("===================================")

# ----------------------------------------------------------------
# setup views used in the visualization
# ----------------------------------------------------------------

# get the material library
materialLibrary1 = GetMaterialLibrary()

# Create a new 'Render View'
renderView1 = CreateView('RenderView')
renderView1.ViewSize = [1346, 1038]
renderView1.AxesGrid = 'Grid Axes 3D Actor'
renderView1.CenterOfRotation = [3.200000047683716, 3.200000047683716, 3.200000047683716]
renderView1.StereoType = 'Crystal Eyes'
renderView1.CameraPosition = [18.088315529859052, 12.240142633070233, 0.0296710011682049]
renderView1.CameraFocalPoint = [3.215504769133173, 3.3499461158150994, 3.6338440351778045]
renderView1.CameraViewUp = [-0.1811840470939434, -0.09377243323918803, -0.9789683712168957]
renderView1.CameraFocalDisk = 1.0
renderView1.CameraParallelScale = 5.542562666811026
renderView1.LegendGrid = 'Legend Grid Actor'
renderView1.PolarGrid = 'Polar Grid Actor'
renderView1.BackEnd = 'OSPRay raycaster'
renderView1.OSPRayMaterialLibrary = materialLibrary1

SetActiveView(None)

# ----------------------------------------------------------------
# setup view layouts
# ----------------------------------------------------------------

# create new layout object 'Layout #1'
layout1 = CreateLayout(name='Layout #1')
layout1.AssignView(0, renderView1)
layout1.SetSize(1346, 1038)

# ----------------------------------------------------------------
# restore active view
SetActiveView(renderView1)
# ----------------------------------------------------------------

# ----------------------------------------------------------------
# setup the data processing pipelines
# ----------------------------------------------------------------

# create a new 'XML Partitioned Image Data Reader'
grayScott_step000010pvti = PVTrivialProducer(registrationName=channel_name)
grayScott_step000010pvti.PointArrayStatus = ['u', 'v']
grayScott_step000010pvti.TimeArray = 'None'

# create a new 'Clip'
clip1 = Clip(registrationName='Clip1', Input=grayScott_step000010pvti)
clip1.ClipType = 'Plane'
clip1.HyperTreeGridClipper = 'Plane'
clip1.Scalars = ['POINTS', 'u']
clip1.Value = 0.5520011223852634

# init the 'Plane' selected for 'ClipType'
clip1.ClipType.Origin = [3.2, 3.2, 3.2]

# init the 'Plane' selected for 'HyperTreeGridClipper'
clip1.HyperTreeGridClipper.Origin = [3.2, 3.2, 3.2]

# create a new 'Resample To Image'
resampleToImage2 = ResampleToImage(registrationName='ResampleToImage2', Input=clip1)
resampleToImage2.SamplingBounds = [0.0, 3.2, 0.0, 6.4, 0.0, 6.4]

# ----------------------------------------------------------------
# setup the visualization in view 'renderView1'
# ----------------------------------------------------------------

# show data from grayScott_step000010pvti
grayScott_step000010pvtiDisplay = Show(grayScott_step000010pvti, renderView1, 'UniformGridRepresentation')

# get 2D transfer function for 'u'
uTF2D = GetTransferFunction2D('u')
uTF2D.ScalarRangeInitialized = 1
uTF2D.Range = [0.10400166362524033, 1.0000004768371582, 0.0, 1.0]

# get color transfer function/color map for 'u'
uLUT = GetColorTransferFunction('u')
uLUT.TransferFunction2D = uTF2D
uLUT.RGBPoints = [0.10400164872407913, 0.231373, 0.298039, 0.752941, 0.5520011223852634, 0.865003, 0.865003, 0.865003, 1.0000005960464478, 0.705882, 0.0156863, 0.14902]
uLUT.ScalarRangeInitialized = 1.0

# get opacity transfer function/opacity map for 'u'
uPWF = GetOpacityTransferFunction('u')
uPWF.Points = [0.10400164872407913, 0.0, 0.5, 0.0, 0.13931195437908173, 0.941964328289032, 0.5, 0.0, 1.0000005960464478, 0.0223214291036129, 0.5, 0.0]
uPWF.ScalarRangeInitialized = 1

# trace defaults for the display properties.
grayScott_step000010pvtiDisplay.Representation = 'Outline'
grayScott_step000010pvtiDisplay.ColorArrayName = ['POINTS', 'u']
grayScott_step000010pvtiDisplay.LookupTable = uLUT
grayScott_step000010pvtiDisplay.SelectNormalArray = 'None'
grayScott_step000010pvtiDisplay.SelectTangentArray = 'None'
grayScott_step000010pvtiDisplay.SelectTCoordArray = 'None'
grayScott_step000010pvtiDisplay.TextureTransform = 'Transform2'
grayScott_step000010pvtiDisplay.OSPRayScaleArray = 'u'
grayScott_step000010pvtiDisplay.OSPRayScaleFunction = 'Piecewise Function'
grayScott_step000010pvtiDisplay.Assembly = ''
grayScott_step000010pvtiDisplay.SelectedBlockSelectors = ['']
grayScott_step000010pvtiDisplay.SelectOrientationVectors = 'None'
grayScott_step000010pvtiDisplay.ScaleFactor = 0.6400000000000001
grayScott_step000010pvtiDisplay.SelectScaleArray = 'None'
grayScott_step000010pvtiDisplay.GlyphType = 'Arrow'
grayScott_step000010pvtiDisplay.GlyphTableIndexArray = 'None'
grayScott_step000010pvtiDisplay.GaussianRadius = 0.032
grayScott_step000010pvtiDisplay.SetScaleArray = ['POINTS', 'u']
grayScott_step000010pvtiDisplay.ScaleTransferFunction = 'Piecewise Function'
grayScott_step000010pvtiDisplay.OpacityArray = ['POINTS', 'u']
grayScott_step000010pvtiDisplay.OpacityTransferFunction = 'Piecewise Function'
grayScott_step000010pvtiDisplay.DataAxesGrid = 'Grid Axes Representation'
grayScott_step000010pvtiDisplay.PolarAxes = 'Polar Axes Representation'
grayScott_step000010pvtiDisplay.ScalarOpacityUnitDistance = 0.17320508075688776
grayScott_step000010pvtiDisplay.ScalarOpacityFunction = uPWF
grayScott_step000010pvtiDisplay.TransferFunction2D = uTF2D
grayScott_step000010pvtiDisplay.OpacityArrayName = ['POINTS', 'u']
grayScott_step000010pvtiDisplay.ColorArray2Name = ['POINTS', 'u']
grayScott_step000010pvtiDisplay.SliceFunction = 'Plane'
grayScott_step000010pvtiDisplay.Slice = 32
grayScott_step000010pvtiDisplay.SelectInputVectors = [None, '']
grayScott_step000010pvtiDisplay.WriteLog = ''

# init the 'Piecewise Function' selected for 'ScaleTransferFunction'
grayScott_step000010pvtiDisplay.ScaleTransferFunction.Points = [0.10400164872407913, 0.0, 0.5, 0.0, 1.0000005960464478, 1.0, 0.5, 0.0]

# init the 'Piecewise Function' selected for 'OpacityTransferFunction'
grayScott_step000010pvtiDisplay.OpacityTransferFunction.Points = [0.10400164872407913, 0.0, 0.5, 0.0, 1.0000005960464478, 1.0, 0.5, 0.0]

# init the 'Plane' selected for 'SliceFunction'
grayScott_step000010pvtiDisplay.SliceFunction.Origin = [3.2, 3.2, 3.2]

# show data from resampleToImage2
resampleToImage2Display = Show(resampleToImage2, renderView1, 'UniformGridRepresentation')

# trace defaults for the display properties.
resampleToImage2Display.Representation = 'Volume'
resampleToImage2Display.ColorArrayName = ['POINTS', 'u']
resampleToImage2Display.LookupTable = uLUT
resampleToImage2Display.SelectNormalArray = 'None'
resampleToImage2Display.SelectTangentArray = 'None'
resampleToImage2Display.SelectTCoordArray = 'None'
resampleToImage2Display.TextureTransform = 'Transform2'
resampleToImage2Display.OSPRayScaleArray = 'u'
resampleToImage2Display.OSPRayScaleFunction = 'Piecewise Function'
resampleToImage2Display.Assembly = ''
resampleToImage2Display.SelectedBlockSelectors = ['']
resampleToImage2Display.SelectOrientationVectors = 'None'
resampleToImage2Display.ScaleFactor = 0.6399993600000001
resampleToImage2Display.SelectScaleArray = 'None'
resampleToImage2Display.GlyphType = 'Arrow'
resampleToImage2Display.GlyphTableIndexArray = 'None'
resampleToImage2Display.GaussianRadius = 0.031999968000000004
resampleToImage2Display.SetScaleArray = ['POINTS', 'u']
resampleToImage2Display.ScaleTransferFunction = 'Piecewise Function'
resampleToImage2Display.OpacityArray = ['POINTS', 'u']
resampleToImage2Display.OpacityTransferFunction = 'Piecewise Function'
resampleToImage2Display.DataAxesGrid = 'Grid Axes Representation'
resampleToImage2Display.PolarAxes = 'Polar Axes Representation'
resampleToImage2Display.ScalarOpacityUnitDistance = 0.0969696
resampleToImage2Display.ScalarOpacityFunction = uPWF
resampleToImage2Display.TransferFunction2D = uTF2D
resampleToImage2Display.OpacityArrayName = ['POINTS', 'u']
resampleToImage2Display.ColorArray2Name = ['POINTS', 'u']
resampleToImage2Display.SliceFunction = 'Plane'
resampleToImage2Display.Slice = 49
resampleToImage2Display.SelectInputVectors = [None, '']
resampleToImage2Display.WriteLog = ''

# init the 'Piecewise Function' selected for 'ScaleTransferFunction'
resampleToImage2Display.ScaleTransferFunction.Points = [0.10400166362524033, 0.0, 0.5, 0.0, 1.0000004768371582, 1.0, 0.5, 0.0]

# init the 'Piecewise Function' selected for 'OpacityTransferFunction'
resampleToImage2Display.OpacityTransferFunction.Points = [0.10400166362524033, 0.0, 0.5, 0.0, 1.0000004768371582, 1.0, 0.5, 0.0]

# init the 'Plane' selected for 'SliceFunction'
resampleToImage2Display.SliceFunction.Origin = [1.6000000000000003, 3.2000000000000006, 3.2000000000000006]

# setup the color legend parameters for each legend in this view

# get color legend/bar for uLUT in view renderView1
uLUTColorBar = GetScalarBar(uLUT, renderView1)
uLUTColorBar.Title = 'u'
uLUTColorBar.ComponentTitle = ''

# set color bar visibility
uLUTColorBar.Visibility = 1

# show color legend
grayScott_step000010pvtiDisplay.SetScalarBarVisibility(renderView1, True)

# show color legend
resampleToImage2Display.SetScalarBarVisibility(renderView1, True)

# ----------------------------------------------------------------
# setup color maps and opacity maps used in the visualization
# note: the Get..() functions create a new object, if needed
# ----------------------------------------------------------------

# ----------------------------------------------------------------
# setup animation scene, tracks and keyframes
# note: the Get..() functions create a new object, if needed
# ----------------------------------------------------------------

# get the time-keeper
timeKeeper1 = GetTimeKeeper()

# initialize the timekeeper

# get time animation track
timeAnimationCue1 = GetTimeTrack()

# initialize the animation track

# get animation scene
animationScene1 = GetAnimationScene()

# initialize the animation scene
animationScene1.ViewModules = renderView1
animationScene1.Cues = timeAnimationCue1
animationScene1.AnimationTime = 0.0
animationScene1.EndTime = 9.0
animationScene1.PlayMode = 'Snap To TimeSteps'

# initialize the animation scene

# ----------------------------------------------------------------
# setup extractors
# ----------------------------------------------------------------

# create extractor
pNG1 = CreateExtractor('PNG', renderView1, registrationName='PNG1')
# trace defaults for the extractor.
pNG1.Trigger = 'Time Step'

# init the 'PNG' selected for 'Writer'
pNG1.Writer.FileName = 'RenderView1_{timestep:06d}{camera}.png'
pNG1.Writer.ImageResolution = [1346, 1038]
pNG1.Writer.Format = 'PNG'

# ----------------------------------------------------------------
# restore active source
SetActiveSource(pNG1)
# ----------------------------------------------------------------

# ------------------------------------------------------------------------------
# Catalyst options
from paraview import catalyst
options = catalyst.Options()
options.GlobalTrigger = 'Time Step'
options.EnableCatalystLive = 1
options.CatalystLiveTrigger = 'Time Step'
options.ExtractsOutputDirectory = outputDirectory

#--------------------------------------------------------------
# Dynamically determine client
clientport = 22222
clienthost = 'localhost'
if 'CATALYST_CLIENT' in os.environ:
  clienthost = os.environ['CATALYST_CLIENT']
options.CatalystLiveURL = str(clienthost) + ":" + str(clientport)


# ------------------------------------------------------------------------------
if __name__ == '__main__':
    from paraview.simple import SaveExtractsUsingCatalystOptions
    # Code for non in-situ environments; if executing in post-processing
    # i.e. non-Catalyst mode, let's generate extracts using Catalyst options
    SaveExtractsUsingCatalystOptions(options)
