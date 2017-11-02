di = inputs[0]
pdi = di.PointData['P']
pdo = self.GetPolyDataOutput()
newPts = vtk.vtkPoints()
(dimX, dimY, dimZ) = di.GetDimensions()
minIdx = pdi.argmin()
i = minIdx % (dimX * dimY)
minX = i % dimX
minY = i // dimX
newPts.InsertPoint(0, minX, minY, 0)
pdo.SetPoints(newPts)
