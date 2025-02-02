/* ---------------------------------------------------------------------------


This file is part of the ``compile.type_checking'' package of NuSMV
version 2. Copyright (C) 2005 by FBK-irst.

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
  \author Andrei Tchaltsev, Roberto Cavada
  \brief Implementation of class 'TypeChecker'

  \todo: Missing description

*/


#include "nusmv/core/utils/StreamMgr.h"
#include "nusmv/core/utils/Logger.h"
#include "nusmv/core/node/NodeMgr.h"
#include "nusmv/core/node/printers/MasterPrinter.h"
#include "nusmv/core/compile/type_checking/TypeChecker_private.h"
#include "nusmv/core/compile/type_checking/checkers/CheckerBase.h"
#include "nusmv/core/compile/type_checking/checkers/CheckerCore.h"
#include "nusmv/core/compile/type_checking/checkers/CheckerStatement.h"
#include "nusmv/core/compile/type_checking/checkers/CheckerPsl.h"
#include "nusmv/core/compile/type_checking/checkers/CheckerPsl_private.h"
#include "nusmv/core/compile/type_checking/type_in_expr_walkers/TypeInExprBase.h"
#include "nusmv/core/compile/type_checking/type_in_expr_walkers/TypeInExprCore.h"

#include "nusmv/core/utils/WordNumberMgr.h"
#include "nusmv/core/prob/prop/Prop.h"
#include "nusmv/core/compile/symb_table/symb_table.h"
#include "nusmv/core/parser/symbols.h"
#include "nusmv/core/utils/utils.h"
#include "nusmv/core/utils/ustring.h"
#include "nusmv/core/utils/assoc.h"
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
/* See 'TypeChecker_private.h' for class 'TypeChecker' definition. */


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

static void type_checker_init(TypeChecker_ptr self,
                              SymbTable_ptr symbolTable);
static void type_checker_deinit(TypeChecker_ptr self);

static void type_checker_finalize(Object_ptr object, void* dummy);

static boolean
type_checker_check_constraint_list(TypeChecker_ptr self,
                                  int kind, node_ptr expressions,
                                  boolean allow_clock_left,
                                  boolean allow_clock_right);

static boolean type_checker_check_constraint_list_is_leaf(const node_ptr exp,
                                                         const int kind);

static void type_checker_memoizing_force_enabled(TypeChecker_ptr self);

static void type_checker_reset_memoizing(TypeChecker_ptr self);

static void type_checker_remove_symbol_trigger(const SymbTable_ptr st,
                                               const node_ptr sym,
                                               SymbTableTriggerAction action,
                                               void* arg);

static SymbType_ptr type_checker_check_timed_expression(TypeChecker_ptr self,
                                                        node_ptr expression,
                                                        node_ptr context,
                                                        boolean allow_clock);

static boolean type_checker_is_timed_specification_wellformed(TypeChecker_ptr self,
                                                              node_ptr expression,
                                                              boolean allow_clock);

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

TypeChecker_ptr TypeChecker_create(SymbTable_ptr symbolTable)
{
  TypeChecker_ptr self = ALLOC(TypeChecker, 1);
  TYPE_CHECKER_CHECK_INSTANCE(self);

  type_checker_init(self, symbolTable);
  return self;
}

TypeChecker_ptr
TypeChecker_create_with_default_checkers(SymbTable_ptr symbolTable)
{
  const TypeChecker_ptr self = TypeChecker_create(symbolTable);
  const NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(symbolTable));
  NodeWalker_ptr walker;

  /* checkers creation and registration */
  walker = NODE_WALKER(CheckerCore_create(env)); /* core */
  MasterNodeWalker_register_walker(MASTER_NODE_WALKER(self), walker);

  walker = NODE_WALKER(CheckerStatement_create(env)); /* statements */
  MasterNodeWalker_register_walker(MASTER_NODE_WALKER(self), walker);

  walker = NODE_WALKER(CheckerPsl_create(env)); /* psl */
  MasterNodeWalker_register_walker(MASTER_NODE_WALKER(self), walker);

  /* internal walker */
  walker = NODE_WALKER(TypeInExprCore_create(self));
  MasterNodeWalker_register_walker(self->type_in_expr_walkers, walker);

  return self;
}

void TypeChecker_destroy(TypeChecker_ptr self)
{
  TYPE_CHECKER_CHECK_INSTANCE(self);
  Object_destroy(OBJECT(self), NULL);
}

TypeChecker_ptr
TypeChecker_copy(const TypeChecker_ptr self, SymbTable_ptr symbolTable)
{
  const TypeChecker_ptr other = ALLOC(TypeChecker, 1);

  TYPE_CHECKER_CHECK_INSTANCE(self);
  TYPE_CHECKER_CHECK_INSTANCE(other);
  SYMB_TABLE_CHECK_INSTANCE(symbolTable);
  /* avoid sharing of symbol table */
  nusmv_assert(TypeChecker_get_symb_table(self) != symbolTable);
  /* same environment */
  nusmv_assert(ENV_OBJECT_GET_ENV(symbolTable) == ENV_OBJECT_GET_ENV(self));

  type_checker_copy_aux(self, other, symbolTable);

  return other;
}

SymbTable_ptr TypeChecker_get_symb_table(TypeChecker_ptr self)
{
  TYPE_CHECKER_CHECK_INSTANCE(self);
  return self->symbolTable;
}

boolean
TypeChecker_check_symb_table(TypeChecker_ptr self)
{
  boolean isOK = true;
  SymbTable_ptr table;
  ListIter_ptr iter;
  NodeList_ptr layers;

  table = TypeChecker_get_symb_table(self);
  layers = SymbTable_get_layers(table);

  NODE_LIST_FOREACH(layers, iter) {
    SymbLayer_ptr layer = SYMB_LAYER(NodeList_get_elem_at(layers, iter));

    isOK = TypeChecker_check_layer(self, layer);
    if (! isOK) break;
  }

  return isOK;
}

boolean TypeChecker_check_layer(TypeChecker_ptr self,
                                SymbLayer_ptr layer)
{
  boolean isOK;
  SymbTable_ptr table;
  SymbLayerIter liter;

  TYPE_CHECKER_CHECK_INSTANCE(self);

  table = TypeChecker_get_symb_table(self);

  /* the type checker's symbol table should contain the given layer */
  nusmv_assert(SymbTable_get_layer(table, SymbLayer_get_name(layer)) == layer);

  isOK = true;
  {
    const NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(self));
    const NodeMgr_ptr nodemgr =
      NODE_MGR(NuSMVEnv_get_value(env, ENV_NODE_MGR));

    SYMB_LAYER_FOREACH(layer, liter, STT_VAR) {
      node_ptr varName = SymbLayer_iter_get_symbol(layer, &liter);
      SymbType_ptr type = SymbTable_get_var_type(table, varName);
      isOK = TypeChecker_is_type_wellformed(self, type,  varName) && isOK;
    }

    if (!isOK) return false;


    SYMB_LAYER_FOREACH(layer, liter, STT_DEFINE | STT_ARRAY_DEFINE) {
      node_ptr defName = SymbLayer_iter_get_symbol(layer, &liter);

      /* wrap in DEFINE because it is required by the function invoked */
      isOK = TypeChecker_is_specification_wellformed(self,
                                                     find_node(nodemgr, DEFINE, defName, Nil))
        && isOK;
    }
  }
  return isOK;
}

boolean TypeChecker_check_constraints(TypeChecker_ptr self,
                                      node_ptr init, node_ptr trans,
                                      node_ptr invar, node_ptr assign,
                                      node_ptr urgent, node_ptr justice,
                                      node_ptr compassion)
{
  NuSMVEnv_ptr env;
  OptsHandler_ptr opts;

  boolean isOK;

  TYPE_CHECKER_CHECK_INSTANCE(self);

  env = EnvObject_get_environment(ENV_OBJECT(self));
  opts = OPTS_HANDLER(NuSMVEnv_get_value(env, ENV_OPTS_HANDLER));

  isOK = true;

  /* check INIT */
  if (!type_checker_check_constraint_list(self, INIT, init, true, true)) {
    isOK = false;
  }

  /* check TRANS */
  if (!type_checker_check_constraint_list(self, TRANS, trans, true, true)) {
    isOK = false;
  }

  /* check INVAR */
  if (!type_checker_check_constraint_list(self, INVAR, invar, true, true)) {
    isOK = false;
  }

  /* check ASSIGN */
  if (!type_checker_check_constraint_list(self, ASSIGN, assign, true, true)) {
    isOK = false;
  }

  /* check URGENT, Clock type not allowed */
  if (!type_checker_check_constraint_list(self, URGENT, urgent, false, false)) {
    isOK = false;
  }

  /* check  JUSTICE */
  if (!type_checker_check_constraint_list(self, JUSTICE, justice, true, true)) {
    isOK = false;
  }

  /* check  COMPASSION */
  if (!type_checker_check_constraint_list(self, COMPASSION, compassion, true, true)) {
    isOK = false;
  }

  if (opt_verbose_level_gt(opts, 3)) {
    Logger_ptr logger = LOGGER(NuSMVEnv_get_value(env, ENV_LOGGER));
    if (isOK) Logger_log(logger,
                      "Successful type-checking of the module constraints\n");
  }

  return isOK;
}

boolean TypeChecker_check_property(TypeChecker_ptr self,
                                   Prop_ptr property)
{
  int kind;
  boolean isOK;
  node_ptr exp;

  TYPE_CHECKER_CHECK_INSTANCE(self);

  {
    const NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(self));
    const NodeMgr_ptr nodemgr =
      NODE_MGR(NuSMVEnv_get_value(env, ENV_NODE_MGR));
    const OptsHandler_ptr opts =
      OPTS_HANDLER(NuSMVEnv_get_value(env, ENV_OPTS_HANDLER));


    switch (Prop_get_type(property)) {
    case Prop_NoType:  error_unreachable_code(); /* incorrect property */
    case Prop_Ctl:     kind = SPEC;      break;
    case Prop_Ltl:     kind = LTLSPEC;   break;
    case Prop_Psl:     kind = PSLSPEC;   break;
    case Prop_Invar:   kind = INVARSPEC; break;
    case Prop_Compute: kind = COMPUTE;   break;

    default:           error_unreachable_code();
    } /* switch */

    nusmv_yylineno = node_get_lineno(Prop_get_expr(property));
    exp = find_node(nodemgr, kind, Prop_get_expr(property), Nil);

    isOK = TypeChecker_is_specification_wellformed(self, exp);

    if (opt_verbose_level_gt(opts, 3)) {
      Logger_ptr logger = LOGGER(NuSMVEnv_get_value(env, ENV_LOGGER));
      if (isOK) {
        /* the property is not yet inserted to database => there is no index */
        Logger_log(logger, "Successful type-checking of a property\n");
      }
    }

  }
  return isOK;
}

boolean TypeChecker_is_expression_wellformed(TypeChecker_ptr self,
                                             node_ptr expression,
                                             node_ptr context)
{
  SymbType_ptr type;

  /* WARNING [MD] Here is missing an assertion about the nodetype of
     expression */

  /* enables memoizing */
  type_checker_memoizing_force_enabled(self);

  /* Nil expressions in NuSMV often correspond to TRUE value => no violations */
  if (Nil == expression) return true;

  type = type_checker_check_expression(self, expression, context);
  if (SymbType_is_error(type)) {
    /* an error occurred, resets the type memoizing hash */
    type_checker_reset_memoizing(self);
    return false;
  }
  return true;
}

boolean TypeChecker_is_specification_wellformed(TypeChecker_ptr self,
                                                node_ptr expression)
{
  return type_checker_is_timed_specification_wellformed(self, expression, true);
}

static boolean type_checker_is_timed_specification_wellformed(TypeChecker_ptr self,
                                                              node_ptr expression,
                                                              boolean allow_clock)
{
  NuSMVEnv_ptr env;
  SymbType_ptr type;

  TYPE_CHECKER_CHECK_INSTANCE(self);
  /* WARNING [MD] Here is missing an assertion about the nodetype of
     expression */

  env = EnvObject_get_environment(ENV_OBJECT(self));

  /* enables memoizing */
  type_checker_memoizing_force_enabled(self);

  type = tc_lookup_expr_type(self, expression);
  /* the _whole_ expression has been already checked */
  if (nullType != type) return (type != SymbTablePkg_error_type(env));

  type = type_checker_check_timed_expression(self, expression, Nil, allow_clock);
  if (SymbType_is_error(type)) {
    /* an error occurred, resets the type memoizing hash */
    type_checker_reset_memoizing(self);
    return false;
  }

  return true;
}



boolean TypeChecker_is_type_wellformed(TypeChecker_ptr self,
                                       SymbType_ptr type,
                                       node_ptr varName)
{
  TYPE_CHECKER_CHECK_INSTANCE(self);

  {
    const NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(self));
    const NodeMgr_ptr nodemgr =
      NODE_MGR(NuSMVEnv_get_value(env, ENV_NODE_MGR));


    switch (SymbType_get_tag(type)) {
    case SYMB_TYPE_BOOLEAN: break;

    case SYMB_TYPE_ENUM: {
      /* check that no value is repeated */
      /* Here, we assume that 'constant_list' in parser/grammar.y makes
         the values in the list unique (with the help of find_atom or find_node)
         but the list itself is not unique and contains correct line info.
      */

      node_ptr values = SymbType_get_enum_type_values(type);
      NodeList_ptr list = NodeList_create_from_list(values);
      ListIter_ptr iter = NodeList_get_first_iter(list);
      while (!ListIter_is_end(iter)) {
        if ((NodeList_count_elem(list, NodeList_get_elem_at(list, iter)) > 1) &&
            (type_checker_viol_handler(self,
                                       TC_VIOLATION_DUPLICATE_CONSTANTS,
                                       new_lined_node(nodemgr, CONS, varName, values,
                                                      node_get_lineno(values))))) {
          return false;
        }

        iter = ListIter_get_next(iter);
      }

      NodeList_destroy(list);
      break;
    }

    case SYMB_TYPE_UNSIGNED_WORD:
    case SYMB_TYPE_SIGNED_WORD: {
      int width = SymbType_get_word_width(type);
      if (0 < width && width <= WordNumber_max_width()) {
        return true;
      }

      if ( type_checker_viol_handler(self,
                                     TC_VIOLATION_INCORRECT_WORD_WIDTH,
                                     new_lined_node(nodemgr, CONS, varName,
                                                    new_node(nodemgr, NUMBER, NODE_FROM_INT(width), Nil),
                                                    SymbType_get_word_line_number(type))) ) {
        return false;
      }
      break;
    }

      /* Array & matrix type */
    case SYMB_TYPE_ARRAY: {
#if !defined(_MSC_VER)
#warning [AMi] : Probably we have to check the correctness of the array tree shape
#endif
      break;
    }

    case SYMB_TYPE_WORDARRAY: {
      int awidth;

      awidth = SymbType_get_wordarray_awidth(type);

      if (!(0 < awidth && awidth <= WordNumber_max_width())) {
        if ( type_checker_viol_handler(self,
                                       TC_VIOLATION_INCORRECT_WORDARRAY_WIDTH,
                                       new_lined_node(nodemgr, CONS, varName,
                                                      new_node(nodemgr, NUMBER, NODE_FROM_INT(awidth),
                                                               Nil),
                                                      SymbType_get_word_line_number(type))) ) {
          return false;
        }
      }

      break;
    }

    case SYMB_TYPE_INTARRAY:

    case SYMB_TYPE_CLOCK:
    case SYMB_TYPE_INTEGER:  /* (infinite-precision) integer */
    case SYMB_TYPE_REAL: /* (infinite-precision) rational */
    case SYMB_TYPE_CONTINUOUS:
      break;

    case SYMB_TYPE_NONE: /* no-type */
    case SYMB_TYPE_STATEMENT: /* statement */
    case SYMB_TYPE_SET_BOOL:  /* a set of integer values */
    case SYMB_TYPE_SET_INT:  /* a set of integer values */
    case SYMB_TYPE_SET_SYMB: /* a set of symbolic values */
    case SYMB_TYPE_SET_INT_SYMB:/* a set of symbolic and integer values */
    case SYMB_TYPE_ERROR: /* indicates an error */
    default:
      error_unreachable_code(); /* a variable cannot have these types */
    }
  }

  return true;
}

SymbType_ptr TypeChecker_get_expression_type(TypeChecker_ptr self,
                                             node_ptr expression,
                                             node_ptr context)
{
  TYPE_CHECKER_CHECK_INSTANCE(self);

  {
    const NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(self));
    const NodeMgr_ptr nodemgr =
      NODE_MGR(NuSMVEnv_get_value(env, ENV_NODE_MGR));

    SymbType_ptr res;
    node_ptr ctx_expr;

    /* wrap exp into context if context is not Nil */
    if (Nil != context) ctx_expr = find_node(nodemgr, CONTEXT, context, expression);
    else ctx_expr = expression;

    res = tc_lookup_expr_type(self, ctx_expr);

    if (res == nullType) {
      res = type_checker_check_expression(self, expression, context);
    }

    return res;
  }
}

boolean TypeChecker_is_expression_type_checked(TypeChecker_ptr self,
                                               node_ptr expression,
                                               node_ptr context)
{
  const NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(self));
  const NodeMgr_ptr nodemgr =
    NODE_MGR(NuSMVEnv_get_value(env, ENV_NODE_MGR));
  SymbType_ptr res;
  node_ptr ctx_expr;

  /* wrap exp into context if context is not Nil */
  if (Nil != context) ctx_expr = find_node(nodemgr, CONTEXT, context, expression);
  else ctx_expr = expression;

  res = tc_lookup_expr_type(self, ctx_expr);

  return res != nullType;
}

node_ptr TypeChecker_find_type_in_expr(const TypeChecker_ptr self,
                                       const node_ptr expression,
                                       const node_ptr context,
                                       const SymbType_ptr type)
{
  const MasterNodeWalker_ptr master_walker = self->type_in_expr_walkers;
  const NodeList_ptr walkers = master_walker->walkers;
  ListIter_ptr iter = NodeList_get_first_iter(walkers);
    while (!ListIter_is_end(iter)) {
      const TypeInExprBase_ptr walker =
        TYPE_IN_EXPR_BASE(NodeList_get_elem_at(walkers, iter));
      if (NodeWalker_can_handle(NODE_WALKER(walker), expression)) {
        return TypeInExprBase_find_type_in_expr(walker, expression, context, type);
      }
      iter = ListIter_get_next(iter);
    }

    error_unreachable_code();
}



/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/
/*!
  \brief Private copy constructor

  This will copy self's symbol table reference into other.
  Since this may be not desiderable, there is special method
  TypeChecker_copy which also sets a different symbol table.

  \sa TypeChecker_copy
*/
void type_checker_copy_aux(const TypeChecker_ptr self,
                           const TypeChecker_ptr other,
                           const SymbTable_ptr other_st)
{
  /* base class copy constructor */
  master_node_walker_copy_aux(MASTER_NODE_WALKER(self),
                              MASTER_NODE_WALKER(other));

  /* members copy */
  other->memoizing_counter = self->memoizing_counter;

  /* do not copy the cache of expression types to save space in memory,
     initialize the other cache to a newly create map. */
  other->expressionTypeMapping = new_assoc();
  other->freshly_cleared = true;

  /* Avoid sharing the same symbol table: see issues #723, #724 */
  nusmv_assert(TypeChecker_get_symb_table(self) != other_st);
  other->symbolTable = other_st;
  /* add trigger: when a symbol is removed from the symbol table,
     we need to invalidate the type checker cache. */
  SymbTable_add_trigger(other_st,
                        type_checker_remove_symbol_trigger,
                        ST_TRIGGER_SYMBOL_REDECLARE,
                        (void*) other, false);

  /* copy virtual methods */
  other->type_in_expr_walkers =
    MASTER_NODE_WALKER(Object_copy(OBJECT(self->type_in_expr_walkers)));
}

SymbType_ptr tc_set_expression_type(TypeChecker_ptr self,
                                    node_ptr expression, SymbType_ptr type)
{
  if (type_checker_is_memoizing_enabled(self)) {
    nusmv_assert(nullType == tc_lookup_expr_type(self, expression));
    insert_assoc(self->expressionTypeMapping, expression, (node_ptr) type);
    self->freshly_cleared = false;
  }
  return type;
}

SymbType_ptr tc_lookup_expr_type(TypeChecker_ptr self, node_ptr expression)
{
  if (type_checker_is_memoizing_enabled(self)) {
    return SYMB_TYPE(find_assoc(self->expressionTypeMapping, expression));
  }
  return nullType;
}

SymbType_ptr type_checker_check_expression(TypeChecker_ptr self,
                                           node_ptr expression,
                                           node_ptr context)
{
  return type_checker_check_timed_expression(self, expression, context, true);
}

static void
type_checker_check_timed_expression_fallback (NuSMVEnv_ptr env,
                                              node_ptr expression)
{
  const StreamMgr_ptr streams =
    STREAM_MGR(NuSMVEnv_get_value(env, ENV_STREAM_MANAGER));
  const MasterPrinter_ptr wffprint =
    MASTER_PRINTER(NuSMVEnv_get_value(env, ENV_WFF_PRINTER));
  const MasterPrinter_ptr sexpprint =
    MASTER_PRINTER(NuSMVEnv_get_value(env, ENV_SEXP_PRINTER));
  const OptsHandler_ptr opts =
    OPTS_HANDLER(NuSMVEnv_get_value(env, ENV_OPTS_HANDLER));

  if (opt_verbose_level_gt(opts, 0)) {
    StreamMgr_print_error(streams,
      "Warning: no compatible checker found for expression:\n");
    StreamMgr_nprint_error(streams, wffprint, "%N", expression);
    StreamMgr_print_error(streams,  "\n");
    StreamMgr_nprint_error(streams, sexpprint, "%N", expression);
  }
}

static SymbType_ptr type_checker_check_timed_expression(TypeChecker_ptr self,
                                                        node_ptr expression,
                                                        node_ptr context,
                                                        boolean allow_clock)
{
  const NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(self));
  const StreamMgr_ptr streams =
    STREAM_MGR(NuSMVEnv_get_value(env, ENV_STREAM_MANAGER));
  const MasterPrinter_ptr wffprint =
    MASTER_PRINTER(NuSMVEnv_get_value(env, ENV_WFF_PRINTER));

  /* check for illegal usage of Clock type */
  if(!allow_clock &&
     TypeChecker_find_type_in_expr(self, car(expression), context,
                                   SymbTablePkg_clock_type(env)) != Nil){
    type_checker_viol_handler(self, TC_VIOLATION_FORBIDDEN_CLOCK, expression);

    return tc_set_expression_type(self, expression,
                                  SymbTablePkg_error_type(env));
  }

  {
    ListIter_ptr iter = NodeList_get_first_iter(MASTER_NODE_WALKER(self)->walkers);
    while (!ListIter_is_end(iter)) {
      CheckerBase_ptr cb =
        CHECKER_BASE(NodeList_get_elem_at(MASTER_NODE_WALKER(self)->walkers,
                                          iter));
      if (NodeWalker_can_handle(NODE_WALKER(cb), expression)) {
        return CheckerBase_check_expr(cb, expression, context);
      }
      iter = ListIter_get_next(iter);
    }
  }

  /* Fall back */
  type_checker_check_timed_expression_fallback (env, expression);

  return SymbTablePkg_error_type(env);
}

/* TODO[MD] It would be nice to also print expr, but PrinterWff does not
   support all possible inputs */
void type_checker_print_error_message(TypeChecker_ptr self, node_ptr expr,
                                      boolean is_error)
{
  const NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(self));
  const StreamMgr_ptr streams =
    STREAM_MGR(NuSMVEnv_get_value(env, ENV_STREAM_MANAGER));
  const OptsHandler_ptr opts =
    OPTS_HANDLER(NuSMVEnv_get_value(env, ENV_OPTS_HANDLER));


  if (is_error) StreamMgr_print_error(streams,  "\nTYPE ERROR ");
  else          StreamMgr_print_error(streams,  "\nTYPE WARNING ");

  if (get_input_file(opts)) {
    StreamMgr_print_error(streams,  "file %s", get_input_file(opts));
  }
  else StreamMgr_print_error(streams,  "file stdin");

  if (node_get_lineno(expr)) {
    StreamMgr_print_error(streams,  ": line %d", node_get_lineno(expr));
  }
  StreamMgr_print_error(streams,   " : ");
}

void type_checker_enable_memoizing(TypeChecker_ptr self, boolean enabled)
{
  if (enabled && self->memoizing_counter > 0) self->memoizing_counter -= 1;
  else if (!enabled) self->memoizing_counter += 1;
}

boolean type_checker_is_memoizing_enabled(const TypeChecker_ptr self)
{
  return self->memoizing_counter == 0;
}

boolean type_checker_viol_handler(TypeChecker_ptr self,
                                  TypeSystemViolation violation,
                                  node_ptr expression)
{
  /* In the output message, the information about the expression
     location are output. So, make sure that the input file name and
     line number are correctly set!
  */
  const NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(self));
  const OptsHandler_ptr opts =
    OPTS_HANDLER(NuSMVEnv_get_value(env, ENV_OPTS_HANDLER));
  const MasterPrinter_ptr wffprint =
    MASTER_PRINTER(NuSMVEnv_get_value(env, ENV_WFF_PRINTER));
  const StreamMgr_ptr streams =
    STREAM_MGR(NuSMVEnv_get_value(env, ENV_STREAM_MANAGER));

  boolean isError = true; /* is this error or warning */

  /* get rid of the context the expression may be wrapped in */
  node_ptr expr = expression;
  if (node_get_type(expression) == CONTEXT) {
    expr = cdr(expression);
  }

  /* checks the given violation */
  nusmv_assert(TypeSystemViolation_is_valid(violation));

  /* only violation TC_VIOLATION_DUPLICATE_CONSTANTS and the variable
     type_checking_backward_compatibility being true, may make a
     warning from an error.
  */
  if ((TC_VIOLATION_DUPLICATE_CONSTANTS == violation) &&
      opt_backward_comp(opts)) {
    isError = false;
  }

  if (!isError && !opt_type_checking_warning_on(opts)) {
    /* this is a warning and warning messages are not allowed.
       So, do nothing, just return false (this is not an error)
    */
    return false;
  }

  type_checker_print_error_message(self, expr, isError);

  switch (violation) {

  case TC_VIOLATION_INCORRECT_WORD_WIDTH:
    nusmv_assert(CONS == node_get_type(expr));
    StreamMgr_print_error(streams,  "in the declaration of '");
    StreamMgr_nprint_error(streams, wffprint, "%N", car(expr));
    StreamMgr_print_error(streams,
            "' the Word width is not a positive number (from range [1..%d])\n",
            WordNumber_max_width());
    break;

  case TC_VIOLATION_INCORRECT_WORDARRAY_WIDTH:
    nusmv_assert(CONS == node_get_type(expr));
    StreamMgr_print_error(streams,  "in the declaration of '");
    StreamMgr_nprint_error(streams, wffprint, "%N", car(expr));
    StreamMgr_print_error(streams,
            "' either the address or the value width are not in range [1..%d]\n",
            WordNumber_max_width());
    break;

  case TC_VIOLATION_DUPLICATE_CONSTANTS:
    nusmv_assert(CONS == node_get_type(expr));
    StreamMgr_print_error(streams,  "duplicate constants in the enum type of variable '");
    StreamMgr_nprint_error(streams, wffprint, "%N", car(expr));
    StreamMgr_print_error(streams,  "'\n");
    break;

  case TC_VIOLATION_FORBIDDEN_CLOCK:
    type_checker_print_error_message(self, expression, true);
    StreamMgr_print_error(streams,  "invalid usage of Clock type in '");
    StreamMgr_nprint_error(streams, wffprint, "%N", car(expression));
    StreamMgr_print_error(streams,  "'\n");
    break;

  default:
    error_unreachable_code(); /* unknown kind of an error */
  } /* switch (errorKind) */

  return isError;
}


MasterNodeWalker_ptr
type_checker_get_type_in_expr_walkers(const TypeChecker_ptr self)
{
  return self->type_in_expr_walkers;
}


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/*!
  \brief The TypeChecker class private initializer

  The TypeChecker class private initializer

  \sa TypeChecker_create
*/
static void type_checker_init(TypeChecker_ptr self, SymbTable_ptr symbolTable)
{
  const NuSMVEnv_ptr env = ENV_OBJECT_GET_ENV(symbolTable);

  /* base class initialization */
  master_node_walker_init(MASTER_NODE_WALKER(self), env);

  /* members initialisation */
  self->memoizing_counter = 0; /* memoizing is enabled by default */
  self->expressionTypeMapping = new_assoc();
  self->symbolTable = symbolTable;
  self->freshly_cleared = false;
  self->type_in_expr_walkers = MasterNodeWalker_create(env);

  /* When a symbol is redeclared, we need to clear the type checker
     cache. See bug #422*/
  SymbTable_add_trigger(symbolTable,
                        type_checker_remove_symbol_trigger,
                        ST_TRIGGER_SYMBOL_REDECLARE,
                        (void*) self, false);

  /* virtual methods settings */
  OVERRIDE(Object, finalize) = type_checker_finalize;
  /* Do not override copy, use specialised method TypeChecker_copy instead.
     Avoid the creation of 2 TypeCheckers sharing the same symbol table.
     The sharing of the symbol table could lead to dangling pointers in
     the triggers.
     See issues #723 #724.
  */
}

/*!
  \brief The TypeChecker class private deinitializer

  The TypeChecker class private deinitializer

  \sa TypeChecker_destroy
*/
static void type_checker_deinit(TypeChecker_ptr self)
{
  /* members deinitialization */
  free_assoc(self->expressionTypeMapping);
  MasterNodeWalker_destroy(self->type_in_expr_walkers);

  /* base class deinitialization */
  master_node_walker_deinit(MASTER_NODE_WALKER(self));
}

/*!
  \brief The PrinterBase class virtual finalizer

  Called by the class destructor
*/
static void type_checker_finalize(Object_ptr object, void* dummy)
{
  TypeChecker_ptr self = TYPE_CHECKER(object);

  type_checker_deinit(self);
  FREE(self);
}

/**Static Function************************************************************

   Synopsis           [Checks a list of expressions for being well-types.
   Expressions can be unmodified (after compilation), flattened or
   flattened+expanded.]

   Description        [The first parameter is the type checker performing
   the type checking. The second one is the kind of SMV constraint
   containing the input expressions (like INVAR, TRANS, etc). The third
   one is a set of expressions to be checked.
   In the set the expression are separated by AND or CONS. The set also
   can be Nil. This function is specialised to deal with the output of
   the compilation phase.

   NB for developers: the list of constraint expressions collected
   during flattening, i.e. "expressions" contains the actual
   expressions only. If the expression consist of one identifier only,
   then the indentifier is normalized (with resolve-name and find-atom)
   and info about the line number is lost. Then only the wrapping AND or CONS
   node will contain the correct line.

   The return value is true if there is no type violations, and false otherwise]

   SideEffects        []

******************************************************************************/

/*!
  \brief \todo Missing synopsis

  \todo Missing description
*/
static boolean type_checker_check_constraint_list(TypeChecker_ptr self,
                                                 int kind,
                                                 node_ptr expressions,
                                                 boolean allow_clock_left,
                                                 boolean allow_clock_right){
  const NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(self));
  const NodeMgr_ptr nodemgr =
    NODE_MGR(NuSMVEnv_get_value(env, ENV_NODE_MGR));
  node_ptr exp;
  boolean isOK = true;

  if (Nil == expressions) return true;


  /* assert expressions type matches separator for that kind */
  /* URGENT -> separator == OR */
  nusmv_assert(kind != URGENT || OR == node_get_type(expressions));
  /* allowed separators: AND, CONS, OR */
  nusmv_assert(AND == node_get_type(expressions) ||
               CONS == node_get_type(expressions) ||
               OR == node_get_type(expressions));

  /* process the left part of the list */
  exp = car(expressions);

  if (type_checker_check_constraint_list_is_leaf(exp, kind)) {
    /* this is an actual expression. */
    nusmv_yylineno = node_get_lineno(expressions);
    exp = find_node(nodemgr, kind, exp, Nil);
    node_set_lineno(exp, nusmv_yylineno);

    isOK = type_checker_is_timed_specification_wellformed(self, exp,
                                                          allow_clock_left)
      && isOK;
  }
  else {
    /* this is a subset of expressions */
    isOK = type_checker_check_constraint_list(self, kind, exp, allow_clock_left, allow_clock_right)
      && isOK;
  }

  /* process the right part of the list */
  exp = cdr(expressions);

  if (type_checker_check_constraint_list_is_leaf(exp, kind)) {
    /* this is an actual expression  */
    nusmv_yylineno = node_get_lineno(expressions);
    exp = find_node(nodemgr, kind, exp, Nil);
    node_set_lineno(exp, nusmv_yylineno);

    isOK = type_checker_is_timed_specification_wellformed(self, exp,
                                                          allow_clock_right)
      && isOK;
  }
  else {
    /* this is a subset of expressions */
    isOK = type_checker_check_constraint_list(self, kind, exp, allow_clock_left, allow_clock_right)
      && isOK;
  }

  return isOK;
}

/*!
  \brief check if exp is a base case for the given kind

  return false if exp == Nil,
  if exp is a base case for the given kind return true,
  false otherwise
*/
static boolean type_checker_check_constraint_list_is_leaf(const node_ptr exp, const int kind)
{
  if(exp == Nil) return false;

  /* set separators for each kind */
  switch(kind) {
  case URGENT:
    return (OR != node_get_type(exp));
  default:
    return (AND != node_get_type(exp) && CONS != node_get_type(exp));
  }
}

/*!
  \brief Resets memoizing to be enabled

  This function is called by high level type checking
   functions to enable memoizing
*/
static void type_checker_memoizing_force_enabled(TypeChecker_ptr self)
{ self->memoizing_counter = 0; }

/*!
  \brief Resets memoizing, cleaning up all type information

  This function is called by high level type checking
   functions to reset memoizing, after an error occurs
*/
static void type_checker_reset_memoizing(TypeChecker_ptr self)
{
  if (!(self->freshly_cleared)) {
    /* shuts down the memoizing */
    clear_assoc(self->expressionTypeMapping);
  }

  /* resets the memoizing enable counter */
  type_checker_memoizing_force_enabled(self);

  self->freshly_cleared = true;
}

/*!
  \brief Trigger for symbols removal in the symbol table

  Trigger for symbols removal in the symbol table.
                       When invoked, clears the memoization hash.
*/
static void type_checker_remove_symbol_trigger(const SymbTable_ptr st,
                                               const node_ptr sym,
                                               SymbTableTriggerAction action,
                                               void* arg)
{
  TypeChecker_ptr self = TYPE_CHECKER(arg);
  const NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(self));
  const OptsHandler_ptr opts =
    OPTS_HANDLER(NuSMVEnv_get_value(env, ENV_OPTS_HANDLER));

  UNUSED_PARAM(st);
  UNUSED_PARAM(sym);
  UNUSED_PARAM(action);

  if (opt_verbose_level_gt(opts, 4)) {
    Logger_ptr logger = LOGGER(NuSMVEnv_get_value(env, ENV_LOGGER));
    Logger_log(logger, "TypeChecker: cache invalidator trigger invoked\n");
  }

  type_checker_reset_memoizing(self);
}

/**AutomaticEnd***************************************************************/
