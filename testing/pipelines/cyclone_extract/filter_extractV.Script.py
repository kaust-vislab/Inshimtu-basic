import vtk
di = inputs[0]
gi = self.GetInput()
ci = inputs[1]
idi = di.PointData['V']
ido = self.GetImageDataOutput()
diExt = di.GetExtent()
offs = 64
ctrs = ci.GetPoints()
ctrX = int(ctrs[0][0])
ctrY = int(ctrs[0][1])
oext = [ int(max([int(ctrX-offs),int(diExt[0])])), int(min([int(ctrX+offs),int(diExt[1])]))
       , int(max([int(ctrY-offs),int(diExt[2])])), int(min([int(ctrY+offs),int(diExt[3])]))
       , int(diExt[4]), int(diExt[5])]
voi = vtk.vtkExtractVOI()
voi.SetVOI(oext[0], oext[1], oext[2], oext[3], oext[4], oext[5])
voi.SetInputData(gi)
voi.Update()
ido.ShallowCopy(voi.GetOutput())

