import os, sys
sys.path.insert(0, os.getcwd())

import InshimtuLib as pplz

dir(pplz)

ispi = pplz.InputSpecPipeline()
dp = pplz.FilesystemPath('/home/holstgr/Development/Inshimtu/testing/data')
r = pplz.Regex('wrfoutReady_d01_.*')
isp = pplz.InputSpecPaths(dp, r)

fp_match = pplz.FilesystemPath('/home/holstgr/Development/Inshimtu/testing/data/wrfoutReady_d01_2015-10-30_23:00:00.nc')
fp_nomatch = pplz.FilesystemPath('/home/holstgr/Development/Inshimtu/testing/data/wrfout_d01_2015-10-30_23:00:00.nc')
isp.match(fp_match)
isp.match(fp_nomatch)
ispec = pplz.InputSpec(isp)
print(type(ispec))

pr = pplz.Regex('^(.*)/wrfoutReady_(.*)$')
rf = pplz.ReplaceRegexFormat(pr, '${1}/wrfout_${2}')
psrf = pplz.ProcessingSpecReadyFile(rf)
orf = psrf.get(fp_match)
print(orf.get().string() if orf.is_initialized() else None)

sf = pplz.FilesystemPath('/home/holstgr/Development/Inshimtu/testing/configs/vti_notified.json')
ex_echo = pplz.FilesystemPath('/usr/bin/echo')
ex_cat = pplz.FilesystemPath('/usr/bin/cat')
args0 = pplz.VectorString()
args1 = pplz.VectorString()
args0.extend(['Reading File: ', pplz.ProcessingSpecCommands.FILENAME_ARG])
args1.extend([pplz.ProcessingSpecCommands.FILENAME_ARG])
cmd0 = pplz.Command(ex_echo, args0)
cmd1 = pplz.Command(ex_cat, args1)
cmds = pplz.CommandSequence()
cmds.extend([cmd0, cmd1])
psc = pplz.ProcessingSpecCommands(cmds)
psc.process(sf)

cscpts = pplz.VectorFilesystemPath()
cscpts.extend([pplz.FilesystemPath(i) for i in ['/home/holstgr/Development/Inshimtu/testing/pipelines/gridwriter.py','/home/holstgr/Development/Inshimtu/testing/pipelines/gridviewer_vti_velocity.py']])
cvars = pplz.VectorString()
cvars.extend(['U,V,W,QVAPOR'])
pscc = pplz.ProcessingSpecCatalyst(cscpts, cvars)
pspec = pplz.ProcessingSpec(psc) 

osp = pplz.OutputSpecDone()
ospp = pplz.OutputSpecPipeline()
ospec = pplz.OutputSpec(ospp)

pipeline = pplz.PipelineSpec(ispec, pspec, ospec)

