import math
di = self.GetInput()
ipd = di.GetPointData()
pdi = ipd.GetScalars('P')
pdo = self.GetPolyDataOutput()
newPts = vtk.vtkPoints()
(dimX, dimY, dimZ) = di.GetDimensions()
minP = pdi.GetTuple1(0)
minX = 0
minY = 0
k = 0
for z in range(0, dimZ):
  for y in range(0, dimY):
    for x in range(0, dimX):
      pk = pdi.GetTuple1(k)
      if (pk < minP):
        minP = pk
        minX = x
        minY = y
      k = k + 1
newPts.InsertPoint(0, minX, minY, 0)
pdo.SetPoints(newPts)
