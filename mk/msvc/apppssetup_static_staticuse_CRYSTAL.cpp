// This file is automatically generated.
#include "cssysdef.h"
#include "csutil/scf.h"

// Put static linking stuff into own section.
// The idea is that this allows the section to be swapped out but not
// swapped in again b/c something else in it was needed.
#if !defined(CS_DEBUG) && defined(CS_COMPILER_MSVC)
#pragma const_seg(".CSmetai")
#pragma comment(linker, "/section:.CSmetai,r")
#pragma code_seg(".CSmeta")
#pragma comment(linker, "/section:.CSmeta,er")
#pragma comment(linker, "/merge:.CSmetai=.CSmeta")
#endif
struct _static_use_CRYSTAL { _static_use_CRYSTAL (); };
_static_use_CRYSTAL::_static_use_CRYSTAL () {}
SCF_USE_STATIC_PLUGIN(csddsimg)
SCF_USE_STATIC_PLUGIN(cspngimg)
SCF_USE_STATIC_PLUGIN(csfont)
SCF_USE_STATIC_PLUGIN(cssynldr)
SCF_USE_STATIC_PLUGIN(dsplex)
SCF_USE_STATIC_PLUGIN(engine)
SCF_USE_STATIC_PLUGIN(fontplex)
SCF_USE_STATIC_PLUGIN(freefnt2)
SCF_USE_STATIC_PLUGIN(gl3d)
SCF_USE_STATIC_PLUGIN(imgplex)
SCF_USE_STATIC_PLUGIN(rendstep_std)
SCF_USE_STATIC_PLUGIN(reporter)
SCF_USE_STATIC_PLUGIN(shadermgr)
SCF_USE_STATIC_PLUGIN(stdrep)
SCF_USE_STATIC_PLUGIN(vfs)
SCF_USE_STATIC_PLUGIN(xmlshader)
SCF_USE_STATIC_PLUGIN(xmltiny)
SCF_USE_STATIC_PLUGIN(glwin32)

