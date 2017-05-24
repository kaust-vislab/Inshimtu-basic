#### import the simple module from the paraview
from paraview.simple import *

"""
To use:
File -> Load State...
  Specify state file: testing/pipelines/cyclone_extract/cyclone_initial.pvsm

Catalyst -> Connect...
Catalyst -> Pause Simulation

Launch Inshimtu:
  testing/launchers/launch_inshimtu_kvl.sh -S CycloneExtract

Extract ProgrammableFilter7 from Catalyst
Extract ProgrammableFIlter2 from Catalyst

Select 'builtin' pipeline (not 'catalyst' part of pipeline)

Tools -> Python Shell
Run Script
  Specify this file: cyclone_setup.py

Catalyst -> Continue
"""

# find source
regionSurfaceobj = FindSource('regionSurface.obj')
slice1 = FindSource('Slice1')
slice2 = FindSource('Slice2')
contour1 = FindSource('Contour1')
programmableFilter2 = FindSource('ProgrammableFilter2')
programmableFilter7 = FindSource('ProgrammableFilter7')
extractProgrammableFilter7 = FindSource('Extract: ProgrammableFilter7')
extractProgrammableFilter2 = FindSource('Extract: ProgrammableFilter2')

print(slice1)
print(slice2)
print(contour1)
print(regionSurfaceobj)
print(programmableFilter2)
print(programmableFilter7)
print(extractProgrammableFilter2)
print(extractProgrammableFilter7)


# Properties modified
slice1.Input = extractProgrammableFilter7
slice2.Input = extractProgrammableFilter7
contour1.Input = extractProgrammableFilter2

