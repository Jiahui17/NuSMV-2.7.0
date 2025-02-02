/* ---------------------------------------------------------------------------

  This file is part of the ``compile.pne'' package of NuSMV version 2.
  Copyright (C) 2014 by FBK-irst.

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
   \author Andrei Tchaltsev
   \brief Implementation of class 'PredicateExtractorCore'

   (Implementation of modularity by Elia Rigo)

   See PredicateExtractor.h for more info

   \sa PredicateExtractorCore.h
*/

#include "nusmv/core/compile/pne/MasterCompilePredicateExtractor_private.h"
#include "nusmv/core/compile/pne/PredicateExtractorBase.h"
#include "nusmv/core/compile/pne/PredicateExtractorBase_private.h"
#include "nusmv/core/compile/pne/PredicateExtractorCore.h"
#include "nusmv/core/compile/pne/PredicateExtractorCore_private.h"
#include "nusmv/core/utils/StreamMgr.h"
#include "nusmv/core/node/NodeMgr.h"
#include "nusmv/core/utils/ErrorMgr.h"
#include "nusmv/core/node/printers/MasterPrinter.h"
#include "nusmv/core/compile/pne/MasterCompilePredicateExtractor.h"
#include "nusmv/core/compile/compileInt.h"
#include "nusmv/core/parser/symbols.h"
#include "nusmv/core/utils/WordNumberMgr.h"
#include "nusmv/core/utils/utils.h"
#include "nusmv/core/utils/assoc.h"
#include "nusmv/core/utils/error.h"
#include "nusmv/core/compile/symb_table/ResolveSymbol.h"
#include "nusmv/core/utils/EnvObject_private.h"
#include "nusmv/core/parser/symbols.h"


/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
/* See 'PredicateExtractorCore_private.h' for class 'PredicateExtractorCore' definition. */

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static void predicate_extractor_core_finalize (Object_ptr object, void* dummy);

/*static Set_t pred_extract_process_recur(PredicateExtractorBase_ptr self,
                                        MasterCompilePredicateExtractorSupport_ptr support,
                                        node_ptr expr,
                                        node_ptr context);*/

static boolean pred_extract_is_bool_preds(PredicateExtractorBase_ptr self,
                                          MasterCompilePredicateExtractorSupport_ptr support,
                                          Set_t result);

static Set_t pred_extract_fix_any_preds(PredicateExtractorBase_ptr self,
                                        MasterCompilePredicateExtractorSupport_ptr support,
                                        Set_t result);

static Set_t pred_extract_apply_unary(PredicateExtractorBase_ptr self,
                                      MasterCompilePredicateExtractorSupport_ptr support,
                                      int type,
                                      Set_t childResult);

static Set_t pred_extract_apply_binary(PredicateExtractorBase_ptr self,
                                       MasterCompilePredicateExtractorSupport_ptr support,
                                       int type,
                                       Set_t leftResult,
                                       Set_t rightResult);

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

PredicateExtractorCore_ptr PredicateExtractorCore_create(const NuSMVEnv_ptr env,
                                                         const char* name)
{
  PredicateExtractorCore_ptr self = ALLOC(PredicateExtractorCore, 1);
  PREDICATE_EXTRACTOR_CORE_CHECK_INSTANCE(self);

  predicate_extractor_core_init(self, env, name,
                                NUSMV_CORE_SYMBOL_FIRST,
                                NUSMV_CORE_SYMBOL_LAST - NUSMV_CORE_SYMBOL_FIRST,
                                true /* can handle null */);
  return self;
}


void PredicateExtractorCore_destroy(PredicateExtractorCore_ptr self)
{
  PREDICATE_EXTRACTOR_CORE_CHECK_INSTANCE(self);

  Object_destroy(OBJECT(self), NULL);
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

void predicate_extractor_core_init(PredicateExtractorCore_ptr self,
                                   const NuSMVEnv_ptr env, const char* name,
                                   int low, size_t num, boolean can_handle_null)
{
  /* base class initialization */
  predicate_extractor_base_init(PREDICATE_EXTRACTOR_BASE(self), env, name,
                                low, num, can_handle_null);

  /* members initialization */

  /* virtual methods settings */
  OVERRIDE(Object, finalize) = predicate_extractor_core_finalize;
  OVERRIDE(PredicateExtractorBase, compute_preds) = predicate_extractor_core_compute_preds;
}


void predicate_extractor_core_deinit(PredicateExtractorCore_ptr self)
{
  /* members deinitialization */


  /* base class deinitialization */
  predicate_extractor_base_deinit(PREDICATE_EXTRACTOR_BASE(self));
}

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/*!
  \brief The PredicateExtractorCore class virtual finalizer

  Called by the class destructor
*/
static void predicate_extractor_core_finalize(Object_ptr object, void* dummy)
{
  PredicateExtractorCore_ptr self = PREDICATE_EXTRACTOR_CORE(object);

  predicate_extractor_core_deinit(self);
  FREE(self);
}

/*!
  \brief Performs the predicates extraction

  See predicate_extractor_core_compute_preds for more info.

This is the main function for extraction.
 The function returns the set of predicate subparts, i.e. Set_t of
   node_ptr. For expressions having whole predicates (i.e. boolean
   expressions) PREDICATES_TRUE/PREDICATES_FALSE/PREDICATES_ARBITRARY
   value is returned.

   Returned set of predicate subparts belong to support->expr2preds. The
   expression (predicates subparts are find_node-ed and belong to
   whole system).
*/
Set_t
predicate_extractor_core_compute_preds(PredicateExtractorBase_ptr self,
                                       MasterCompilePredicateExtractorSupport_ptr support,
                                       node_ptr expr,
                                       node_ptr context)
{
  const NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(self));
  const NodeMgr_ptr nodemgr =
    NODE_MGR(NuSMVEnv_get_value(env, ENV_NODE_MGR));
  const ExprMgr_ptr exprs =
    EXPR_MGR(NuSMVEnv_get_value(env, ENV_EXPR_MANAGER));

  SymbType_ptr type;
  Set_t result, left, right;
  node_ptr tmp, key;
  int node_type;
  /* from ex compute_preds function */
  int lineno_tmp;

  PREDICATE_EXTRACTOR_BASE_CHECK_INSTANCE(self);

  if (Nil == expr) return Set_MakeEmpty();

  /* new node will be created with for sure error line number */
  lineno_tmp = nusmv_yylineno;
  nusmv_yylineno = -1;
  /* end ex compute_preds function */

  nusmv_assert(Nil != expr);

  key = find_node(nodemgr, CONTEXT, context, expr);

  /* is already processed. */
  result = (Set_t) find_assoc(support->expr2preds, key);
  if (NULL != result) return result;

  type = TypeChecker_get_expression_type(support->checker, expr, context);
  nusmv_assert(!SymbType_is_error(type));

  node_type = node_get_type(expr);

  /* for sure incorrect value for debugging */
  result = left = right = (Set_t) -1;

  /* process every kind of an expression */
  switch (node_type) {
  case CONTEXT: {
    node_ptr new_ctx = CompileFlatten_concat_contexts(env, context, car(expr));

    result = _THROW(support, cdr(expr), new_ctx);
    break;
  }

    /* list of simple boolean constants => they are not part of predicates */
  case FAILURE:
    /* failures are dealt in IF and CASE expressions.  It must be deal
       there because otherwise optimizations with only TRUE/only FALSE
       predicates may become impossible */
    error_unreachable_code();
    result = PREDICATES_ARBITRARY;
    break;

  case FALSEEXP:
    result = PREDICATES_FALSE;
    break;

  case TRUEEXP:
    result = PREDICATES_TRUE;
    break;

  case NUMBER:
    /* NUMBER may be boolean and not-boolean. Here we always consider
       it as not-boolean. The outer expression will decide what to do
       with the result. */
    /* list of simple not-boolean constants => they become predicates subpart */
  case NUMBER_UNSIGNED_WORD:
  case NUMBER_SIGNED_WORD:
  case NUMBER_FRAC:
  case NUMBER_REAL:
  case NUMBER_EXP:
  case UWCONST:
  case SWCONST:
  case TWODOTS:
    tmp = ExprMgr_resolve(exprs, support->st, node_type, car(expr), cdr(expr));
    result = Set_MakeSingleton(tmp);
    break;

  case NFUNCTION: {
    node_ptr list = cdr(expr);
    result = Set_MakeEmpty();

    /* We extract the predicates from the arguments, and then we
       consider the expression */
    while (Nil != list) {
      Set_t elem = _THROW(support, car(list), context);

      if (IS_OVER_APPROX(elem)) break;
      list = cdr(list);
    }
    if (Nil != list) {
      Set_ReleaseSet(result);
      result = PREDICATES_OVERAPPROX;
    }
    else {
      if (!SymbType_is_boolean(type)) {
        /* The expression needs to be flattened here before simply
           adding it to the result */
        node_ptr fexpr = Compile_FlattenSexp(support->st, expr, context);
        result = Set_AddMember(result, (Set_Element_t)fexpr);
      }
      else if (Set_IsEmpty(result)) {
        Set_ReleaseSet(result);
        result = PREDICATES_ARBITRARY;
      }
      /* boolean expressions make predicates have arbitrary values */
      /* Arbitrary Union S --> S */
    }
    break;
  }

  case ARRAY: {
    ResolveSymbol_ptr rs;

    rs = SymbTable_resolve_symbol(support->st, expr, context);

    if (ResolveSymbol_is_undefined(rs)) {
      /* Array may be an identifier-with-brackets and may be
         expression.  Here an array-expression is detected =>
         expression is to be flattened at first to resolve array
         identifiers-with-brackets (see description of
         flattener_core_flattenas for details) and then general
         predicate extractor is to be invoked */
      node_ptr tmp = Compile_FlattenSexp(support->st, expr, context);
      nusmv_assert(tmp != expr); /* loop in recursion is impossible */
      result = _THROW(support, tmp, Nil);
      break;
    }
    else {
      /* array is actually identifier => process it with other identifiers */
    }
    /* NO BREAK HERE */
  }

  case DOT:
  case ATOM:
  case BIT: {
    /* The expression is a symbol.
       It can be a variable, a define, a constant or a parameter.
       The expression may have been flattened as well as not flattened.

       Note, that NO ERRORS CAN BE HERE, since all the error
       situations have been checked during type-checking of the
       original expression.
    */

    /* First, try to resolve the symbol */
    ResolveSymbol_ptr rs;
    node_ptr resolvedName;

    rs = SymbTable_resolve_symbol(support->st, expr, context);
    resolvedName = ResolveSymbol_get_resolved_name(rs);

    /* Check whether this is a variable */
    if (ResolveSymbol_is_var(rs)) {
      if (!SymbType_is_boolean(type)) {
        result = Set_MakeSingleton(resolvedName);
      } /* boolean vars make predicates have arbitrary values */
      else result = PREDICATES_ARBITRARY;
    }

    /* check whether is a define */
    else if (ResolveSymbol_is_define(rs)) {
      node_ptr def = SymbTable_get_define_body(support->st, resolvedName);
      node_ptr ctx = SymbTable_get_define_context(support->st, resolvedName);

      /* the context is Nil because expr is already flattened */
      result = _THROW(support, def, ctx);

      /* special case: array define may be declared with Integer (or
         higher) subtype and at the same time has a boolean element.
         In this case the boolean element has to be casted to integer.
      */
      if (ARRAY == node_type &&
          !SymbType_is_boolean(type) &&
          SymbType_is_boolean(TypeChecker_get_expression_type(support->checker,
                                                              def, ctx))) {
        /* boolean can be casted to Int, Int-Symb or their Sets only
           thus conversion to integer is enough*/
        nusmv_assert(SymbType_is_integer(type) ||
                     SymbType_is_int_symbolic_enum(type) ||
                     SYMB_TYPE_SET_INT == SymbType_get_tag(type) ||
                     SYMB_TYPE_SET_INT_SYMB == SymbType_get_tag(type));
      }
    }
    else if (ResolveSymbol_is_array(rs)) {
      result = Set_MakeSingleton(resolvedName);
    }
    /* check whether this symbol is a constant. The ResolveSymbol
       takes care of simple/complex constants */
    else if (ResolveSymbol_is_constant(rs) ||
             ResolveSymbol_is_function(rs)) {
      result = Set_MakeSingleton(resolvedName);
    }
    /* check whether this symbol is a parameter */
    else {
      node_ptr param = Nil;
      node_ptr new_ctx;

      /* it must be a parameter but being a parameter is the last
         possibility */
      nusmv_assert(ResolveSymbol_is_parameter(rs));

      param = SymbTable_get_actual_parameter(support->st, resolvedName);

      new_ctx = SymbTable_get_actual_parameter_context(support->st, resolvedName);

      result = _THROW(support, param, new_ctx);
    }

    break;
  } /* ATOM */

    /* boolean unary expression or boolean binary expressions those
       right child can be ignored and which have not to be optimized */
  case EX: case AX: case EF: case AF: case EG: case AG:
  case OP_NEXT: case OP_PREC: case OP_NOTPRECNOT: case OP_GLOBAL:
  case OP_HISTORICAL: case OP_FUTURE: case OP_ONCE:
  case BOUNDED_UNTIL: case BOUNDED_SINCE:
  case EBF: case ABF: case EBG: case ABG: /* ignore the number..number part */
  case ABU: case EBU: /* ignore the number..number part */
    nusmv_assert(SymbType_is_boolean(type)); /* only boolean can be here */
    result = _THROW(support, car(expr), context);
    result = PREDICATES_ARBITRARY;
    break;

    /* unary operations which may or may not be boolean */
  case NOT:
    nusmv_assert(Nil == cdr(expr)); /* checking that indeed no right child */
    result = PREDICATES_OVERAPPROX;
    left = _THROW(support, car(expr), context);

    /* if it is boolean => apply the operator on possible constant
       value */
    if (!IS_OVER_APPROX(left)) {
      if (SymbType_is_boolean(type)) {
        if (left == PREDICATES_TRUE) result = PREDICATES_FALSE;
        else if (left == PREDICATES_FALSE) result = PREDICATES_TRUE;
        else result = PREDICATES_ARBITRARY;
      }
      else { /* otherwise apply the bitwise operator */
        if (!IS_FLAG_VALID_PREDICATES(left)) {
          result = pred_extract_apply_unary(self, support, NOT, left);
        }
      }
    }
    break;
  case ITYPE:
    /* We do not need to recurse and there is no predicate */
    result = PREDICATES_OVERAPPROX;
    break;
    /* unary operations: cannot have boolean operand */
  case UMINUS:
  case FLOOR:
  /* [AI] adding typeof unary operator */
  case TYPEOF:
    nusmv_assert(Nil == cdr(expr));    /* checking that indeed no right child */
    result = PREDICATES_OVERAPPROX;

    left = _THROW(support, car(expr), context);

    if (!IS_OVER_APPROX(left)) {
      if (!IS_FLAG_VALID_PREDICATES(left)) {
        result = pred_extract_apply_unary(self, support, node_type, left);
      }
    }

    break;

    /* binary boolean operations which cannot be optimized.
       CONS is artificial expression, it is boolean and may have empty
       right child */
  case CONS:
  case UNTIL: case SINCE:
  case AU: case EU:
    result = PREDICATES_OVERAPPROX;

    left = _THROW(support, car(expr), context);

    if (!IS_OVER_APPROX(left)) {
      if (Nil != cdr(expr)) {
        right = _THROW(support, cdr(expr), context);
      }
      result = PREDICATES_ARBITRARY;
    }

    break;

    /* binary expression which may or may not be boolean and
       which can be optimized.
       note: children here always have the same type as the
       expression */
  case AND: case OR: case XOR: case XNOR: case IFF: case IMPLIES:

    left = _THROW(support, car(expr), context);
    result = PREDICATES_OVERAPPROX;

    if (!IS_OVER_APPROX(left)) {
      if (SymbType_is_boolean(type)) {
        /* optimization : check if the first operator result is enough */
        if ((left == PREDICATES_FALSE && node_type == AND) ||
            (left == PREDICATES_TRUE && node_type == OR)) {
          result = left;
          break;
        }
        else if (left == PREDICATES_FALSE && node_type == IMPLIES) {
          result = PREDICATES_TRUE;
          break;
        }
        /* process the second argument (as optimization did not work) */
        right = _THROW(support, cdr(expr), context);

        if (IS_OVER_APPROX(right)) {
          result = PREDICATES_OVERAPPROX;
          break;
        }

        /* compute the value if possible */
        switch (node_type) {
        case AND:
          result = (right == PREDICATES_FALSE)
            ? PREDICATES_FALSE
            : ( (left == PREDICATES_TRUE && right == PREDICATES_TRUE)
                ? PREDICATES_TRUE
                : PREDICATES_ARBITRARY
                );
          break;

        case OR:
          result = (right == PREDICATES_TRUE)
            ? PREDICATES_TRUE
            : ( (left == PREDICATES_FALSE && right == PREDICATES_FALSE)
                ? PREDICATES_FALSE
                : PREDICATES_ARBITRARY
                );
          break;

        case XOR:
          result = (left == PREDICATES_ARBITRARY || right == PREDICATES_ARBITRARY)
            ? PREDICATES_ARBITRARY
            : (left != right ? PREDICATES_TRUE : PREDICATES_FALSE);
          break;

        case XNOR:
        case IFF:
          result = (left == PREDICATES_ARBITRARY || right == PREDICATES_ARBITRARY)
            ? PREDICATES_ARBITRARY
            : (left == right ? PREDICATES_TRUE : PREDICATES_FALSE);
          break;

        case IMPLIES:
          result = (right == PREDICATES_TRUE)
            ? PREDICATES_TRUE
            : ( (left == PREDICATES_TRUE && right == PREDICATES_FALSE)
                ? PREDICATES_FALSE
                : PREDICATES_ARBITRARY
                );
          break;

        default: error_unreachable_code(); /* impossible code */
        }
        /* debug: result was set up to a constant */
        nusmv_assert(IS_FLAG_PREDICATES(result));
      }
      else {
        /* this is not a boolean => it can be only word operations,
           i.e.  apply the binary operator to results. */
        result = PREDICATES_OVERAPPROX;
        right = _THROW(support, cdr(expr), context);

        if (!IS_OVER_APPROX(right)) {
          if (!IS_FLAG_VALID_PREDICATES(left) &&
              !IS_FLAG_VALID_PREDICATES(right)) {
            result = pred_extract_apply_binary(self, support, node_type, left, right);
          }
          else {
            result = Set_MakeSingleton(expr);
          }
        }
      }
    }
    break;

    /* not-boolean unary operators */
  case CAST_UNSIGNED:
  case CAST_SIGNED:
    nusmv_assert(Nil == cdr(expr));    /* checking that indeed no right child */
    result = PREDICATES_OVERAPPROX;
    left = _THROW(support, car(expr), context);
    if (!IS_OVER_APPROX(left)) {
      if (!IS_FLAG_VALID_PREDICATES(left)) {
        result = pred_extract_apply_unary(self, support, node_type, left);
      }
      else {
        result = Set_MakeSingleton(expr);
      }
    }
    break;

    /* "next" and "init" are here as normal unary operation because
       EQDEF is a normal binary operation. No cast is done here. */
  case NEXT:
  case SMALLINIT:
    nusmv_assert(Nil == cdr(expr)); /* checking that indeed no right child */
    result = _THROW(support, car(expr), context);

    /* note that here init and next are applied without modifications,
       i.e.  next(x) := 3 will be kept as it is whereas next(x := 3)
       will be kept as x:=3 because next is applied outside of the
       predicate, not inside.

       It does not matter if we (not) get rid of "init" or "next".
    */
    if (!IS_FLAG_PREDICATES(result)) {
      /* this is not true boolean => apply the operator */
      result = pred_extract_apply_unary(self, support, node_type, result);
    }
    else {
      /* the true/false value is not changed by next/init => do nothing */
    }
    break;

    /* relational operators: they convert not-boolean to boolean  */
  case EQDEF: case SETIN:
  case EQUAL: case NOTEQUAL: case LT: case GT: case LE: case GE: {
    SymbType_ptr type1 =
      TypeChecker_get_expression_type(support->checker, car(expr), context);
    SymbType_ptr type2 =
      TypeChecker_get_expression_type(support->checker, cdr(expr), context);

    nusmv_assert(!SymbType_is_error(type1));
    nusmv_assert(!SymbType_is_error(type2));

    result = PREDICATES_OVERAPPROX;

    left = _THROW(support, car(expr), context);

    if (IS_OVER_APPROX(left)) break;

    right = _THROW(support, cdr(expr), context);
    if (IS_OVER_APPROX(right)) break;

    /* both operands are boolean (or bool-set for EQDEF and SETIN) */
    if ((SymbType_is_boolean(type1) ||
         SYMB_TYPE_SET_BOOL == SymbType_get_tag(type1)) &&
        (SymbType_is_boolean(type2) ||
         SYMB_TYPE_SET_BOOL == SymbType_get_tag(type2))) {
      /* compute the value of expression if possible */
      switch (node_type) {
      case EQDEF:
      case SETIN:
      case EQUAL:
        result = (left == PREDICATES_ARBITRARY || right == PREDICATES_ARBITRARY)
          ? PREDICATES_ARBITRARY
          : (left == right ? PREDICATES_TRUE : PREDICATES_FALSE);
        break;

      case NOTEQUAL:
        result = (left == PREDICATES_ARBITRARY || right == PREDICATES_ARBITRARY)
          ? PREDICATES_ARBITRARY
          : (left != right ? PREDICATES_TRUE : PREDICATES_FALSE);
        break;

      case GT: /* exchange right and left and jump to LT */
        {Set_t tmp = left; left = right; right = tmp;}
        /* no break here! */
      case LT:
        result = (left == PREDICATES_TRUE || right == PREDICATES_FALSE)
          ? PREDICATES_FALSE
          : (left == PREDICATES_FALSE && right == PREDICATES_TRUE)
          ? PREDICATES_TRUE
          : PREDICATES_ARBITRARY;
        break;

      case GE: /* exchange right and left and jump to LE */
        {Set_t tmp = left; left = right; right = tmp;}
        /* no break here! */
      case LE:
        result = (left == PREDICATES_FALSE || right == PREDICATES_TRUE)
          ? PREDICATES_TRUE
          : (left == PREDICATES_TRUE && right == PREDICATES_FALSE)
          ? PREDICATES_FALSE
          : PREDICATES_ARBITRARY;
        break;

      default: error_unreachable_code(); /* impossible code */
      }
    }
    else {
      /* both operands are scalar => do bool2int to cast "true
         boolean" operand (if there is one) and apply the binary
         operator */
      if (!IS_FLAG_VALID_PREDICATES(left) &&
          !IS_FLAG_VALID_PREDICATES(right)) {
        result = pred_extract_apply_binary(self, support, node_type, left, right);
      }
      else {
        result = Set_MakeSingleton(expr);
      }
      /* remember the results */
      result = pred_extract_fix_any_preds(self, support, result);
    }
    break;
  }

    /* These exprs are always scalar. No optimizations are done here. */
  case TIMES: case DIVIDE: case PLUS :case MINUS: case MOD:
  case LSHIFT: case RSHIFT: /*case LROTATE: case RROTATE: */
  case EXTEND: case WRESIZE:
  case CONST_ARRAY: {
    result = PREDICATES_OVERAPPROX;
    left = _THROW(support, car(expr), context);
    if (!IS_OVER_APPROX(left)) {
      right = _THROW(support, cdr(expr), context);
      if (!IS_FLAG_VALID_PREDICATES(left) &&
          !IS_FLAG_VALID_PREDICATES(right)) {
        result = pred_extract_apply_binary(self, support, node_type, left, right);
      }
      else {
        result = Set_MakeSingleton(expr);
      }
    }
    break;
  }

  case WAREAD: {
    result = PREDICATES_OVERAPPROX;
    if (!SymbType_is_boolean(type)) {
      left = _THROW(support, car(expr), context);
      if (!IS_OVER_APPROX(left)) {
        right = _THROW(support, cdr(expr), context);
        if (!IS_FLAG_VALID_PREDICATES(left) &&
            !IS_FLAG_VALID_PREDICATES(right)) {
          result = pred_extract_apply_binary(self, support, node_type, left, right);
        }
        else {
          result = Set_MakeSingleton(expr);
        }
      }
    }
    break;
  }

  case WAWRITE: {
    nusmv_assert(node_get_type(cdr(expr))==WAWRITE);

    result = PREDICATES_OVERAPPROX;
    left = _THROW(support, car(expr), context);
    if (!IS_OVER_APPROX(left) && !IS_FLAG_VALID_PREDICATES(left)) {
      Set_t index, value;
      index = _THROW(support, car(cdr(expr)), context);
      if (!IS_OVER_APPROX(index)) {
        value = _THROW(support, cdr(cdr(expr)), context);
        if (!IS_OVER_APPROX(value)) {
          if (!IS_FLAG_VALID_PREDICATES(index) &&
              !IS_FLAG_VALID_PREDICATES(value)) {
            right = pred_extract_apply_binary(self, support, node_type, index, value);
            result = pred_extract_apply_binary(self, support, node_type, left, right);
          }
          else {
            result = Set_MakeSingleton(expr);
          }
        }
      }
    }
    break;
  }

    /* COLON cannot go as a independent operation */
  case COLON: error_unreachable_code();

  case BIT_SELECTION: {
    /* just consistency check */
    nusmv_assert(COLON == node_get_type(cdr(expr)));
    result = PREDICATES_OVERAPPROX;
    left = _THROW(support, car(expr), context);
    if (!IS_OVER_APPROX(left)) {
      if (!IS_FLAG_VALID_PREDICATES(left)) {
        right = Set_MakeSingleton(find_node(nodemgr,
                                            COLON,
                                            find_atom(nodemgr, car(cdr(expr))),
                                            find_atom(nodemgr, cdr(cdr(expr)))));
        result = pred_extract_apply_binary(self, support, node_type, left, right);
        Set_ReleaseSet(right);
      }
      else {
        /* The expression is propagated up */
        result = Set_MakeSingleton(expr);
      }
    }
    break;
  }

  case WSIZEOF: {
    /* sizeof returns the bit-size of word expressions without evaluating
       the expression itself */
    int width;
    nusmv_assert(SymbType_is_word(type));
    width = SymbType_get_word_width(type);
    result = Set_MakeSingleton(find_node(nodemgr, NUMBER, NODE_FROM_INT(width), Nil));
    break;
  }

  case CAST_TOINT: {
    nusmv_assert(Nil == cdr(expr)); /* indeed no right child */
    result = PREDICATES_OVERAPPROX;
    left = _THROW(support, car(expr), context);
    if (!IS_OVER_APPROX(left)) {
      if (!IS_FLAG_VALID_PREDICATES(left)) {
        result = pred_extract_apply_unary(self, support, node_type, left);
      }
      else {
        node_ptr fexpr = Compile_FlattenSexp(support->st, expr, context);
        result = Set_MakeSingleton(fexpr);
      }
    }
    break;
  }

  case COUNT: {
    node_ptr list = car(expr);
    result = PREDICATES_OVERAPPROX;

    while (Nil != list) {
      Set_t elem = _THROW(support, car(list), context);

      if (IS_OVER_APPROX(elem)) break;
      list = cdr(list);
    }
    if (Nil == list) {
      node_ptr fexpr = Compile_FlattenSexp(support->st, expr, context);
      result = Set_MakeSingleton(fexpr);
    }
    break;
  }

    /* concatenation requires two word arguments (i.e. scalar).
       the only required thing is to convert bool to word1.
    */
  case CONCATENATION: {
    SymbType_ptr type1 =
      TypeChecker_get_expression_type(support->checker, car(expr), context);
    SymbType_ptr type2 =
      TypeChecker_get_expression_type(support->checker, cdr(expr), context);

    nusmv_assert(!SymbType_is_error(type1));
    nusmv_assert(!SymbType_is_error(type2));

    result = PREDICATES_OVERAPPROX;
    left = _THROW(support, car(expr), context);
    if (!IS_OVER_APPROX(left)) {
      right = _THROW(support, cdr(expr), context);

      if (!IS_OVER_APPROX(right)) {
        if (!IS_FLAG_VALID_PREDICATES(left) && !IS_FLAG_VALID_PREDICATES(right)) {
          result = pred_extract_apply_binary(self, support, node_type, left, right);
        }
        else {
          result = Set_MakeSingleton(expr);
        }
      }
    }
    break;
  }

    /* cast to bool is the same as comparison with 0d1_1 */
  case CAST_BOOL: {
    nusmv_assert(cdr(expr) == Nil); /* indeed no right child */
    result = PREDICATES_OVERAPPROX;
    left = _THROW(support, car(expr), context);

    if (!IS_OVER_APPROX(left) && !IS_FLAG_VALID_PREDICATES(left)) {
      result = pred_extract_apply_binary(self, support, EQUAL, left,
                                         support->special_word_preds[1]);
    }
    /* remember the results */
    result = pred_extract_fix_any_preds(self, support, result);
    break;
  }

  case CAST_WORD1:
    nusmv_assert(cdr(expr) == Nil);
    result = PREDICATES_OVERAPPROX;
    left = _THROW(support, car(expr), context);
    if (!IS_OVER_APPROX(left)) {
      /* create a word1 possible values */
      if (left == PREDICATES_FALSE) result = support->special_word_preds[0];
      else if (left == PREDICATES_TRUE) result = support->special_word_preds[1];
      else result = support->special_word_preds[2];
      result = Set_Copy(result); /* create a copy of special predicate set */
    }
    break;

    /* UNION just perform union of possible predicate subparts.
       The same as in PredicateNormaliser.c boolean are always casted
       to int.

       The difference from PredicateNormaliser.c is that here UNION is
       not applied. It is unclear which way to go, .e.g should "A in
       (B union C)" become "(A in B) union (A in C)" or not?
       Especially taking into account that cast from bool-set to int
       does not exist.

       NB: if this code is to be changed then change also the same
       part in PredicateNormaliser.c.
    */
  case UNION:
    left = _THROW(support, car(expr), context);
    result = PREDICATES_OVERAPPROX;

    if (!IS_OVER_APPROX(left)) {
      right = _THROW(support, cdr(expr), context);

      if (!IS_OVER_APPROX(right)) {
        if (SymbType_is_boolean(type) ||
            (SYMB_TYPE_SET_BOOL == SymbType_get_tag(type))) {
          if (left == right) {
            result = right;
          }
          else {
            result = PREDICATES_ARBITRARY;
          }
        }
        else if (!IS_FLAG_VALID_PREDICATES(left) &&
                 !IS_FLAG_VALID_PREDICATES(right)) {
          result = Set_Union(Set_Copy(left), right);
        }
        else {
          result = Set_MakeSingleton(expr);
        }
      }
    }
    break;

  case IFTHENELSE:
  case CASE: {
    node_ptr c, t, e;
    Set_t cond, then, tail;
    node_ptr simp_cond_expr;

    nusmv_assert(COLON == node_get_type(car(expr)));

    result = PREDICATES_OVERAPPROX;

    c = car(car(expr));
    t = cdr(car(expr));
    e = cdr(expr);

    /* simplification added for issue 02590: we first simplify the
     condition, by flattening it as Exp_simplfy does not support
     contextualized expressions */
    c = Compile_FlattenSexp(support->st, c, context);
    simp_cond_expr = ExprMgr_simplify(exprs, support->st, c);

    /* since simp_cond_expr is flattened, context becomes Nil here: */
    cond = _THROW(support, simp_cond_expr, Nil);

    if (IS_OVER_APPROX(cond)) break;

    /* if condition is a constant => process only one branch.
       also if tail is FAILURE => ignore it. */
    if (cond == PREDICATES_TRUE || FAILURE == node_get_type(e)) {
      result = _THROW(support, t, context);
    }
    else if (cond == PREDICATES_FALSE) {
      result = _THROW(support, e, context);
    }
    else { /* process both branches */
      /* the only remaining value */
      nusmv_assert(cond == PREDICATES_ARBITRARY);

      then = _THROW(support, t, context);

      if (!IS_OVER_APPROX(then)) {
        tail = _THROW(support, e, context);

        if (!IS_OVER_APPROX(tail)) {
          /* if expression is boolean then get rid of all predicates,
             otherwise make the union of the results */
          if (SymbType_is_boolean(type)) {
            /* optimization : both branches return the same constant =>
               return it as the value of the whole expression */
            if (then == tail) result = then;
            else result = PREDICATES_ARBITRARY;
          }
          else {
            /* make union of the results from then and tail */
            /*
              {T} Union S ---> S
              {F} Union S ---> S
              Arbitrary Union S ---> S
              Overapprox Union S ---> Overapprox
            */
            if (IS_FLAG_PREDICATES(then)) result = tail;
            else if (IS_FLAG_PREDICATES(tail)) result = then;
            else {
              result = Set_Union(Set_Copy(then), tail);
            }
          }
        }
      }
    }
    break;
  }

  case ATTIME: {
    left = _THROW(support, car(expr), context);
    /* do not normalise right operand, it is just number */
    result = PREDICATES_OVERAPPROX;

    if (!IS_OVER_APPROX(left)) {
      if (!IS_FLAG_VALID_PREDICATES(left)) {
        right = Set_MakeSingleton(find_atom(nodemgr, cdr(expr)));
        result = pred_extract_apply_binary(self, support, node_type, left, right);
        Set_ReleaseSet(right);
      }
      else {
        result = Set_MakeSingleton(expr);
      }
    }
    break;
  }

  case CAST_TO_UNSIGNED_WORD:
    result = PREDICATES_OVERAPPROX;
    left = _THROW(support, car(expr), context);
    if (!IS_OVER_APPROX(left)) {
      right = _THROW(support, cdr(expr), context);
      if (!IS_OVER_APPROX(right)) {
        if (!IS_FLAG_VALID_PREDICATES(left) &&
            !IS_FLAG_VALID_PREDICATES(right)) {
          result = pred_extract_apply_binary(self, support, node_type, left, right);
        }
        else {
          result = Set_MakeSingleton(expr);
        }
      }
    }
    break;

  default:
    /* below condition is introduced in PredicateNormalization by RC and
       copied here by AT. */
#ifndef NDEBUG
    /* Here we assume there are no other boolean operators  */
    result = PREDICATES_OVERAPPROX;
    left = _THROW(support, car(expr), context);
    if (!IS_OVER_APPROX(left)) {
      right = _THROW(support, cdr(expr), context);
      if (!IS_OVER_APPROX(right)) {
        if (!IS_FLAG_VALID_PREDICATES(left) &&
            !IS_FLAG_VALID_PREDICATES(right)) {
          result = pred_extract_apply_binary(self, support, node_type, left, right);
        }
        else {
          result = Set_MakeSingleton(expr);
        }
      }
    }
    break;
#else
    {
      const MasterPrinter_ptr sexpprinter =
        MASTER_PRINTER(NuSMVEnv_get_value(env, ENV_SEXP_PRINTER));
      const StreamMgr_ptr streams =
        STREAM_MGR(NuSMVEnv_get_value(env, ENV_STREAM_MANAGER));

      StreamMgr_nprint_error(streams, sexpprinter, "%N", expr);
      StreamMgr_print_error(streams,  "unknown token = %d\n", node_type);
      error_unreachable_code(); /* unknown kind of an expression */
    }
#endif
  } /* switch */

  /* debug that result was properly set up */
  nusmv_assert(result != (Set_t) -1);

  /* preds returned should not be one of special predicates set
     which used only to construct other predicates */
  nusmv_assert(result != support->special_word_preds[0] &&
               result != support->special_word_preds[1] &&
               result != support->special_word_preds[2]);

  /* boolean expressions may have only boolean predicate or nothing.
     not-boolean expressions always have proper predicate set */
  nusmv_assert(!(SymbType_is_boolean(type) ||
                 SYMB_TYPE_SET_BOOL == SymbType_get_tag(type)) ||
               pred_extract_is_bool_preds(self, support, result));
  nusmv_assert((SymbType_is_boolean(type) ||
                SYMB_TYPE_SET_BOOL == SymbType_get_tag(type)) ||
               !IS_FLAG_PREDICATES(result) ||
               IS_OVER_APPROX(result));

  /* remember the processed expression */
  insert_assoc(support->expr2preds, key, NODE_PTR(result));
  /* from ex compute_preds function */
  nusmv_yylineno = lineno_tmp; /* restore line number */
  /* end ex compute_preds function */
  return result;
}

/*!
  \brief This function returns true iff the result set of predicates
   subparts may belong only to boolean expression

  There 2 expressions which can be boolean and not
   boolean at the same time: "0" and 1".  These can be considered
   as predicate subparts as well as complete predicates.

   This function returns true iff the result consists of such kind
   of predicates.

  \se pred_extract_process_recur
*/
static boolean pred_extract_is_bool_preds(PredicateExtractorBase_ptr self,
                                          MasterCompilePredicateExtractorSupport_ptr support,
                                          Set_t result)
{
  Set_Iterator_t iter;
  const NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(self));
  const ExprMgr_ptr exprs = EXPR_MGR(NuSMVEnv_get_value(env, ENV_EXPR_MANAGER));

  /* a flag that expressions is truly boolean => not predicate subparts */
  if (IS_FLAG_PREDICATES(result)) return true;

  SET_FOREACH(result, iter) {
    node_ptr expr = Set_GetMember(result, iter);
    /* there may be "next" or "init" wrapping boolean predicates */
    node_ptr unnexted =
      (node_get_type(expr) == NEXT || node_get_type(expr) == SMALLINIT)
      ? car(expr) : expr;

    /* for 0 and 1 just do nothing. such predicates are useless. */
    if (!ExprMgr_is_true(exprs, unnexted) &&
        !ExprMgr_is_false(exprs, unnexted)) {
      return false;
    }
  } /* loop */

  return true;
}

/*!
  \brief This function put any expression in the set into
   "self" as complete predicates. "result" if released by this
   function.

  \se pred_extract_process_recur
*/
static Set_t pred_extract_fix_any_preds(PredicateExtractorBase_ptr self,
                                        MasterCompilePredicateExtractorSupport_ptr support,
                                        Set_t result)
{
  Set_Iterator_t iter;
  boolean there_is_0 = false;
  boolean there_is_1 = false;
  boolean there_is_arbit = false;

  /* overapproximation resolves to <T,F> */
  if (IS_OVER_APPROX(result)) return PREDICATES_ARBITRARY;

  nusmv_assert(!IS_FLAG_PREDICATES(result)); /* only proper sets are expected */

  SET_FOREACH(result, iter) {
    node_ptr expr = Set_GetMember(result, iter);

    /* optimization: skip 0 and 1, TRUE and FALSE as useless predicates. */
    if (FALSEEXP == node_get_type(expr)) {
      there_is_0 = true;
    }
    else if (TRUEEXP == node_get_type(expr)) {
      there_is_1 = true;
    }
    else {
      there_is_arbit = true;
      /* remember the obtained predicates (only if it is new) */
      if (!Set_IsMember(support->all_preds, expr)) {
        support->all_preds = Set_AddMember(support->all_preds, expr);
        support->unclustered_preds = Set_AddMember(support->unclustered_preds, expr);
      }
    }
  } /* loop */

  Set_ReleaseSet(result);

  if (there_is_0 && !there_is_1 && !there_is_arbit) return PREDICATES_FALSE;
  if (!there_is_0 && there_is_1 && !there_is_arbit) return PREDICATES_TRUE;

  return PREDICATES_ARBITRARY;
}

/*!
  \brief This function take a unary operator,
   the result of predicates extraction of the child expression
   and returns new result for the whole expression.

  This function is used only by pred_extract_process_recur.
   Created set belongs to the invoker, i.e. to pred_extract_process_recur
   which will insert them into support->expr2preds.

  \se pred_extract_process_recur
*/
static Set_t pred_extract_apply_unary(PredicateExtractorBase_ptr self,
                                      MasterCompilePredicateExtractorSupport_ptr support,
                                      int type,
                                      Set_t childResult)
{
  const NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(self));
  const ExprMgr_ptr exprs = EXPR_MGR(NuSMVEnv_get_value(env, ENV_EXPR_MANAGER));
  Set_t result;
  Set_Iterator_t iter;

  /* keep approximation */
  if (IS_OVER_APPROX(childResult)) {
    return PREDICATES_OVERAPPROX;
  }

  /* child result is properly created set of predicates to apply the operator */
  nusmv_assert(!IS_FLAG_PREDICATES(childResult));

  result = Set_MakeEmpty();

  /* apply the operator to every element of the predicate subparts */

  SET_FOREACH(childResult, iter) {
    node_ptr expr = Set_GetMember(childResult, iter);
    expr = ExprMgr_resolve(exprs, support->st, type, expr, Nil);
    result = Set_AddMember(result, expr);
  }

  return result;
}

/*!
  \brief This function take a binary operator,
   the results of predicates extraction of the children subexpressions
   and returns new result for the whole expression

  This function is used only by pred_extract_process_recur.
   Created set belongs to an invoker, pred_extract_process_recur which will
   insert them into support->expr2preds.


  \se pred_extract_process_recur
*/
static Set_t pred_extract_apply_binary(PredicateExtractorBase_ptr self,
                                       MasterCompilePredicateExtractorSupport_ptr support,
                                       int type,
                                       Set_t leftResult,
                                       Set_t rightResult)
{
  const NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(self));
  const ExprMgr_ptr exprs = EXPR_MGR(NuSMVEnv_get_value(env, ENV_EXPR_MANAGER));
  Set_t result;
  Set_Iterator_t l_iter;

  /* keep approximation */
  if (IS_OVER_APPROX(leftResult) || IS_OVER_APPROX(rightResult)) {
    return PREDICATES_OVERAPPROX;
  }

  /* children results are properly created predicates to apply the operator */
  nusmv_assert(!IS_FLAG_PREDICATES(leftResult) &&
               !IS_FLAG_PREDICATES(rightResult));

  if (support->use_approx &&
      (((size_t) Set_GiveCardinality(leftResult) *
        (size_t) Set_GiveCardinality(rightResult)) > OVER_APPROX_THRESHOLD)) {
    /* too-big: gives up */
    return PREDICATES_OVERAPPROX;
  }

  result = Set_MakeEmpty();

  /* create Cartesian produce of predicate subparts and apply the
     operator to every pair */
  SET_FOREACH(leftResult, l_iter) {
    Set_Iterator_t r_iter;
    node_ptr l_expr = Set_GetMember(leftResult, l_iter);
    nusmv_assert(Nil != l_expr); /* expression is well-formed */

    SET_FOREACH(rightResult, r_iter) {
      node_ptr r_expr = Set_GetMember(rightResult, r_iter);
      nusmv_assert(Nil != r_expr); /* expression is well-formed */

      result = Set_AddMember(result,
                             ExprMgr_resolve(exprs, support->st,
                                             type, l_expr, r_expr));
    }
  }
  return result;
}
