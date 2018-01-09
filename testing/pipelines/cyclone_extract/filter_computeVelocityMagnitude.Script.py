from math import sqrt
diU = inputs[0]
diV = inputs[1]
diW = inputs[2]
idiU = diU.PointData['U']
idiV = diV.PointData['V']
idiW = diW.PointData['W']
ido = self.GetImageDataOutput()
[minXu, maxXu, minYu, maxYu, minZu, maxZu] = diU.GetExtent()
(dimXu, dimYu, dimZu) = diU.GetDimensions()
[minXv, maxXv, minYv, maxYv, minZv, maxZv] = diV.GetExtent()
(dimXv, dimYv, dimZv) = diV.GetDimensions()
[minXw, maxXw, minYw, maxYw, minZw, maxZw] = diW.GetExtent()
(dimXw, dimYw, dimZw) = diW.GetDimensions()

[minX, maxX, minY, maxY, minZ, maxZ] = [ max(minXu,minXv,minXw), min(maxXu,maxXv,maxXw)
                                       , max(minYu,minYv,minYw), min(maxYu,maxYv,maxYw)
                                       , max(minZu,minZv,minZw), min(maxZu,maxZv,maxZw)]
(dimX, dimY, dimZ) = ( int(min(dimXu,dimXv,dimXw))
                     , int(min(dimYu,dimYv,dimYw))
                     , int(min(dimZu,dimZv,dimZw)))

eext = [minX, maxX, minY, maxY, minZ, maxZ]
oext = [0, maxX - minX, 0, maxY - minY, 0, maxZ - minZ]

# TODO: Get this version working
#idiUe = vtk.vtkExtractVOI()
#idiUe.SetVOI(eext[0], eext[1], eext[2], eext[3], eext[4], eext[5])
#idiUe.SetInputData(idiU)
#idiUe.Update()
#idiVe = vtk.vtkExtractVOI()
#idiVe.SetVOI(eext[0], eext[1], eext[2], eext[3], eext[4], eext[5])
#idiVe.SetInputData(idiV)
#idiUe.Update()
#idiWe = vtk.vtkExtractVOI()
#idiWe.SetVOI(eext[0], eext[1], eext[2], eext[3], eext[4], eext[5])
#idiWe.SetInputData(idiW)
#idiWe.Update()

ido.SetExtent(oext)
ca = vtk.vtkFloatArray()
ca.SetName('vel')
ca.SetNumberOfComponents(1)
ca.SetNumberOfTuples(dimX*dimY*dimZ)

# TODO: Get this version working
#for k in range(0, ca.GetNumberOfTuples()):
#  ca.SetValue(k, sqrt(idiUe[k]*idiUe[k] + idiVe[k]*idiVe[k] +idiWe[k]*idiWe[k]))

# TODO: because we've altered the extent of the inputs, the assumptions of the single loop
#       are not valid (e.g., that U,V,W start from the same position and have the same extent)
#       need to calculate kU,kV,kW individually
# Note: Assuming Fortran format (?
for z in range(oext[4], oext[5]):
for y in range(oext[2], oext[3]):
for x in range(oext[0], oext[1]):
  k = x + (y * oext[1]) + (z * oext[1] * oext[3])
  ku = (x + minXu) + ((y + minYu) * (maxXu - maxXu)) + ((z + minZu) * (maxXu - maxXu) * (maxYu - maxYu))
  kv = (x + minXv) + ((y + minYv) * (maxXv - maxXv)) + ((z + minZv) * (maxXv - maxXv) * (maxYv - maxYv))
  kw = (x + minXw) + ((y + minYw) * (maxXw - maxXw)) + ((z + minZw) * (maxXw - maxXw) * (maxYw - maxYw))
  ca.SetValue(k, sqrt(idiU[ku]*idiU[ku] + idiV[ku]*idiV[ku] +idiW[kw]*idiW[kw]))


ido.GetPointData().AddArray(ca)

