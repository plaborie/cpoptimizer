using CP;

int nbJobs = ...;
int nbMchs = ...;

range Jobs = 0..nbJobs-1;
range Mchs = 0..nbMchs-1; 

int OpDurations[j in Jobs][m in Mchs] = ...;

dvar interval itvs[j in Jobs][m in Mchs]  size OpDurations[j][m];
dvar sequence mchs[m in Mchs] in all(j in Jobs) itvs[j][m];
dvar sequence jobs[j in Jobs] in all(m in Mchs) itvs[j][m];

execute {
   cp.param.FailLimit = 10000;
}

minimize max(j in Jobs, m in Mchs) endOf(itvs[j][m]);
subject to {
  forall (j in Jobs)
    noOverlap(jobs[j]);
  forall (m in Mchs)
    noOverlap(mchs[m]);
}

execute {
  for (var j = 0; j <= nbJobs-1; j++) {
    for (var m = 0; m <= nbMchs-1; m++) {
      write(itvs[j][m].start + " ");
    }
    writeln("");
  }
}
