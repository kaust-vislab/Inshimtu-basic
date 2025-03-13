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

#---------------------------------------------------------
# Input parameters
#---------------------------------------------------------
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


# ----------------------------------------------------------------
# setup views used in the visualization
# ----------------------------------------------------------------

# get the material library
materialLibrary1 = GetMaterialLibrary()

# Create a new 'Render View'
renderView1 = CreateView('RenderView')
renderView1.ViewSize = [1696, 920]
renderView1.AxesGrid = 'Grid Axes 3D Actor'
renderView1.CenterOfRotation = [51.20000076293945, 51.20000076293945, 51.20000076293945]
renderView1.StereoType = 'Crystal Eyes'
renderView1.CameraPosition = [257.2454326280291, 217.3042441810464, 37.66902590632296]
renderView1.CameraFocalPoint = [5.676530059607163, -14.411659261399665, 58.18726362519262]
renderView1.CameraViewUp = [-0.040437026963373854, -0.04448734939431682, -0.9981912254644556]
renderView1.CameraFocalDisk = 1.0
renderView1.CameraParallelScale = 88.68100266897642
renderView1.LegendGrid = 'Legend Grid Actor'
renderView1.PolarGrid = 'Polar Grid Actor'
renderView1.UseColorPaletteForBackground = 0
renderView1.BackgroundColorMode = 'Gradient'
renderView1.Background2 = [0.42745098039215684, 0.5450980392156862, 0.6352941176470588]
renderView1.BackEnd = 'OSPRay raycaster'
renderView1.OSPRayMaterialLibrary = materialLibrary1

SetActiveView(None)

# ----------------------------------------------------------------
# setup view layouts
# ----------------------------------------------------------------

# create new layout object 'Layout #1'
layout1 = CreateLayout(name='Layout #1')
layout1.AssignView(0, renderView1)
layout1.SetSize(1696, 920)

# ----------------------------------------------------------------
# restore active view
SetActiveView(renderView1)
# ----------------------------------------------------------------

# ----------------------------------------------------------------
# setup the data processing pipelines
# ----------------------------------------------------------------

# create a new 'XML Partitioned Image Data Reader'
#grayScott_step000200pvti = XMLPartitionedImageDataReader(registrationName='grayScott_step-000200.pvti*', FileName=['/home/kressjm/Downloads/grayScott_step-000200.pvti', '/home/kressjm/Downloads/grayScott_step-000400.pvti'])
grayScott_step000200pvti = PVTrivialProducer(registrationName=channel_name)
grayScott_step000200pvti.PointArrayStatus = ['u']
grayScott_step000200pvti.TimeArray = 'None'

# create a new 'Clip'
clip1 = Clip(registrationName='Clip1', Input=grayScott_step000200pvti)
clip1.ClipType = 'Plane'
clip1.HyperTreeGridClipper = 'Plane'
clip1.Scalars = ['POINTS', 'u']
clip1.Value = 0.5777203142642975

# init the 'Plane' selected for 'ClipType'
clip1.ClipType.Origin = [51.2, 51.2, 51.2]

# init the 'Plane' selected for 'HyperTreeGridClipper'
clip1.HyperTreeGridClipper.Origin = [51.2, 51.2, 51.2]

# create a new 'Clip'
clip2 = Clip(registrationName='Clip2', Input=grayScott_step000200pvti)
clip2.ClipType = 'Plane'
clip2.HyperTreeGridClipper = 'Plane'
clip2.Scalars = ['POINTS', 'u']
clip2.Value = 0.5777203142642975
clip2.Invert = 0

# init the 'Plane' selected for 'ClipType'
clip2.ClipType.Origin = [51.2, 51.2, 51.2]

# init the 'Plane' selected for 'HyperTreeGridClipper'
clip2.HyperTreeGridClipper.Origin = [51.2, 51.2, 51.2]

# create a new 'Transform'
transform1 = Transform(registrationName='Transform1', Input=clip2)
transform1.Transform = 'Transform'

# init the 'Transform' selected for 'Transform'
transform1.Transform.Translate = [50.0, 50.0, 0.0]
transform1.Transform.Rotate = [0.0, 0.0, 270.0]

# create a new 'Contour'
contour1 = Contour(registrationName='Contour1', Input=transform1)
contour1.ContourBy = ['POINTS', 'u']
contour1.Isosurfaces = [0.5777203142642975, 0.15543991327285767, 0.24928000238206652, 0.34312009149127537, 0.43696018060048425, 0.5308002697096931, 0.6246403588189019, 0.7184804479281108, 0.8123205370373197, 0.9061606261465285, 1.0000007152557373]
contour1.PointMergeMethod = 'Uniform Binning'

# ----------------------------------------------------------------
# setup the visualization in view 'renderView1'
# ----------------------------------------------------------------

# show data from grayScott_step000200pvti
grayScott_step000200pvtiDisplay = Show(grayScott_step000200pvti, renderView1, 'UniformGridRepresentation')

# get 2D transfer function for 'u'
uTF2D = GetTransferFunction2D('u')
uTF2D.ScalarRangeInitialized = 1
uTF2D.Range = [0.15543991327285767, 1.0000007152557373, 0.0, 1.0]

# get color transfer function/color map for 'u'
uLUT = GetColorTransferFunction('u')
uLUT.TransferFunction2D = uTF2D
uLUT.RGBPoints = [0.15543991327285767, 0.278431372549, 0.278431372549, 0.858823529412, 0.27621210795640944, 0.0, 0.0, 0.360784313725, 0.39613974183797834, 0.0, 1.0, 1.0, 0.517756497323513, 0.0, 0.501960784314, 0.0, 0.6376841312050818, 1.0, 1.0, 0.0, 0.7584563258886337, 1.0, 0.380392156863, 0.0, 0.8792285205721855, 0.419607843137, 0.0, 0.0, 1.0000007152557373, 0.878431372549, 0.301960784314, 0.301960784314]
uLUT.ColorSpace = 'RGB'
uLUT.ScalarRangeInitialized = 1.0

# get opacity transfer function/opacity map for 'u'
uPWF = GetOpacityTransferFunction('u')
uPWF.Points = [0.15543991327285767, 0.0, 0.5, 0.0, 1.0000007152557373, 1.0, 0.5, 0.0]
uPWF.ScalarRangeInitialized = 1

# trace defaults for the display properties.
grayScott_step000200pvtiDisplay.Representation = 'Outline'
grayScott_step000200pvtiDisplay.ColorArrayName = ['POINTS', 'u']
grayScott_step000200pvtiDisplay.LookupTable = uLUT
grayScott_step000200pvtiDisplay.SelectNormalArray = 'None'
grayScott_step000200pvtiDisplay.SelectTangentArray = 'None'
grayScott_step000200pvtiDisplay.SelectTCoordArray = 'None'
grayScott_step000200pvtiDisplay.TextureTransform = 'Transform2'
grayScott_step000200pvtiDisplay.OSPRayScaleArray = 'u'
grayScott_step000200pvtiDisplay.OSPRayScaleFunction = 'Piecewise Function'
grayScott_step000200pvtiDisplay.Assembly = ''
grayScott_step000200pvtiDisplay.SelectedBlockSelectors = ['']
grayScott_step000200pvtiDisplay.SelectOrientationVectors = 'None'
grayScott_step000200pvtiDisplay.ScaleFactor = 10.240000000000002
grayScott_step000200pvtiDisplay.SelectScaleArray = 'None'
grayScott_step000200pvtiDisplay.GlyphType = 'Arrow'
grayScott_step000200pvtiDisplay.GlyphTableIndexArray = 'None'
grayScott_step000200pvtiDisplay.GaussianRadius = 0.512
grayScott_step000200pvtiDisplay.SetScaleArray = ['POINTS', 'u']
grayScott_step000200pvtiDisplay.ScaleTransferFunction = 'Piecewise Function'
grayScott_step000200pvtiDisplay.OpacityArray = ['POINTS', 'u']
grayScott_step000200pvtiDisplay.OpacityTransferFunction = 'Piecewise Function'
grayScott_step000200pvtiDisplay.DataAxesGrid = 'Grid Axes Representation'
grayScott_step000200pvtiDisplay.PolarAxes = 'Polar Axes Representation'
grayScott_step000200pvtiDisplay.ScalarOpacityUnitDistance = 0.17320508075688776
grayScott_step000200pvtiDisplay.ScalarOpacityFunction = uPWF
grayScott_step000200pvtiDisplay.TransferFunction2D = uTF2D
grayScott_step000200pvtiDisplay.OpacityArrayName = ['POINTS', 'u']
grayScott_step000200pvtiDisplay.ColorArray2Name = ['POINTS', 'u']
grayScott_step000200pvtiDisplay.SliceFunction = 'Plane'
grayScott_step000200pvtiDisplay.Slice = 512
grayScott_step000200pvtiDisplay.SelectInputVectors = [None, '']
grayScott_step000200pvtiDisplay.WriteLog = ''

# init the 'Piecewise Function' selected for 'ScaleTransferFunction'
grayScott_step000200pvtiDisplay.ScaleTransferFunction.Points = [0.15543991327285767, 0.0, 0.5, 0.0, 1.0000007152557373, 1.0, 0.5, 0.0]

# init the 'Piecewise Function' selected for 'OpacityTransferFunction'
grayScott_step000200pvtiDisplay.OpacityTransferFunction.Points = [0.15543991327285767, 0.0, 0.5, 0.0, 1.0000007152557373, 1.0, 0.5, 0.0]

# init the 'Plane' selected for 'SliceFunction'
grayScott_step000200pvtiDisplay.SliceFunction.Origin = [51.2, 51.2, 51.2]

# show data from clip1
clip1Display = Show(clip1, renderView1, 'UnstructuredGridRepresentation')

# trace defaults for the display properties.
clip1Display.Representation = 'Surface'
clip1Display.ColorArrayName = ['POINTS', 'u']
clip1Display.LookupTable = uLUT
clip1Display.SelectNormalArray = 'None'
clip1Display.SelectTangentArray = 'None'
clip1Display.SelectTCoordArray = 'None'
clip1Display.TextureTransform = 'Transform2'
clip1Display.OSPRayScaleArray = 'u'
clip1Display.OSPRayScaleFunction = 'Piecewise Function'
clip1Display.Assembly = ''
clip1Display.SelectedBlockSelectors = ['']
clip1Display.SelectOrientationVectors = 'None'
clip1Display.ScaleFactor = 10.240000000000002
clip1Display.SelectScaleArray = 'None'
clip1Display.GlyphType = 'Arrow'
clip1Display.GlyphTableIndexArray = 'None'
clip1Display.GaussianRadius = 0.512
clip1Display.SetScaleArray = ['POINTS', 'u']
clip1Display.ScaleTransferFunction = 'Piecewise Function'
clip1Display.OpacityArray = ['POINTS', 'u']
clip1Display.OpacityTransferFunction = 'Piecewise Function'
clip1Display.DataAxesGrid = 'Grid Axes Representation'
clip1Display.PolarAxes = 'Polar Axes Representation'
clip1Display.ScalarOpacityFunction = uPWF
clip1Display.ScalarOpacityUnitDistance = 0.18898815748423098
clip1Display.OpacityArrayName = ['POINTS', 'u']
clip1Display.SelectInputVectors = [None, '']
clip1Display.WriteLog = ''

# init the 'Piecewise Function' selected for 'ScaleTransferFunction'
clip1Display.ScaleTransferFunction.Points = [0.15544012188911438, 0.0, 0.5, 0.0, 1.0000007152557373, 1.0, 0.5, 0.0]

# init the 'Piecewise Function' selected for 'OpacityTransferFunction'
clip1Display.OpacityTransferFunction.Points = [0.15544012188911438, 0.0, 0.5, 0.0, 1.0000007152557373, 1.0, 0.5, 0.0]

# show data from contour1
contour1Display = Show(contour1, renderView1, 'GeometryRepresentation')

# trace defaults for the display properties.
contour1Display.Representation = 'Surface'
contour1Display.ColorArrayName = ['POINTS', 'u']
contour1Display.LookupTable = uLUT
contour1Display.SelectNormalArray = 'Normals'
contour1Display.SelectTangentArray = 'None'
contour1Display.SelectTCoordArray = 'None'
contour1Display.TextureTransform = 'Transform2'
contour1Display.OSPRayScaleArray = 'u'
contour1Display.OSPRayScaleFunction = 'Piecewise Function'
contour1Display.Assembly = ''
contour1Display.SelectedBlockSelectors = ['']
contour1Display.SelectOrientationVectors = 'None'
contour1Display.ScaleFactor = 9.700000000000003
contour1Display.SelectScaleArray = 'u'
contour1Display.GlyphType = 'Arrow'
contour1Display.GlyphTableIndexArray = 'u'
contour1Display.GaussianRadius = 0.4850000000000001
contour1Display.SetScaleArray = ['POINTS', 'u']
contour1Display.ScaleTransferFunction = 'Piecewise Function'
contour1Display.OpacityArray = ['POINTS', 'u']
contour1Display.OpacityTransferFunction = 'Piecewise Function'
contour1Display.DataAxesGrid = 'Grid Axes Representation'
contour1Display.PolarAxes = 'Polar Axes Representation'
contour1Display.SelectInputVectors = ['POINTS', 'Normals']
contour1Display.WriteLog = ''

# init the 'Piecewise Function' selected for 'ScaleTransferFunction'
contour1Display.ScaleTransferFunction.Points = [0.24928000569343567, 0.0, 0.5, 0.0, 1.0000007152557373, 1.0, 0.5, 0.0]

# init the 'Piecewise Function' selected for 'OpacityTransferFunction'
contour1Display.OpacityTransferFunction.Points = [0.24928000569343567, 0.0, 0.5, 0.0, 1.0000007152557373, 1.0, 0.5, 0.0]

# setup the color legend parameters for each legend in this view

# get color legend/bar for uLUT in view renderView1
uLUTColorBar = GetScalarBar(uLUT, renderView1)
uLUTColorBar.Orientation = 'Horizontal'
uLUTColorBar.WindowLocation = 'Any Location'
uLUTColorBar.Position = [0.570353773584906, 0.024999999999999994]
uLUTColorBar.Title = 'u'
uLUTColorBar.ComponentTitle = ''
uLUTColorBar.ScalarBarLength = 0.32999999999999974

# set color bar visibility
uLUTColorBar.Visibility = 1

# show color legend
grayScott_step000200pvtiDisplay.SetScalarBarVisibility(renderView1, True)

# show color legend
clip1Display.SetScalarBarVisibility(renderView1, True)

# show color legend
contour1Display.SetScalarBarVisibility(renderView1, True)

# ----------------------------------------------------------------
# setup color maps and opacity maps used in the visualization
# note: the Get..() functions create a new object, if needed
# ----------------------------------------------------------------

# ----------------------------------------------------------------
# setup animation scene, tracks and keyframes
# note: the Get..() functions create a new object, if needed
# ----------------------------------------------------------------

# get time animation track
timeAnimationCue1 = GetTimeTrack()

# initialize the animation scene

# get the time-keeper
timeKeeper1 = GetTimeKeeper()

# initialize the timekeeper

# initialize the animation track

# get animation scene
animationScene1 = GetAnimationScene()

# initialize the animation scene
animationScene1.ViewModules = renderView1
animationScene1.Cues = timeAnimationCue1
animationScene1.AnimationTime = 0.0
animationScene1.PlayMode = 'Snap To TimeSteps'

# ----------------------------------------------------------------
# setup extractors
# ----------------------------------------------------------------

# create extractor
pNG1 = CreateExtractor('PNG', renderView1, registrationName='PNG1')
# trace defaults for the extractor.
pNG1.Trigger = 'Time Step'

# init the 'PNG' selected for 'Writer'
pNG1.Writer.FileName = 'RenderView1_{timestep:06d}{camera}.png'
pNG1.Writer.ImageResolution = [1696, 920]
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

# ------------------------------------------------------------------------------
if __name__ == '__main__':
    from paraview.simple import SaveExtractsUsingCatalystOptions
    # Code for non in-situ environments; if executing in post-processing
    # i.e. non-Catalyst mode, let's generate extracts using Catalyst options
    SaveExtractsUsingCatalystOptions(options)
