# NuSMV/code/nusmv/core/trace/meson.build

core_c += files(
  'pkg_trace.c',
  'Trace.c',
  'traceEval.c',
  'TraceLabel.c',
  'TraceMgr.c',
  'TraceOpt.c',
  'Trace_private.c',
  'traceTest.c',
  'traceUtils.c',
  'TraceXml.c',

  'eval/BaseEvaluator.c',

  'exec/BaseTraceExecutor.c',
  'exec/BDDCompleteTraceExecutor.c',
  'exec/BDDPartialTraceExecutor.c',
  'exec/CompleteTraceExecutor.c',
  'exec/PartialTraceExecutor.c',
  'exec/SATCompleteTraceExecutor.c',
  'exec/SATPartialTraceExecutor.c',
  'exec/traceExec.c',

  'loaders/TraceLoader.c',
  
  'plugins/TraceCompact.c',
  'plugins/TraceExplainer.c',
  'plugins/TracePlugin.c',
  'plugins/TraceTable.c',
  'plugins/TraceXmlDumper.c',
  'plugins/TraceEmpty.c',
)

if libxml2_dep.found()
  core_c += files(
    'loaders/TraceXmlLoader.c',
  )
endif
