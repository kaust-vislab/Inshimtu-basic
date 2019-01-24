import InshimtuLib as pplz
import os.path

dir(pplz)

ispi = pplz.InputSpecPipeline()
dp = pplz.FilesystemPath(os.path.join(TESTPATH, 'data'))
r = pplz.Regex('wrfoutReady_d01_.*')
isp = pplz.InputSpecPaths(dp, r)

fp_match = pplz.FilesystemPath(os.path.join(TESTPATH, 'data/wrfoutReady_d01_2015-10-30_23:00:00.nc'))
fp_nomatch = pplz.FilesystemPath(os.path.join(TESTPATH, 'data/wrfout_d01_2015-10-30_23:00:00.nc'))
isp.match(fp_match)
isp.match(fp_nomatch)
ispec = pplz.InputSpec(isp)
print(type(ispec))

pr = pplz.Regex('^(.*)/wrfoutReady_(.*)$')
rf = pplz.ReplaceRegexFormat(pr, '${1}/wrfout_${2}')
psrf = pplz.ProcessingSpecReadyFile(rf)
orf = psrf.get(fp_match)
print(orf.get().string() if orf.is_initialized() else None)

sf = pplz.FilesystemPath(os.path.join(TESTPATH, 'configs/vti_notified.json'))
ex_echo = pplz.CommandExe('/usr/bin/echo')
ex_cat = pplz.CommandExe('/usr/bin/cat')
args0 = pplz.VectorString(['Reading File: ', pplz.ProcessingSpecCommands.FILENAME_ARG])
args1 = pplz.VectorString([pplz.ProcessingSpecCommands.FILENAME_ARG])
cmd0 = pplz.Command(ex_echo, args0)
cmd1 = pplz.Command(ex_cat, args1)
cmds = pplz.CommandSequence()
cmds.extend([cmd0, cmd1])
psc = pplz.ProcessingSpecCommands(cmds)
psc.process(sf)

cscpts = pplz.VectorFilesystemPath([pplz.FilesystemPath(i) for i in [os.path.join(TESTPATH, 'pipelines/gridwriter.py'), os.path.join(TESTPATH, 'pipelines/gridviewer_vti_velocity.py')]])
cvars = pplz.VectorString(['U,V,W,QVAPOR'])
pscc = pplz.ProcessingSpecCatalyst(cscpts, cvars)
pspec = pplz.ProcessingSpec(psc) 

osp = pplz.OutputSpecDone()
ospp = pplz.OutputSpecPipeline()
ospec = pplz.OutputSpec(ospp)

pipeline = pplz.PipelineSpec('pipeline', ispec, pspec, ospec)
print(pipeline.name)

