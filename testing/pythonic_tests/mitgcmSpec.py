import InshimtuLib as pplz
import os.path, sys

# Python interpreter Hack
# TESTPATH = "../testing"

# Pipeline Spec

dp = pplz.FilesystemPath('/lustre/scratch/inshimtu-testing/mitgcm_4km-output/mitgcmrun-output')
varz = ['Convtave','ETAtave','Eta','PhHytave','S','Stave','T', 'Tdiftave', 'Ttave', 'U', 'V', 'W', 'sFluxtave', 'tFluxtave', 'uFluxtave', 'uVeltave', 'uZtave', 'vFluxtave', 'vVeltave', 'vZtave', 'wVeltave']
extz = ['data', 'meta']
r = pplz.Regex('(' + '|'.join(varz) + ')\.[0-9]+\.(' + '|'.join(extz) + ')')
isp = pplz.InputSpecPaths(dp, r)

ex_zip = pplz.CommandExe('echo') #('zip')
args0 = pplz.VectorString(['zip', '-j', '-m', '-T', '/lustre/scratch/mitgcm-output/mitgcm.${TIMESTEP_CODE}.zip', '-i', pplz.ProcessingSpecCommands.FILENAMES_ARRAY_ARG]) 
cmd0 = pplz.Command(ex_zip, args0)
cmds = pplz.CommandSequence([cmd0])
psc = pplz.ProcessingSpecCommands(cmds)
pspec = pplz.ProcessingSpec(psc)

osp = pplz.OutputSpecDone()
ospp = pplz.OutputSpecPipeline()
ospec = pplz.OutputSpec(ospp)


# Input Spec Tests
filenameList = os.path.join(TESTPATH, 'data/mitgcm-output.example-ls.txt')
with open(filenameList) as f:
  filenames = f.readlines()

filepaths = [pplz.FilesystemPath(fn.rstrip()) for fn in filenames]
matches = [isp.match(p) for p in filepaths]
matched = [p for p, m in zip(filepaths, matches) if m == True]
matched_true = [(p, m) for p, m in zip(filepaths, matches) if m == True]
matched_false = [(p, m) for p, m in zip(filepaths, matches) if m == False]

print("Matched = True:")
for p, m in matched_true:
  print(p.string(), m)

print("Matched = False:")
for p, m in matched_false:
  print(p.string(), m)

ispec = pplz.InputSpec(isp)
readyFiles = pplz.VectorFilesystemPath(filepaths)
acceptedFiles = pplz.VectorFilesystemPath()
outAttributes = pplz.Attributes()
r = pplz.pipelineAcceptInput(ispec, readyFiles, acceptedFiles, outAttributes)

print("Accepted:")
for p in acceptedFiles:
  print(p.string())


# InputSpec Logic Script
''' Collect Timesteps '''

import re

pr = pplz.Regex('^.*/(' + '|'.join(varz) + ')\.([0-9]+)\.(' + '|'.join(extz) + ')$')
rf = pplz.ReplaceRegexFormat(pr, '${2}')
psrf = pplz.ProcessingSpecReadyFile(rf)

ts0 = [psrf.get(fp) for fp in matched]
ts = sorted(list(set([o.get().string() for o in ts0 if o.is_initialized()])))

print(ts)

def collectTimestepFilepaths(timestep, zzs, filepaths):
  (varz, extz) = zzs
  r = '^.*/(' + '|'.join(varz) + ')\.' + timestep + '\.(' + '|'.join(extz) + ')$'
  matches = [p for p in filepaths if re.match(r, p.string())]
  return matches

expectedCount = len(varz) * len(extz)
for t in ts:
  ps = collectTimestepFilepaths(t, (varz, extz), matched)
  print(t, 'expected: %s' % expectedCount, 'found: %s' % (len(ps)))


def acceptFileset(timesteps, zzs, filepaths, accepted):
  (varz, extz) = zzs
  expectedCount = len(varz) * len(extz)
  assert(expectedCount > 0)
  ts = sorted(list(set(timesteps)))
  firstPS = None
  for t in ts:
    ps = collectTimestepFilepaths(t, zzs, filepaths)
    psLen = len(ps)
    if firstPS == None and psLen > 0:
      firstPS = ps
    if psLen == expectedCount:
      accepted.extend(firstPS)
      return True
  return False

accepted = pplz.VectorFilesystemPath()
res = acceptFileset(ts, (varz, extz), matched, accepted)
print("Accepted Filesets:", res)
for p in accepted:
  print(p.string())


# InputSpec Logic Script - 2
script = '''
import re

varz = ['Convtave','ETAtave','Eta','PhHytave','S','Stave','T', 'Tdiftave', 'Ttave', 'U', 'V', 'W', 'sFluxtave', 'tFluxtave', 'uFluxtave', 'uVeltave', 'uZtave', 'vFluxtave', 'vVeltave', 'vZtave', 'wVeltave']
extz = ['data', 'meta']

r = inshimtu.Regex('(' + '|'.join(varz) + ')\.[0-9]+\.(' + '|'.join(extz) + ')')
ipaths = inshimtu.InputSpecPaths(ACCEPT_DIRECTORY, r)

pr = inshimtu.Regex('^.*/(' + '|'.join(varz) + ')\.([0-9]+)\.(' + '|'.join(extz) + ')$')
rf = inshimtu.ReplaceRegexFormat(pr, '${2}')
preplace = inshimtu.ProcessingSpecReadyFile(rf)

def collectTimestepFilepaths(timestep, zzs, filepaths):
  (varz, extz) = zzs
  r = '^.*/(' + '|'.join(varz) + ')\.' + timestep + '\.(' + '|'.join(extz) + ')$'
  matches = [p for p in filepaths if re.match(r, p.string())]
  return matches

def acceptFileset(timesteps, zzs, filepaths):
  (varz, extz) = zzs
  expectedCount = len(varz) * len(extz)
  assert(expectedCount > 0)
  ts = sorted(list(set(timesteps)))
  firstTPS = None
  for t in ts:
    ps = collectTimestepFilepaths(t, zzs, filepaths)
    psLen = len(ps)
    if firstTPS == None and psLen > 0:
      firstTPS = (t, ps)
    if psLen == expectedCount:
      return firstTPS
  return None

def accept(available, outAccepted, outAttributes):
  ts = [o.get().string() for o in [preplace.get(fp) for fp in available] if o.is_initialized()]
  result = acceptFileset(ts, (varz, extz), available)
  if not result:
    return False
  t, ps = result
  if len(t) > 0:
    tv = pplz.AttributeValue(t)
    outAttributes.setAttribute(pplz.ProcessingSpecCommands.TIMESTEP_CODE_ARG, tv)
  outAccepted.extend(ps)
  return True
'''

isp.setAcceptScript(script, pplz.FilesystemPath(os.path.abspath(sys.path[0])))
ispec = pplz.InputSpec(isp)
readyFiles = pplz.VectorFilesystemPath(filepaths)
acceptedFiles = pplz.VectorFilesystemPath()
attributes = pplz.Attributes()
r = pplz.pipelineAcceptInput(ispec, readyFiles, acceptedFiles, attributes)

print("Accepted Script:", r)
for p in acceptedFiles:
  print(p.string())


# Pipeline Execute

ispec = pplz.InputSpec(isp)
pipeline = pplz.PipelineSpec('mitgcm-zip-pipeline', ispec, pspec, ospec)

readyFiles = pplz.VectorFilesystemPath(filepaths)
acceptedFiles = pplz.VectorFilesystemPath()
attributes = pplz.Attributes()

if pplz.pipelineAcceptInput(pipeline.getInput(), readyFiles, acceptedFiles, attributes):
  print('Pipeline "%s" accepted %s file(s)' % (pipeline.name, len(acceptedFiles)))
  task = pplz.pipelineMkPipelineTaskNoCatalyst(pipeline, acceptedFiles, attributes)
  print("Running pipeline...")
  pplz.pipelineProcessTask(task)
  print("Done.")
  print("Task - wasSuccessful: %s, canContinue: %s, hasError: %s, taskStatus: %s" % \
        (task.get().wasSuccessful(), task.get().canContinue(), task.get().hasError(), task.get().taskStatus))
  print("Input Files:")
  for p in task.get().inputFiles:
    print(p.string())  
  print("Output Files:")
  for p in task.get().outputFiles:
    print(p.string())  



