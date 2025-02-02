/* ---------------------------------------------------------------------------

  This file is part of the ``wff.rewriters'' package
  of NuSMV version 2. Copyright (C) 2006 by FBK-irst.

  NuSMV version 2 is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  NuSMV version 2 is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA.

  For more information on NuSMV see <http://nusmv.fbk.eu>
  or email to <nusmv-users@fbk.eu>.
  Please report bugs to <nusmv-users@fbk.eu>.

  To contact the NuSMV development board, email to <nusmv@fbk.eu>.

-----------------------------------------------------------------------------*/

/*!
  \author Enrico Magnago
  \brief Implementaion of class 'WffRewriterBase'

  \todo: Missing description

*/


#include "nusmv/core/wff/rewriters/WffRewriterBase_private.h"
#include "nusmv/core/wff/rewriters/MasterWffRewriter_private.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
/* See 'WffRewriterBase_private.h' for class 'WffRewriterBase' definition.   */

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static void wff_rewriter_base_finalize(Object_ptr object, void* dummy);

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

VIRTUAL void WffRewriterBase_destroy(WffRewriterBase_ptr self)
{
  WFF_REWRITER_BASE_CHECK_INSTANCE(self);
  Object_destroy(OBJECT(self), NULL);
}

VIRTUAL WffRewriteFormulaKind
WffRewriterBase_rewrite_formula(const WffRewriterBase_ptr self,
                                const WffRewriteMethod method,
                                const SymbLayer_ptr layer,
                                const FlatHierarchy_ptr outfh,
                                node_ptr* const expr,
                                const short int spec_type,
                                const NodeList_ptr new_var_exprs)
{
  WFF_REWRITER_BASE_CHECK_INSTANCE(self);
  nusmv_assert(self->rewrite_formula != NULL);

  return self->rewrite_formula(self, method, layer, outfh,
                               expr, spec_type,
                               new_var_exprs);
}

VIRTUAL boolean
WffRewriterBase_is_rewriting_needed(const WffRewriterBase_ptr self,
                                    const SymbTable_ptr st,
                                    const WffRewriterExpectedProperty eproptype,
                                    const node_ptr wff,
                                    const node_ptr context)
{
  WFF_REWRITER_BASE_CHECK_INSTANCE(self);
  nusmv_assert(self->is_rewriting_needed != NULL);

  return self->is_rewriting_needed(self, st, eproptype, wff, context);
}

WffRewriteFormulaKind
wff_rewriter_base_throw_rewrite_formula(const WffRewriterBase_ptr self,
                                        const WffRewriteMethod method,
                                        const SymbLayer_ptr layer,
                                        const FlatHierarchy_ptr outfh,
                                        node_ptr* const expr,
                                        const short int spec_type,
                                        const NodeList_ptr new_var_exprs)
{
  WffRewriteFormulaKind retval;
  if (NodeWalker_can_handle(NODE_WALKER(self), *expr)) {
    /* checks if self can handle the node without need of re-throw
       to the master */
    retval = WffRewriterBase_rewrite_formula(self, method,
                                             layer, outfh,
                                             expr, spec_type,
                                             new_var_exprs);
  }
  else {
    const MasterWffRewriter_ptr master =
      MASTER_WFF_REWRITER(NODE_WALKER(self)->master);
  retval =
    master_wff_rewriter_rewrite(master, method,
                                layer, outfh,
                                expr, spec_type,
                                new_var_exprs);
  }
  return retval;
}

boolean
wff_rewriter_base_throw_is_rewriting_needed(const WffRewriterBase_ptr self,
                                            const SymbTable_ptr st,
                                            const WffRewriterExpectedProperty eproptype,
                                            const node_ptr wff,
                                            const node_ptr context)
{
  boolean retval;
  if (NodeWalker_can_handle(NODE_WALKER(self), wff)) {
    /* checks if self can handle the node without need of re-throw
       to the master */
    retval = WffRewriterBase_is_rewriting_needed(self, st, eproptype,
                                                 wff, context);
  }
  else {
    const MasterWffRewriter_ptr master =
      MASTER_WFF_REWRITER(NODE_WALKER(self)->master);
    retval =
      MasterWffRewriter_is_rewriting_needed(master, st, eproptype,
                                            wff, context);
  }

  return retval;
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

void wff_rewriter_base_init(WffRewriterBase_ptr self, const NuSMVEnv_ptr env,
                            const char* name, const int low, const size_t num,
                            const boolean handle_null)
{
  /* base class initialization */
  node_walker_init(NODE_WALKER(self), env, name, low, num,
                   handle_null);

  /* virtual methods settings */
  OVERRIDE(Object, finalize) = wff_rewriter_base_finalize;
  OVERRIDE(WffRewriterBase, rewrite_formula) = NULL;
  OVERRIDE(WffRewriterBase, is_rewriting_needed) = NULL;
}

void wff_rewriter_base_deinit(WffRewriterBase_ptr self)
{
  /* members deinitialization */

  /* base class initialization */
  node_walker_deinit(NODE_WALKER(self));
}

void wff_rewriter_base_copy_aux(const WffRewriterBase_ptr self,
                                WffRewriterBase_ptr other)
{
  /* base class copy constructor */
  node_walker_copy_aux(NODE_WALKER(self), NODE_WALKER(other));

  /* virtual methods copy */
  other->rewrite_formula = self->rewrite_formula;
  other->is_rewriting_needed = self->is_rewriting_needed;
}


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/*!
  \brief The WffRewriterBase class virtual finalizer

  Called by the class destructor
*/
static void wff_rewriter_base_finalize(Object_ptr object, void* dummy)
{
  WffRewriterBase_ptr self = WFF_REWRITER_BASE(object);

  wff_rewriter_base_deinit(self);
  FREE(self);
}


/**AutomaticEnd***************************************************************/
