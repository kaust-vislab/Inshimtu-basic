from math import sqrt
diU = inputs[0]
diV = inputs[1]
diW = inputs[2]
idiU = diU.PointData['U']
idiV = diV.PointData['V']
idiW = diW.PointData['W']
ido = self.GetImageDataOutput()
[minX, maxX, minY, maxY, minZ, maxZ] = diU.GetExtent()
(dimX, dimY, dimZ) = diU.GetDimensions()
ido.SetExtent([0, maxX - minX, 0, maxY - minY, 0, maxZ - minZ])
ca = vtk.vtkFloatArray()
ca.SetName('vel')
ca.SetNumberOfComponents(1)
ca.SetNumberOfTuples(dimX*dimY*dimZ)
for k in range(0, ca.GetNumberOfTuples()):
  ca.SetValue(k, sqrt(idiU[k]*idiU[k] + idiV[k]*idiV[k] +idiW[k]*idiW[k]))
ido.GetPointData().AddArray(ca)
