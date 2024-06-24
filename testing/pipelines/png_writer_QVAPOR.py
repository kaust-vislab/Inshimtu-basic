# script-version: 2.0
# Catalyst state generated using paraview version 5.12.0
import paraview
paraview.compatibility.major = 5
paraview.compatibility.minor = 12

#### import the simple module from the paraview
from paraview.simple import *
#### disable automatic camera reset on 'Show'
paraview.simple._DisableFirstRenderCameraReset()

# ----------------------------------------------------------------
# setup views used in the visualization
# ----------------------------------------------------------------

# get the material library
materialLibrary1 = GetMaterialLibrary()

# Create a new 'Render View'
renderView1 = CreateView('RenderView')
renderView1.ViewSize = [1255, 963]
renderView1.AxesGrid = 'Grid Axes 3D Actor'
renderView1.CenterOfRotation = [5.0, 4.5, 1.5]
renderView1.StereoType = 'Crystal Eyes'
renderView1.CameraPosition = [5.0, 4.5, -22.081254638542088]
renderView1.CameraFocalPoint = [5.0, 4.5, 1.5]
renderView1.CameraFocalDisk = 1.0
renderView1.CameraParallelScale = 6.103277807866851
renderView1.LegendGrid = 'Legend Grid Actor'
renderView1.BackEnd = 'OSPRay raycaster'
renderView1.OSPRayMaterialLibrary = materialLibrary1

SetActiveView(None)

# ----------------------------------------------------------------
# setup view layouts
# ----------------------------------------------------------------

# create new layout object 'Layout #1'
layout1 = CreateLayout(name='Layout #1')
layout1.AssignView(0, renderView1)
layout1.SetSize(1255, 963)

# ----------------------------------------------------------------
# restore active view
SetActiveView(renderView1)
# ----------------------------------------------------------------

# ----------------------------------------------------------------
# setup the data processing pipelines
# ----------------------------------------------------------------

# create a new 'XML Partitioned Image Data Reader'
qVAPOR = PVTrivialProducer(registrationName='QVAPOR')
qVAPOR.CellArrayStatus = ['QVAPOR']

# ----------------------------------------------------------------
# setup the visualization in view 'renderView1'
# ----------------------------------------------------------------

# show data from qVAPOR
qVAPORDisplay = Show(qVAPOR, renderView1, 'UniformGridRepresentation')

# get 2D transfer function for 'QVAPOR'
qVAPORTF2D = GetTransferFunction2D('QVAPOR')
qVAPORTF2D.ScalarRangeInitialized = 1
qVAPORTF2D.Range = [1.0, 16.0, 0.0, 1.0]

# get color transfer function/color map for 'QVAPOR'
qVAPORLUT = GetColorTransferFunction('QVAPOR')
qVAPORLUT.TransferFunction2D = qVAPORTF2D
qVAPORLUT.RGBPoints = [1.0, 0.231373, 0.298039, 0.752941, 15.5, 0.865003, 0.865003, 0.865003, 30.0, 0.705882, 0.0156863, 0.14902]
qVAPORLUT.ScalarRangeInitialized = 1.0

# get opacity transfer function/opacity map for 'QVAPOR'
qVAPORPWF = GetOpacityTransferFunction('QVAPOR')
qVAPORPWF.Points = [1.0, 0.0, 0.5, 0.0, 30.0, 1.0, 0.5, 0.0]
qVAPORPWF.ScalarRangeInitialized = 1

# trace defaults for the display properties.
qVAPORDisplay.Representation = 'Surface'
qVAPORDisplay.ColorArrayName = ['CELLS', 'QVAPOR']
qVAPORDisplay.LookupTable = qVAPORLUT
qVAPORDisplay.SelectTCoordArray = 'None'
qVAPORDisplay.SelectNormalArray = 'None'
qVAPORDisplay.SelectTangentArray = 'None'
qVAPORDisplay.OSPRayScaleFunction = 'Piecewise Function'
qVAPORDisplay.Assembly = ''
qVAPORDisplay.SelectOrientationVectors = 'None'
qVAPORDisplay.ScaleFactor = 0.9
qVAPORDisplay.SelectScaleArray = 'QVAPOR'
qVAPORDisplay.GlyphType = 'Arrow'
qVAPORDisplay.GlyphTableIndexArray = 'QVAPOR'
qVAPORDisplay.GaussianRadius = 0.045
qVAPORDisplay.SetScaleArray = [None, '']
qVAPORDisplay.ScaleTransferFunction = 'Piecewise Function'
qVAPORDisplay.OpacityArray = [None, '']
qVAPORDisplay.OpacityTransferFunction = 'Piecewise Function'
qVAPORDisplay.DataAxesGrid = 'Grid Axes Representation'
qVAPORDisplay.PolarAxes = 'Polar Axes Representation'
qVAPORDisplay.ScalarOpacityUnitDistance = 2.328836344308287
qVAPORDisplay.ScalarOpacityFunction = qVAPORPWF
qVAPORDisplay.TransferFunction2D = qVAPORTF2D
qVAPORDisplay.OpacityArrayName = ['CELLS', 'QVAPOR']
qVAPORDisplay.ColorArray2Name = ['CELLS', 'QVAPOR']
qVAPORDisplay.IsosurfaceValues = [8.5]
qVAPORDisplay.SliceFunction = 'Plane'
qVAPORDisplay.Slice = 1
qVAPORDisplay.SelectInputVectors = [None, '']
qVAPORDisplay.WriteLog = ''

# init the 'Plane' selected for 'SliceFunction'
qVAPORDisplay.SliceFunction.Origin = [5.0, 4.5, 1.5]

# setup the color legend parameters for each legend in this view

# get color legend/bar for qVAPORLUT in view renderView1
qVAPORLUTColorBar = GetScalarBar(qVAPORLUT, renderView1)
qVAPORLUTColorBar.Title = 'QVAPOR'
qVAPORLUTColorBar.ComponentTitle = ''

# set color bar visibility
qVAPORLUTColorBar.Visibility = 1

# show color legend
qVAPORDisplay.SetScalarBarVisibility(renderView1, True)

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
animationScene1.AnimationTime = 14.0
animationScene1.EndTime = 14.0
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
pNG1.Writer.ImageResolution = [1255, 963]
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

#--------------------------------------------------------------
# Dynamically determine client
clientport = 11111
clienthost = 'localhost'
options.CatalystLiveURL = "localhost:22222"
if 'CATALYST_CLIENT' in os.environ:
  clienthost = os.environ['CATALYST_CLIENT']
options.CatalystLiveURL = str(clienthost) + ":" + str(clientport)


# ------------------------------------------------------------------------------
if __name__ == '__main__':
    from paraview.simple import SaveExtractsUsingCatalystOptions
    # Code for non in-situ environments; if executing in post-processing
    # i.e. non-Catalyst mode, let's generate extracts using Catalyst options
    SaveExtractsUsingCatalystOptions(options)
