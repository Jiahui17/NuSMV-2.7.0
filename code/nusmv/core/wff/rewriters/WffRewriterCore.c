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
  \author Michele Dorigatti, Marco Roveri, Enrico Magnago
  \brief Implementaion of class 'WffRewriterCore'

  \todo: Missing description

*/


#include "nusmv/core/wff/wff.h"
#include "nusmv/core/wff/rewriters/WffRewriterCore_private.h"
#include "nusmv/core/wff/rewriters/MasterWffRewriter_private.h"

#include "nusmv/core/parser/symbols.h"
#include "nusmv/core/compile/compile.h"

#include "nusmv/core/utils/error.h"
#include "nusmv/core/utils/ErrorMgr.h"
#include "nusmv/core/opt/OptsHandler.h"
#include "nusmv/core/utils/Logger.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
/* See 'WffRewriterCore_private.h' for class 'WffRewriterCore' definition.   */

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static void wff_rewriter_core_finalize(Object_ptr object, void* dummy);

static Object_ptr wff_rewriter_core_copy(const Object_ptr self);


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

WffRewriterCore_ptr
WffRewriterCore_create(const NuSMVEnv_ptr env)
{
  WffRewriterCore_ptr self = ALLOC(WffRewriterCore, 1);
  WFF_REWRITER_CORE_CHECK_INSTANCE(self);

  wff_rewriter_core_init(self, env, WFF_REWRITER_CORE_NAME,
                         NUSMV_CORE_SYMBOL_FIRST,
                         NUSMV_CORE_SYMBOL_LAST - NUSMV_CORE_SYMBOL_FIRST,
                         true /* can handle null */);
  return self;
}


VIRTUAL void WffRewriterCore_destroy(WffRewriterCore_ptr self)
{
  WFF_REWRITER_CORE_CHECK_INSTANCE(self);
  Object_destroy(OBJECT(self), NULL);
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

void wff_rewriter_core_init(WffRewriterCore_ptr self, const NuSMVEnv_ptr env,
                            const char* name, const int low, const size_t num,
                            const boolean handle_null)
{
  /* core class initialisation */
  wff_rewriter_base_init(WFF_REWRITER_BASE(self), env, name, low, num,
                         handle_null);

  /* members initialization */

  /* virtual methods settings */
  OVERRIDE(Object, finalize) = wff_rewriter_core_finalize;
  OVERRIDE(Object, copy) = wff_rewriter_core_copy;
  OVERRIDE(WffRewriterBase, rewrite_formula) = wff_rewriter_core_rewrite_formula;
  OVERRIDE(WffRewriterBase, is_rewriting_needed) =
    wff_rewriter_core_is_rewriting_needed;
}

void wff_rewriter_core_deinit(WffRewriterCore_ptr self)
{
  /* members deinitialization */

  /* base class initialization */
  wff_rewriter_base_deinit(WFF_REWRITER_BASE(self));
}

void wff_rewriter_core_copy_aux(const WffRewriterCore_ptr self,
                                WffRewriterCore_ptr other)
{
  /* base class copy constructor */
  wff_rewriter_base_copy_aux(WFF_REWRITER_BASE(self), WFF_REWRITER_BASE(other));
}


boolean wff_rewriter_core_is_rewriting_needed(const WffRewriterBase_ptr self,
                                              const SymbTable_ptr st,
                                              const WffRewriterExpectedProperty eproptype,
                                              const node_ptr wff,
                                              const node_ptr context)
{
  boolean result = false;

  switch(eproptype) {
  case WFF_REWRITER_REWRITE_INPUT_NEXT:
    {
      const node_ptr flat_wff =
        Compile_FlattenSexpExpandDefine(st, wff, context);

      const Set_t cone =
        Formula_GetDependenciesByType(st, flat_wff, Nil,
                                      VFT_NEXT | VFT_INPUT, true);

      /* If there are next or input then return true */
      result = !Set_IsEmpty(cone);
      Set_ReleaseSet(cone);
    }
    break;
  case WFF_REWRITER_LTL_2_INVAR:
    result = true;
    break;
  default:
    error_unreachable_code_msg("Unrecognized rewriting of expected input property");
    break;
  }

  return result;
}

WffRewriteFormulaKind
wff_rewriter_core_rewrite_formula(const WffRewriterBase_ptr self_,
                                  const WffRewriteMethod method,
                                  const SymbLayer_ptr layer,
                                  const FlatHierarchy_ptr outfh,
                                  node_ptr* const expr,
                                  const short int spec_type,
                                  const NodeList_ptr new_var_exprs)
{
  const NuSMVEnv_ptr env = ENV_OBJECT_GET_ENV(self_);
  const WffRewriterCore_ptr self = WFF_REWRITER_CORE(self_);
  const NodeMgr_ptr nodemgr = NODE_MGR(NuSMVEnv_get_value(env, ENV_NODE_MGR));
  const MasterPrinter_ptr wffprint =
    MASTER_PRINTER(NuSMVEnv_get_value(env, ENV_WFF_PRINTER));

  const OptsHandler_ptr opts =
    OPTS_HANDLER(NuSMVEnv_get_value(env, ENV_OPTS_HANDLER));
  const ErrorMgr_ptr errmgr =
    ERROR_MGR(NuSMVEnv_get_value(env, ENV_ERROR_MANAGER));
  const Logger_ptr logger = LOGGER(NuSMVEnv_get_value(env, ENV_LOGGER));

  const SymbTable_ptr symb_table = FlatHierarchy_get_symb_table(outfh);

  WffRewriteFormulaKind retval = WFF_REWRITE_FORMULA_KIND_FIRST;
  WffRewriteFormulaKind kind1 = WFF_REWRITE_FORMULA_KIND_FIRST;
  WffRewriteFormulaKind kind2 = WFF_REWRITE_FORMULA_KIND_FIRST;
  node_ptr expr1 = Nil;
  node_ptr expr2 = Nil;


#ifdef DEBUG_WFF_REWRITE
  {
    const MasterPrinter_ptr sexpprint =
      MASTER_PRINTER(NuSMVEnv_get_value(env, ENV_SEXP_PRINTER));

    Logger_inc_indent_size(logger);
    Logger_vnlog_error(logger, sexpprint, opts, "Input: \n%N\n", *expr);
  }
#endif

  if (Nil == *expr) {
    retval = WFF_REWRITE_FORMULA_KIND_STATE;
  }
  else {
    switch (node_get_type(*expr)) {
      /* --- constants ---
         the expression is already find_atom => no need to create a copy */
    case FAILURE:  case TRUEEXP:  case FALSEEXP:
    case NUMBER:  case NUMBER_UNSIGNED_WORD:  case NUMBER_SIGNED_WORD:
    case UWCONST: case SWCONST:
    case NUMBER_FRAC:  case NUMBER_REAL:  case NUMBER_EXP:
      retval =  WFF_REWRITE_FORMULA_KIND_STATE;
      break;

      /* In this case we do not need to extend the language */
    case WSIZEOF:
      retval=  WFF_REWRITE_FORMULA_KIND_STATE;
      break;

      /* can contain an identifier or a next */
    case CONTEXT:
      expr2 = cdr(*expr);
      kind2 = _THROW_REWRITE(self, method, layer, outfh, &expr2, spec_type,
                             new_var_exprs);


      *expr = find_node(nodemgr, node_get_type(*expr), car(*expr), expr2);
      retval = kind2;
      break;

      /* --- identifier ---
         since the expression is already flattened there is not need
         to resolve the identifier, find_atom it or create a copy. */
    case ATOM:  case DOT:  case ARRAY:
#ifndef NDEBUG
    { /* symbols should be already resolved */
      const ResolveSymbol_ptr rs =
        SymbTable_resolve_symbol(symb_table, *expr, Nil);
      const node_ptr resolved_name = ResolveSymbol_get_resolved_name(rs);
      nusmv_assert(resolved_name == *expr);
    }
#endif
      {
        if (SymbTable_is_symbol_input_var(symb_table, *expr)) {
          retval = WFF_REWRITE_FORMULA_KIND_INPUT;
          break;
        }

        if (SymbTable_is_symbol_state_var(symb_table, *expr) ||
            SymbTable_is_symbol_constant(symb_table, *expr) ||
            SymbTable_is_symbol_frozen_var(symb_table, *expr)) {
          retval = WFF_REWRITE_FORMULA_KIND_STATE;
          break;
        }

        Logger_vnlog_error(logger, wffprint, opts,
                           "Internal error: Unknown identifier %N is met during PROP REWRITE\n",
                           *expr);
        ErrorMgr_nusmv_exit(errmgr, 1);
      }

    case NFUNCTION:
      /* No need to look at the name of the nfunction */
      expr2 = cdr(*expr);
      kind2 = _THROW_REWRITE(self, method, layer, outfh, &expr2, spec_type,
                             new_var_exprs);

      nusmv_assert(NULL != car(*expr));

      *expr = find_node(nodemgr, node_get_type(*expr), car(*expr), expr2);
      retval = kind2;
      break;

      /* --- next --- */
    case NEXT:
      *expr = Wff_make_next(nodemgr, car(*expr));
      retval = WFF_REWRITE_FORMULA_KIND_NEXT;
      break;

      /* --- unary non-temporal operator --- */
    case NOT:
    case UMINUS:
    case FLOOR:
    case CAST_TOINT:
      expr1 = car(*expr);
      kind1 = _THROW_REWRITE(self, method, layer, outfh, &expr1, spec_type,
                             new_var_exprs);

      nusmv_assert(Nil == cdr(*expr));

      *expr = find_node(nodemgr, node_get_type(*expr), expr1, Nil);
      retval =  kind1;
      break;

      /* --- binary non-temporal operators ---
         if one operand has input and other has a temporal op =>
         rewrite operand with input var and return temporal kind.
         If kinds are different "input" and "temporal" kinds wins
         "state".*/
    case TWODOTS: /* This is dealt as a binary operator */
    case AND: case OR: case IMPLIES: case IFF: case XOR: case XNOR:
      {
        const MasterWffRewriter_ptr master =
          MASTER_WFF_REWRITER(NODE_WALKER(self)->master);

        expr1 = car(*expr);
        expr2 = cdr(*expr);
        kind1 = _THROW_REWRITE(self, method, layer, outfh, &expr1, spec_type,
                               new_var_exprs);
        kind2 = _THROW_REWRITE(self, method, layer, outfh, &expr2, spec_type,
                               new_var_exprs);

        /* if kind1 is input, next or both */
        if ((WFF_REWRITE_FORMULA_KIND_INPUT_NEXT & kind1)
            && WFF_REWRITE_FORMULA_KIND_TEMP == kind2) {
          expr1 =
            master_wff_rewriter_create_substitution(master, method, outfh,
                                                    expr1, spec_type,
                                                    new_var_exprs);
          kind1 = WFF_REWRITE_FORMULA_KIND_STATE;
        }
        else if ((WFF_REWRITE_FORMULA_KIND_INPUT_NEXT & kind2)
                 && WFF_REWRITE_FORMULA_KIND_TEMP == kind1) {
          expr2 =
            master_wff_rewriter_create_substitution(master, method, outfh,
                                                    expr2, spec_type,
                                                    new_var_exprs);
          kind2 = WFF_REWRITE_FORMULA_KIND_STATE;
        }

        *expr = find_node(nodemgr, node_get_type(*expr), expr1, expr2);

        if (kind1 == kind2) return kind1;
        else if (WFF_REWRITE_FORMULA_KIND_TEMP == kind1 ||
                 WFF_REWRITE_FORMULA_KIND_TEMP == kind2) {
          return WFF_REWRITE_FORMULA_KIND_TEMP;
        }
        else if ((WFF_REWRITE_FORMULA_KIND_INPUT_NEXT & kind1) ||
                 (WFF_REWRITE_FORMULA_KIND_INPUT_NEXT & kind2)) {
          return (kind1 | kind2) & WFF_REWRITE_FORMULA_KIND_INPUT_NEXT;
        }
        else error_unreachable_code();
      }

    case WRESIZE:
      expr1 = car(*expr);
      expr2 = cdr(*expr);
      kind2 = _THROW_REWRITE(self, method, layer, outfh, &expr1, spec_type,
                             new_var_exprs);
      nusmv_assert(WFF_REWRITE_FORMULA_KIND_TEMP != kind2);

      *expr = find_node(nodemgr, node_get_type(*expr), expr1, expr2);

      retval = kind2;
      break;

      /* --- binary non-temporal operators ---
         it is exactly as previous case but the operands cannot have temporal
         operators. It is written as a special case only for debugging purposes.*/
    case CONS:
    case WAREAD: case WAWRITE:
    case CASE: case COLON:
    case EQUAL: case NOTEQUAL:
    case LT: case GT: case LE: case GE:
    case PLUS: case MINUS: case TIMES: case MOD: case DIVIDE:
    case UNION: case SETIN:
    case LSHIFT: case RSHIFT:
    case BIT: case CONCATENATION: case BIT_SELECTION:  case EXTEND:
    case CAST_BOOL:  case CAST_WORD1:  case CAST_SIGNED: case CAST_UNSIGNED:
    case IFTHENELSE:
      expr1 = car(*expr);
      expr2 = cdr(*expr);
      kind1 = _THROW_REWRITE(self, method, layer, outfh, &expr1, spec_type,
                             new_var_exprs);
      kind2 = _THROW_REWRITE(self, method, layer, outfh, &expr2, spec_type,
                             new_var_exprs);
      nusmv_assert(WFF_REWRITE_FORMULA_KIND_TEMP != kind1 &&
                   WFF_REWRITE_FORMULA_KIND_TEMP != kind2);

      *expr = find_node(nodemgr, node_get_type(*expr), expr1, expr2);

      if ((WFF_REWRITE_FORMULA_KIND_INPUT_NEXT & kind1) ||
          (WFF_REWRITE_FORMULA_KIND_INPUT_NEXT & kind2)) {
        retval = (kind1 | kind2) & WFF_REWRITE_FORMULA_KIND_INPUT_NEXT;
        break;
      }
      else {
        retval = WFF_REWRITE_FORMULA_KIND_STATE;
        break;
      }

      /*  -- unary temporal operators ---
          if operand has inputs then rewrite it. */
    case OP_NEXT: case OP_PREC: case OP_NOTPRECNOT: case OP_FUTURE:
    case OP_ONCE: case OP_GLOBAL: case OP_HISTORICAL:
    case BOUNDED_UNTIL: case BOUNDED_SINCE:
      expr1 = car(*expr);
      kind1 = _THROW_REWRITE(self, method, layer, outfh, &expr1, spec_type,
                             new_var_exprs);
      nusmv_assert(Nil == cdr(*expr));

      if (WFF_REWRITE_FORMULA_KIND_INPUT_NEXT & kind1) {
        const MasterWffRewriter_ptr master =
          MASTER_WFF_REWRITER(NODE_WALKER(self)->master);
        expr1 =
          master_wff_rewriter_create_substitution(master, method, outfh,
                                                  expr1, spec_type,
                                                  new_var_exprs);
      }

      *expr = find_node(nodemgr, node_get_type(*expr), expr1, Nil);

      retval =  WFF_REWRITE_FORMULA_KIND_TEMP;
      break;

      /* --- binary temporal operators ---
         If any operand has inputs then rewrite it.*/
    case UNTIL: case SINCE: case RELEASES: case TRIGGERED:
    {
      const MasterWffRewriter_ptr master =
        MASTER_WFF_REWRITER(NODE_WALKER(self)->master);
      expr1 = car(*expr);
      expr2 = cdr(*expr);
      kind1 = _THROW_REWRITE(self, method, layer, outfh, &expr1, spec_type,
                             new_var_exprs);
      kind2 = _THROW_REWRITE(self, method, layer, outfh, &expr2, spec_type,
                             new_var_exprs);

      if (WFF_REWRITE_FORMULA_KIND_INPUT_NEXT & kind1) {
        expr1 =
          master_wff_rewriter_create_substitution(master, method, outfh,
                                                  expr1, spec_type,
                                                  new_var_exprs);
      }
      if (WFF_REWRITE_FORMULA_KIND_INPUT_NEXT & kind2) {
        expr2 = master_wff_rewriter_create_substitution(master, method, outfh,
                                                        expr2, spec_type,
                                                        new_var_exprs);
      }
      *expr = find_node(nodemgr, node_get_type(*expr), expr1, expr2);

      retval =  WFF_REWRITE_FORMULA_KIND_TEMP;
    }
      break;

    default:
      {
        const StreamMgr_ptr streams =
          STREAM_MGR(NuSMVEnv_get_value(env, ENV_STREAM_MANAGER));

        StreamMgr_print_error(streams, "Error: %s:%d:%s: unexpected operator of type %d\n",
                              __FILE__, __LINE__, __func__, node_get_type(*expr));
        ErrorMgr_nusmv_exit(errmgr, 1);
      }
    } /* switch */
  }

#ifdef DEBUG_WFF_REWRITE
  {
    const MasterPrinter_ptr sexpprint =
      MASTER_PRINTER(NuSMVEnv_get_value(env, ENV_SEXP_PRINTER));

    Logger_vnlog_error(logger, sexpprint, opts, "Output: \n%N\n", *expr);
    Logger_dec_indent_size(logger);
  }
#endif

  return retval;
}


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/*!
  \brief The WffRewriterCore class virtual finalizer

  Called by the class destructor
*/
static void wff_rewriter_core_finalize(Object_ptr object, void* dummy)
{
  WffRewriterCore_ptr self = WFF_REWRITER_CORE(object);

  wff_rewriter_core_deinit(self);
  FREE(self);
}

/*!
  \brief Private copy constructor

  \sa wff_rewriter_core_copy_aux
*/
static Object_ptr wff_rewriter_core_copy(const Object_ptr self)
{
  WffRewriterCore_ptr other = ALLOC(WffRewriterCore, 1);
  TYPE_CHECKER_CHECK_INSTANCE(other);

  wff_rewriter_core_copy_aux(WFF_REWRITER_CORE(self), other);
  return OBJECT(other);
}





/**AutomaticEnd***************************************************************/
