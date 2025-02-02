# subprojects/packagefiles/cudd-2.4.1/cudd/meson.build

cudd_inc = include_directories(
  '.',
)

cudd_h = files(
  'cudd.h',
)

cudd_c = files(
  'cuddAddFind.c',
  'cuddAddIte.c',
  'cuddAddOp.c',
  'cuddAndAbs.c',
  'cuddAnneal.c',
  'cuddAPI.c',
  'cuddApprox.c',
  'cuddBddAbs.c',
  'cuddBddCorr.c',
  'cuddBddIte.c',
  'cuddBddOp.c',
  'cuddBridge.c',
  'cuddCache.c',
  'cuddCheck.c',
  'cuddClip.c',
  'cuddCof.c',
  'cuddCompose.c',
  'cuddDecomp.c',
  'cuddEssent.c',
  'cuddExact.c',
  'cuddExport.c',
  'cuddGenCof.c',
  'cuddGenetic.c',
  'cuddGroup.c',
  'cuddInit.c',
  'cuddInteract.c',
  'cuddLCache.c',
  'cuddLevelQ.c',
  'cuddLinear.c',
  'cuddLiteral.c',
  'cuddRef.c',
  'cuddReorder.c',
  'cuddSat.c',
  'cuddSign.c',
  'cuddSolve.c',
  'cuddSplit.c',
  'cuddSubsetHB.c',
  'cuddSubsetSP.c',
  'cuddSymmetry.c',
  'cuddTable.c',
  'cuddUtil.c',
  'cuddWindow.c',
  'cuddZddCount.c',
  'cuddZddFuncs.c',
  'cuddZddGroup.c',
  'cuddZddIsop.c',
  'cuddZddLin.c',
  'cuddZddMisc.c',
  'cuddZddPort.c',
  'cuddZddReord.c',
  'cuddZddSetop.c',
  'cuddZddSymm.c',
  'cuddZddUtil.c',
)

if meson.is_subproject()
  # Awful workaround for meson issue
  # https://github.com/mesonbuild/meson/issues/2546
  fs = import('fs')
  foreach h: [cudd_h, mtr_h, st_h, util_h]
    configure_file(copy: true, input: h, output: fs.name(h))
  endforeach
endif
