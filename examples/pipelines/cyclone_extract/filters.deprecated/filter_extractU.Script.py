import vtk
di = self.GetInputDataObject(0,0)
ipd = di.GetPointData()
ci = self.GetInputDataObject(0,1)
ido = self.GetImageDataOutput()
diExt = di.GetExtent()
offs = 64
ctrs = ci.GetPoints().GetPoint(0)
ctrX = int(ctrs[0])
ctrY = int(ctrs[1])
oext = [ int(max([int(ctrX-offs),int(diExt[0])])), int(min([int(ctrX+offs),int(diExt[1])]))
       , int(max([int(ctrY-offs),int(diExt[2])])), int(min([int(ctrY+offs),int(diExt[3])]))
       , int(diExt[4]), int(diExt[5])]
voi = vtk.vtkExtractVOI()
voi.SetVOI(oext[0], oext[1], oext[2], oext[3], oext[4], oext[5])
voi.SetInputData(di)
voi.Update()
ido.ShallowCopy(voi.GetOutput())
