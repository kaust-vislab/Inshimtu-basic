import vtk
from math import sqrt
doU = self.GetInputDataObject(0,0)
doV = self.GetInputDataObject(0,1)
doW = self.GetInputDataObject(0,2)
diU = doU.GetPointData()
diV = doV.GetPointData()
diW = doW.GetPointData()
idiU = diU.GetScalars('U')
idiV = diV.GetScalars('V')
idiW = diW.GetScalars('W')
ido = self.GetImageDataOutput()
[minX, maxX, minY, maxY, minZ, maxZ] = doU.GetExtent()
(dimX, dimY, dimZ) = doU.GetDimensions()
ido.SetExtent([0, maxX - minX, 0, maxY - minY, 0, maxZ - minZ])
ca = vtk.vtkFloatArray()
ca.SetName('vel')
ca.SetNumberOfComponents(1)
ca.SetNumberOfTuples(dimX*dimY*dimZ)
for k in range(0, ca.GetNumberOfTuples()):
  kU = idiU.GetTuple1(k)
  kV = idiV.GetTuple1(k)
  kW = idiW.GetTuple1(k)
  ca.SetValue(k, sqrt(kU*kU + kV*kV +kW*kW))
ido.GetPointData().AddArray(ca)

