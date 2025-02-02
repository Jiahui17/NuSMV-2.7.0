/* ---------------------------------------------------------------------------

  This file is part of the ``compile.type_checking.checkers'' package
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
  \brief Implementaion of class 'TypeInExprCore'

  \todo: Missing description

*/


#include "nusmv/core/utils/ErrorMgr.h"
#include "nusmv/core/compile/type_checking/type_in_expr_walkers/TypeInExprCore.h"
#include "nusmv/core/compile/type_checking/type_in_expr_walkers/TypeInExprCore_private.h"
#include "nusmv/core/compile/type_checking/TypeChecker_private.h"

#include "nusmv/core/parser/symbols.h"
#include "nusmv/core/utils/utils.h"
#include "nusmv/core/utils/error.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
/* See 'TypeInExprCore_private.h' for class 'TypeInExprCore' definition.     */

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static void type_in_expr_core_finalize(Object_ptr object, void* dummy);

static Object_ptr type_in_expr_core_copy(const Object_ptr self);

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

TypeInExprCore_ptr TypeInExprCore_create(const TypeChecker_ptr type_checker)
{
  const TypeInExprCore_ptr self = ALLOC(TypeInExprCore, 1);
  TYPE_IN_EXPR_CORE_CHECK_INSTANCE(self);

  type_in_expr_core_init(self, type_checker, TYPE_IN_EXPR_CORE_NAME,
                         NUSMV_CORE_SYMBOL_FIRST,
                         NUSMV_CORE_SYMBOL_LAST - NUSMV_CORE_SYMBOL_FIRST,
                         true /* does handle Nil */);

  return self;
}

void TypeInExprCore_destroy(TypeInExprCore_ptr self)
{
  TYPE_IN_EXPR_CORE_CHECK_INSTANCE(self);

  Object_destroy(OBJECT(self), NULL);
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

void type_in_expr_core_init(const TypeInExprCore_ptr self,
                            const TypeChecker_ptr type_checker,
                            const char* name, const int low, const size_t num,
                            const boolean can_handle_null)
{
  const NuSMVEnv_ptr env = ENV_OBJECT_GET_ENV(type_checker);
  const NodeMgr_ptr node_mgr = NODE_MGR(NuSMVEnv_get_value(env, ENV_NODE_MGR));
  type_in_expr_base_init(TYPE_IN_EXPR_BASE(self), type_checker, name, low, num,
                         can_handle_null);

  /* members initialisation */
  self->cache = new_assoc();
  self->last_seach_type = SYMB_TYPE(NULL);
  self->marker = NodeMgr_new_lined_node(node_mgr, CONS, Nil, Nil, -1);

  /* virtual methods settings */
  OVERRIDE(Object, finalize) = type_in_expr_core_finalize;
  OVERRIDE(Object, copy) = type_in_expr_core_copy;
  OVERRIDE(TypeInExprBase, find_type_in_expr) =
    type_in_expr_core_find_type_in_expr;
}


void type_in_expr_core_deinit(TypeInExprCore_ptr self)
{
  if (self->cache != (hash_ptr)NULL)
    free_assoc(self->cache);
  if (self->marker != Nil) {
    const NuSMVEnv_ptr env = ENV_OBJECT_GET_ENV(self);
    const NodeMgr_ptr node_mgr = NODE_MGR(NuSMVEnv_get_value(env, ENV_NODE_MGR));
    NodeMgr_free_node(node_mgr, self->marker);
  }

  type_in_expr_base_deinit(TYPE_IN_EXPR_BASE(self));
}


void type_in_expr_core_copy_aux(const TypeInExprCore_ptr self,
                                TypeInExprCore_ptr other)
{
  const NuSMVEnv_ptr env = ENV_OBJECT_GET_ENV(self);
  const NodeMgr_ptr node_mgr = NODE_MGR(NuSMVEnv_get_value(env, ENV_NODE_MGR));
  /* base class copy constructor */
  type_in_expr_base_copy_aux(TYPE_IN_EXPR_BASE(self), TYPE_IN_EXPR_BASE(other));

  /* members copy */
  other->cache = copy_assoc(self->cache);
  other->last_seach_type = self->last_seach_type;
  other->marker = NodeMgr_new_lined_node(node_mgr, node_get_type(self->marker),
                                         car(self->marker),
                                         cdr(self->marker),
                                         node_get_lineno(self->marker));
}

node_ptr type_in_expr_core_find_type_in_expr(const TypeInExprBase_ptr _self,
                                             const node_ptr expr,
                                             const node_ptr context,
                                             const SymbType_ptr type)
{
  const TypeInExprCore_ptr self = TYPE_IN_EXPR_CORE(_self);
  const NuSMVEnv_ptr env = ENV_OBJECT_GET_ENV(self);
  const NodeMgr_ptr node_mgr = NODE_MGR(NuSMVEnv_get_value(env, ENV_NODE_MGR));
  const TypeChecker_ptr type_checker = type_in_expr_base_get_type_checker(_self);

  const node_ptr ctx_expr =
    (context == Nil)? expr : find_node(node_mgr, CONTEXT, context, expr);

  node_ptr retval = Nil;

  TYPE_CHECKER_CHECK_INSTANCE(type_checker);

  if (expr == Nil)
    return retval;

  nusmv_assert(self->cache != (hash_ptr)NULL);
  nusmv_assert(expr != Nil);
  nusmv_assert(!type_in_expr_core_is_marker(self, expr));

  /* if type changed, reset cache */
  if (self->last_seach_type != SYMB_TYPE(NULL) &&
      !SymbType_equals(self->last_seach_type, type)) {
    clear_assoc(self->cache);
  }
  else {
    retval = find_assoc(self->cache, ctx_expr);
    if (retval != Nil) {
      return type_in_expr_core_is_marker(self, retval)? Nil : retval;
    }
  }

  switch(node_get_type(expr)) {
     /* leafs, end of recursion */
    case FAILURE:
    case FALSEEXP:
    case TRUEEXP:
    case NUMBER:
    case NUMBER_UNSIGNED_WORD:
    case NUMBER_SIGNED_WORD:
    case NUMBER_FRAC:
    case NUMBER_REAL:
    case NUMBER_EXP:
    case UWCONST:
    case SWCONST:
    case TWODOTS:
    case SELF:
    case ARRAY:
    case COUNT:
    case BIT_SELECTION:
    case ITYPE:
      {
        const SymbType_ptr expr_type =
          type_checker_check_expression(type_checker, expr, context);
        retval = SymbType_equals(type, expr_type)? expr : Nil;
      }
      break;

  case DOT:
  case ATOM:
    {
      const SymbType_ptr expr_type = tc_lookup_expr_type(type_checker, expr);
      if (expr_type != nullType) {
        retval = SymbType_equals(type, expr_type)? expr : Nil;
        break;
      }
      {
        const SymbTable_ptr symb_table =
          TypeChecker_get_symb_table(type_checker);
        const ResolveSymbol_ptr rs =
          SymbTable_resolve_symbol(symb_table, expr, context);
        const node_ptr resolvedName = ResolveSymbol_get_resolved_name(rs);

        /* An identifier has not been declared => error */
        if (ResolveSymbol_is_undefined(rs)) {
          error_unreachable_code();
        }

        /* An identifier has more than one meaning => error */
        if (ResolveSymbol_is_ambiguous(rs)) {
          error_unreachable_code();
        }

        /* this is a constant */
        if (ResolveSymbol_is_constant(rs)) {
          const SymbType_ptr expr_type =
            type_checker_check_expression(type_checker, expr, context);
          retval = SymbType_equals(type, expr_type)? expr : Nil;
          break;
        }

        /* this is a parameter */
        if (ResolveSymbol_is_parameter(rs)) {
          const node_ptr parameter =
            SymbTable_get_flatten_actual_parameter(symb_table, resolvedName);
          const SymbType_ptr expr_type =
            type_checker_check_expression(type_checker, parameter, context);
          retval = SymbType_equals(type, expr_type)? expr : Nil;
          break;
        }

        /* this is a define */
        if (ResolveSymbol_is_define(rs)) {
          /* the bodies of defines are cached(in BddEnc).
             So, the type of expressions will also be correctly cached.
          */
          const node_ptr bodyDef =
            SymbTable_get_define_body(symb_table, resolvedName);
          const node_ptr ctxDef =
            SymbTable_get_define_context(symb_table, resolvedName);
          /* the context is Nil because expr is already flattened */
          const SymbType_ptr expr_type =
            type_checker_check_expression(type_checker, bodyDef, ctxDef);
          retval = SymbType_equals(type, expr_type)? expr : Nil;
          break;
        }

        /* this is a array define */
        if (ResolveSymbol_is_array_def(rs)) {

          const node_ptr arraydef =
            SymbTable_get_array_define_body(symb_table, resolvedName);
          const node_ptr array_ctx =
            SymbTable_get_array_define_context(symb_table, resolvedName);
          const SymbType_ptr expr_type =
            type_checker_check_expression(type_checker, arraydef, array_ctx);
          retval = SymbType_equals(type, expr_type)? expr : Nil;
          break;
        }

        /* An array type */
        if (ResolveSymbol_is_array(rs)) {
          SymbType_ptr expr_type =
            SymbTable_get_variable_array_type(symb_table, resolvedName);

          /* Here we share the variable's type to allow
             comparison of pointers instead of content of the types
          */
          expr_type = SymbType_make_memory_shared(expr_type);
          retval = SymbType_equals(type, expr_type)? expr : Nil;
          break;
        }

        /* this is a function */
        if (ResolveSymbol_is_function(rs)) {
          const SymbType_ptr expr_type =
            SymbTable_get_function_type(symb_table, resolvedName);
          retval = SymbType_equals(type, expr_type)? expr : Nil;
          break;
        }

        /* This is a variable */
        {
          SymbType_ptr expr_type;
          nusmv_assert(ResolveSymbol_is_var(rs));
          expr_type = SymbTable_get_var_type(symb_table, resolvedName);
          /* Convert the syntactic type to a symbol type.
             Here we share the variable's type to allow
             comparison of pointers instead of content of the types
          */
          expr_type = SymbType_make_memory_shared(expr_type);
          retval = SymbType_equals(type, expr_type)? expr : Nil;
          break;
        }
        error_unreachable_code();
      }
    } /* case ATOM, DOT */
    break;

    /* unary operators (left) */
  case CAST_BOOL:
  case CAST_WORD1:
  case CAST_SIGNED:
  case CAST_UNSIGNED:
  case WSIZEOF:
  case CAST_TOINT:
  case TYPEOF:
  case NOT:
  case UMINUS:
  case NEXT:
  case EX: case AX: case EF: case AF: case EG: case AG:
  case ABU: case EBU:
  case EBF: case ABF: case EBG: case ABG:
  case OP_NEXT: case OP_PREC: case OP_NOTPRECNOT: case OP_GLOBAL:
  case OP_HISTORICAL: case OP_FUTURE: case OP_ONCE:
  case BOUNDED_UNTIL: case BOUNDED_SINCE:
    {
      /* get current expression type, check if it matches
         what we're looking for, otherwise keep searching on
         subexpression (left) */
      const SymbType_ptr expr_type =
        type_checker_check_expression(type_checker, expr, context);
      retval = SymbType_equals(type, expr_type)?
        expr :
        _THROW(_self, car(expr), context, type);
    }
    break;

    /* binary operators */
  case COLON:
    {
    /* checkers do not assign a type to colon, can not ask for the colon type;
       type is the one given by the subexpressions */
      retval =
        _THROW(_self, car(expr), context, type);

      if (retval == Nil)
        retval =
          _THROW(_self, cdr(expr), context, type);
    }
    break;

    case CASE:
    case CONCATENATION:
    case TIMES: case DIVIDE: case PLUS :case MINUS: case MOD:
    case LSHIFT: case RSHIFT: case LROTATE: case RROTATE:
    case WAREAD: case WAWRITE:
    case UNION: case SETIN:
    case EQUAL: case NOTEQUAL: case LT: case GT: case LE: case GE:
    case IFTHENELSE:
    case EXTEND:
    case WRESIZE:
    case CONST_ARRAY:
    case CAST_TO_UNSIGNED_WORD:
    case AND: case OR: case XOR: case XNOR: case IFF: case IMPLIES:
    case AU: case EU:
    case UNTIL: case SINCE:
      {
        /* get current expression type, check if it matches
           what we're looking for, otherwise keep searching on
           subexpressions (left and right) */
        const SymbType_ptr expr_type =
          type_checker_check_expression(type_checker, expr, context);
        if (SymbType_equals(type, expr_type)) {
          retval = expr;
        }
        else {
          retval = _THROW(_self, car(expr), context, type);
          if (retval == Nil)
            retval = _THROW(_self, cdr(expr), context, type);
        }
      }
      break;

  default:
    retval = type_in_expr_base_find_type_in_expr(_self, expr, context, type);
  }

  insert_assoc(self->cache, ctx_expr,
               (retval == Nil)? self->marker : retval);

  return retval;
}


/*!
  \brief Return true iff the given node is equal to the marker node.
*/
boolean type_in_expr_core_is_marker(const TypeInExprCore_ptr self,
                                    const node_ptr node)
{
  const node_ptr marker = self->marker;

  assert(car(marker) == Nil && cdr(marker) == Nil);

  if (marker == node ||
      (node_get_type(marker) == node_get_type(node) &&
       car(marker) == car(node) && cdr(marker) == cdr(node) &&
       node_get_lineno(marker) == node_get_lineno(node)))
    return true;

  return false;
}


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/*!
  \brief The TypeInExprCore class virtual finalizer

  Called by the class destructor
*/
static void type_in_expr_core_finalize(Object_ptr object, void* dummy)
{
  TypeInExprCore_ptr self = TYPE_IN_EXPR_CORE(object);

  type_in_expr_core_deinit(self);
  FREE(self);
}

/*!
  \brief Private copy constructor

  \sa node_walker_copy_aux
*/
static Object_ptr type_in_expr_core_copy(const Object_ptr self)
{
  const TypeInExprCore_ptr other = ALLOC(TypeInExprCore, 1);
  TYPE_IN_EXPR_CORE_CHECK_INSTANCE(other);

  type_in_expr_core_copy_aux(TYPE_IN_EXPR_CORE(self), other);

  return OBJECT(other);
}
