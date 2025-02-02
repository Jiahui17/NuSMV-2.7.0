/* ---------------------------------------------------------------------------

  This file is part of the ``prop'' package of NuSMV version 2.
  Copyright (C) 2010 by FBK-irst.

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
  \author marco Roveri, Roberto Cavada
  \brief Implementation of class 'PropDbAccessor'

  \todo: Missing description

*/


#if HAVE_CONFIG_H
#include "nusmv-config.h"
#endif

#include "nusmv/core/utils/OStream.h"
#include "nusmv/core/utils/StreamMgr.h"
#include "nusmv/core/utils/Logger.h"
#include "nusmv/core/node/NodeMgr.h"
#include "nusmv/core/utils/ErrorMgr.h"

#include "nusmv/core/prob/prop/PropDbAccessor.h"
#include "nusmv/core/prob/prop/PropDbAccessor_private.h"
#include "nusmv/core/prob/prop/Prop.h"
#include "nusmv/core/prob/prop/Prop_private.h"
#include "nusmv/core/prob/prop/propInt.h"


#include "nusmv/core/compile/compile.h"
#include "nusmv/core/compile/symb_table/SymbTable.h"
#include "nusmv/core/parser/symbols.h"
#include "nusmv/core/parser/parser.h"
#include "nusmv/core/parser/psl/pslNode.h"
#include "nusmv/core/utils/utils.h"
#include "nusmv/core/utils/ucmd.h"


/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
/* See 'PropDbAccessor_private.h' for class 'PropDbAccessor' definition. */

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

static void prop_db_accessor_finalize(Object_ptr object, void* dummy);

static int
prop_db_accessor_prop_parse_from_arg_and_add(PropDbAccessor_ptr self,
                                             SymbTable_ptr symb_table,
                                             int argc, const char** argv,
                                             const Prop_Type type);
static const char*
prop_db_accessor_get_prop_type_as_parsing_string(PropDbAccessor_ptr self,
                                                 const Prop_Type type);

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

PropDbAccessor_ptr PropDbAccessor_create(ProblemDb_ptr db)
{
  PropDbAccessor_ptr self = ALLOC(PropDbAccessor, 1);
  PROP_DB_ACCESSOR_CHECK_INSTANCE(self);

  prop_db_accessor_init(self, db,
                        ProblemDb_get_tags(db, PROP_DB_ACCESSOR_TAGS_NAME));
  return self;
}

void PropDbAccessor_destroy(PropDbAccessor_ptr self)
{
  PROP_DB_ACCESSOR_CHECK_INSTANCE(self);

  Object_destroy(OBJECT(self), NULL);
}

int PropDbAccessor_fill(PropDbAccessor_ptr self, SymbTable_ptr symb_table,
                        node_ptr ctlspec, node_ptr computespec,
                        node_ptr ltlspec, node_ptr pslspec,
                        node_ptr invarspec)
{
  node_ptr l;
  int res;
  Prop_ptr prop;

  PROP_DB_ACCESSOR_CHECK_INSTANCE(self);
  SYMB_TABLE_CHECK_INSTANCE(symb_table);

  /* [AM] Named specs
   * Named:
   *    [LTL;PSL;COMPUTE;INVAR;]SPEC
   *       /                 \
   *      CONTEXT            [p_name (DOT/ATOM)]
   *     /    \
   *   ...    ...
   */

  for (l = ctlspec; l != Nil; l = cdr(l)) {
    res = PropDbAccessor_prop_create_and_add(
        self, symb_table, car(car(l)), Prop_Ctl);
    if (res == -1) return 1;
    if (Nil != cdr(car(l))){
      prop = PropDbAccessor_get_prop_at_index(self, res);
      Prop_set_name(prop, cdr(car(l)));
    }
  }
  for (l = computespec; l != Nil; l = cdr(l)) {
    res = PropDbAccessor_prop_create_and_add(
        self, symb_table, car(car(l)), Prop_Compute);
    if (res == -1) return 1;
    if (Nil != cdr(car(l))){
      prop = PropDbAccessor_get_prop_at_index(self, res);
      Prop_set_name(prop, cdr(car(l)));
    }
  }
  for (l = ltlspec; l != Nil; l = cdr(l)) {
    res = PropDbAccessor_prop_create_and_add(
        self, symb_table, car(car(l)), Prop_Ltl);
    if (res == -1) return 1;
    if (Nil != cdr(car(l))){
      prop = PropDbAccessor_get_prop_at_index(self, res);
      Prop_set_name(prop, cdr(car(l)));
    }
  }
  for (l = pslspec; l != Nil; l = cdr(l)) {
    res = PropDbAccessor_prop_create_and_add(
        self, symb_table, car(car(l)), Prop_Psl);
    if (res == -1) return 1;
    if (Nil != cdr(car(l))){
      prop = PropDbAccessor_get_prop_at_index(self, res);
      Prop_set_name(prop, cdr(car(l)));
    }
  }
  for (l = invarspec; l != Nil; l = cdr(l)) {
    res = PropDbAccessor_prop_create_and_add(
        self, symb_table, car(car(l)), Prop_Invar);
    if (res == -1) return 1;
    if (Nil != cdr(car(l))){
      prop = PropDbAccessor_get_prop_at_index(self, res);
      Prop_set_name(prop, cdr(car(l)));
    }
  }

  return 0;
}

int PropDbAccessor_add(PropDbAccessor_ptr self, Prop_ptr prop)
{
  int idx;

  PROP_DB_ACCESSOR_CHECK_INSTANCE(self);
  PROP_CHECK_INSTANCE(prop);

  idx = ProblemDbAccessor_add(PROBLEM_DB_ACCESSOR(self), PROBLEM(prop));
  if (Prop_get_index(prop) == -1) {
    Prop_set_index(prop, idx);
  }

  /* sets tags */
  ProblemDb_set_problem_tags(
      ProblemDbAccessor_get_db(PROBLEM_DB_ACCESSOR(self)),
      PROBLEM(prop),
      ProblemDbAccessor_get_tags_mask(PROBLEM_DB_ACCESSOR(self)));

  return idx;
}

VIRTUAL int PropDbAccessor_prop_create_and_add(PropDbAccessor_ptr self,
                                               SymbTable_ptr symb_table,
                                               node_ptr spec,
                                               Prop_Type type)
{
  PROP_DB_ACCESSOR_CHECK_INSTANCE(self);
  SYMB_TABLE_CHECK_INSTANCE(symb_table);
  return self->prop_create_and_add(self, symb_table, spec, type);
}

Prop_ptr PropDbAccessor_get_last(const PropDbAccessor_ptr self)
{
  PROP_DB_ACCESSOR_CHECK_INSTANCE(self);
  return PROP(
      ProblemDbAccessor_get_last_problem(PROBLEM_DB_ACCESSOR(self), NULL));
}

Prop_ptr PropDbAccessor_get_prop_at_index(const PropDbAccessor_ptr self,
                                          int index)
{
  Problem_ptr prob;
  PROP_DB_ACCESSOR_CHECK_INSTANCE(self);

  prob = ProblemDbAccessor_get_problem_at_index(PROBLEM_DB_ACCESSOR(self),
                                                index);
  return PROP(prob);
}

Prop_ptr PropDbAccessor_get_prop_at_iter(const PropDbAccessor_ptr self,
                                         const struct ProblemDbIter* iter)
{
  PROP_DB_ACCESSOR_CHECK_INSTANCE(self);
  return PropDbAccessor_get_prop_at_index(self, iter->idx);
}

Set_t PropDbAccessor_get_props_at_indices(const PropDbAccessor_ptr self,
                                          const char* indices)
{
  Vector_ptr probs;
  Set_t res;
  int i;

  PROP_DB_ACCESSOR_CHECK_INSTANCE(self);

  probs = ProblemDb_get_problems_at_indices(
      ProblemDbAccessor_get_db(PROBLEM_DB_ACCESSOR(self)), indices,
      ProblemDbAccessor_get_tags_mask(PROBLEM_DB_ACCESSOR(self)));

  /* converts the vector to a set as required by the interface */
  res = Set_MakeEmpty();
  for (i=0; i<VECTOR_SIZE(probs); ++i) {
    Problem_ptr prob = VECTOR_AT(probs, i);
    if (prob) {
      res = Set_AddMember(res, (Set_Element_t) prob);
    }
  }

  Vector_destroy(probs);
  return res;
}

int PropDbAccessor_get_prop_name_index(const PropDbAccessor_ptr self,
                                       const node_ptr name)
{
  struct ProblemDbIter iter;

  PROP_DB_ACCESSOR_CHECK_INSTANCE(self);

  PROBLEM_DB_ACCESSOR_FOREACH(self, iter) {
    Prop_ptr prop = PropDbAccessor_get_prop_at_iter(self, &iter);
    if (Prop_get_name(prop) == name)
      return iter.idx;
  }

  return -1; /* not found */
}

PropDb_PrintFmt PropDbAccessor_get_print_fmt(const PropDbAccessor_ptr self)
{
  PROP_DB_ACCESSOR_CHECK_INSTANCE(self);
  return self->print_fmt;
}

PropDb_PrintFmt PropDbAccessor_set_print_fmt(const PropDbAccessor_ptr self,
                                             PropDb_PrintFmt new_fmt)
{
  PropDb_PrintFmt old;

  PROP_DB_ACCESSOR_CHECK_INSTANCE(self);

  old = self->print_fmt;
  self->print_fmt = new_fmt;
  return old;
}

void PropDbAccessor_print_list_header(const PropDbAccessor_ptr self,
                                      OStream_ptr file)
{
  PROP_DB_ACCESSOR_CHECK_INSTANCE(self);
  {
    const NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(self));
    const ErrorMgr_ptr errmgr =
      ERROR_MGR(NuSMVEnv_get_value(env, ENV_ERROR_MANAGER));

    switch (PropDbAccessor_get_print_fmt(self)) {
    case PROPDB_PRINT_FMT_TABULAR:
      OStream_printf(
          file,
          "**** PROPERTY LIST [ Type, Status, Counter-example Number, Name ]"
          " ****\n");
      OStream_printf(
          file,
          "--------------------------  PROPERTY LIST  "
          "-------------------------\n");
      break;

    case PROPDB_PRINT_FMT_XML:
      OStream_printf(file, "<?xml version=\"1.0\"?>\n");
      OStream_printf(file, "<properties xmlns=\"http://es.fbk.eu\"\n");
      OStream_printf(
          file,
          "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n");
      OStream_printf(
          file,
          "xsi:schemaLocation=\"http://es.fbk.eu/xsd properties.xsd\">\n\n");
      break;

    default:
      ErrorMgr_internal_error(errmgr, "Unsupported prop print format");
    }
  }
}

void PropDbAccessor_print_list_footer(const PropDbAccessor_ptr self,
                                      OStream_ptr file)
{
  PROP_DB_ACCESSOR_CHECK_INSTANCE(self);
  {
    const NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(self));
    const ErrorMgr_ptr errmgr =
      ERROR_MGR(NuSMVEnv_get_value(env, ENV_ERROR_MANAGER));

    switch (PropDbAccessor_get_print_fmt(self)) {
    case PROPDB_PRINT_FMT_TABULAR:
      break;

    case PROPDB_PRINT_FMT_XML:
      OStream_printf(file, "</properties>\n");
      break;

    default:
      ErrorMgr_internal_error(errmgr, "Unsupported prop print format");
    }
  }
}


/*!
  \brief Prints the specified property from the DB

  Prints on the given file stream the property
  whose unique identifier is specified
*/

int PropDbAccessor_print_prop_at_index(const PropDbAccessor_ptr self,
                               OStream_ptr file, const int index)
{
  int retval;
  Prop_ptr prop;

  PROP_DB_ACCESSOR_CHECK_INSTANCE(self);

  prop = PropDbAccessor_get_prop_at_index(self, index);
  if (prop != PROP(NULL)) {
    Prop_print_db(prop, file, PropDbAccessor_get_print_fmt(self));
    retval = 0;
  }
  else {
    retval = 1;
  }

  return retval;
}

void PropDbAccessor_print_all(const PropDbAccessor_ptr self, OStream_ptr file)
{
  PROP_DB_ACCESSOR_CHECK_INSTANCE(self);

  {
    const NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(self));
    const ErrorMgr_ptr errmgr =
      ERROR_MGR(NuSMVEnv_get_value(env, ENV_ERROR_MANAGER));

    PropDbAccessor_print_all_status_type(self, file,
                                         Prop_NoStatus, Prop_NoType);

    if (ProblemDbAccessor_count_problems(PROBLEM_DB_ACCESSOR(self)) == 0) {
      switch (PropDbAccessor_get_print_fmt(self)) {
      case PROPDB_PRINT_FMT_TABULAR:
        OStream_printf(file, "The properties DB is empty.\n");
        break;

      case PROPDB_PRINT_FMT_XML:
        OStream_printf(file, "  <!-- The properties DB is empty. -->\n");
        break;

      default:
        ErrorMgr_internal_error(errmgr, "Invalid print format");
      }
    }
  }
}

void PropDbAccessor_print_all_status_type(const PropDbAccessor_ptr self,
                                          OStream_ptr file,
                                          Prop_Status status,
                                          Prop_Type type)
{
  struct ProblemDbIter iter;

  PROP_DB_ACCESSOR_CHECK_INSTANCE(self);

  PROBLEM_DB_ACCESSOR_FOREACH(self, iter) {
    Prop_ptr p = PropDbAccessor_get_prop_at_iter(self, &iter);

    if (((type == Prop_NoType) || (Prop_get_type(p) == type)) &&
        ((status == Prop_NoStatus) || (Prop_get_status(p) == status))) {
      Prop_print_db(p, file, PropDbAccessor_get_print_fmt(self));
    }
  }
}

void PropDbAccessor_print_all_type(const PropDbAccessor_ptr self,
                                   OStream_ptr file, Prop_Type type)
{
  PROP_DB_ACCESSOR_CHECK_INSTANCE(self);

  PropDbAccessor_print_all_status_type(self, file, Prop_NoStatus, type);
}

void PropDbAccessor_print_all_status(const PropDbAccessor_ptr self,
                             OStream_ptr file, Prop_Status status)
{
  PROP_DB_ACCESSOR_CHECK_INSTANCE(self);

  PropDbAccessor_print_all_status_type(self, file, status, Prop_NoType);
}

lsList PropDbAccessor_get_ordered_props_of_type(
    const PropDbAccessor_ptr self,
    const FlatHierarchy_ptr hierarchy,
    const Prop_Type type)
{
  const NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(self));
  const NodeMgr_ptr nodemgr =
    NODE_MGR(NuSMVEnv_get_value(env, ENV_NODE_MGR));
  NodeList_ptr list;
  lsList result;
  ListIter_ptr iter;

  PROP_DB_ACCESSOR_CHECK_INSTANCE(self);

  result = lsCreate();
  nusmv_assert((lsList) NULL != result);

  list = PropDbAccessor_get_ordered_properties(self, hierarchy);
  NODE_LIST_FOREACH(list, iter) {
    node_ptr couple = NodeList_get_elem_at(list, iter);
    Prop_ptr prop = PROP(car(couple));

    if (Prop_get_type(prop) == type) {
      lsNewEnd(result, (lsGeneric)prop, LS_NH);
    }

    Set_ReleaseSet((Set_t)cdr(couple));
    free_node(nodemgr, couple);
  }

  NodeList_destroy(list);
  return result;
}

lsList PropDbAccessor_get_props_of_type(const PropDbAccessor_ptr self,
                                        const Prop_Type type)
{

  lsList result;
  struct ProblemDbIter iter;

  PROP_DB_ACCESSOR_CHECK_INSTANCE(self);

  result = lsCreate();
  nusmv_assert((lsList) NULL != result);

  PROBLEM_DB_ACCESSOR_FOREACH(self, iter) {
    Prop_ptr p = PropDbAccessor_get_prop_at_iter(self, &iter);

    if (Prop_get_type(p) == type) {
      lsNewEnd(result, (lsGeneric)p, LS_NH);
    }
  }

  return result;
}

int PropDbAccessor_prop_parse_and_add(const PropDbAccessor_ptr self,
                              SymbTable_ptr symb_table,
                              const char* str,
                              const Prop_Type type,
                              const node_ptr expr_name)
{
  const char* argv[2];
  int argc = 2;
  int res;

  PROP_DB_ACCESSOR_CHECK_INSTANCE(self);
  nusmv_assert(str != (char*) NULL);

  argv[0] = (char*) NULL;
  argv[1] = (char*) str;

  res = prop_db_accessor_prop_parse_from_arg_and_add(self, symb_table,
                                                     argc, argv, type);

  if (-1 == res) return -1;
  else if (Nil != expr_name) {
    Prop_ptr property = PropDbAccessor_get_prop_at_index(self, res);

    Prop_set_name(property, expr_name);
  }

  return res;
}

int PropDbAccessor_prop_parse_name(const PropDbAccessor_ptr self,
                                   const char* str)
{
  StreamMgr_ptr streams;
  node_ptr property;
  node_ptr parsed_command = Nil;
  int parse_result;
  NuSMVEnv_ptr env;

  PROP_DB_ACCESSOR_CHECK_INSTANCE(self);
  env = EnvObject_get_environment(ENV_OBJECT(self));
  streams = STREAM_MGR(NuSMVEnv_get_value(env, ENV_STREAM_MANAGER));

  nusmv_assert(str != (char*) NULL);

  parse_result =
    Parser_ReadIdentifierExprFromString(env, str, &parsed_command);

  if (parse_result != 0 || parsed_command == Nil) {
    StreamMgr_print_error(streams,
            "Parsing error: expected a property name.\n");
    return -1;
  }

  property = car(parsed_command);
  property = CompileFlatten_concat_contexts(env, Nil, property);

  return PropDbAccessor_get_prop_name_index(self, property);
}

int PropDbAccessor_get_prop_index_from_trace_index(
    const PropDbAccessor_ptr self,
    const int trace_idx)
{
  struct ProblemDbIter iter;

  PROP_DB_ACCESSOR_CHECK_INSTANCE(self);

  PROBLEM_DB_ACCESSOR_FOREACH(self, iter) {
    Prop_ptr prop = PropDbAccessor_get_prop_at_iter(self, &iter);
    if (Prop_get_trace(prop) == (trace_idx + 1)) {
      return iter.idx;
    }
  }
  return -1;
}

void PropDbAccessor_verify_prop_at_index(const PropDbAccessor_ptr self,
                                         const int index)
{
  Prop_ptr p;

  PROP_DB_ACCESSOR_CHECK_INSTANCE(self);

  p = PropDbAccessor_get_prop_at_index(self, index);
  nusmv_assert(p);
  Prop_verify(p);
}

void PropDbAccessor_verify_all_type(const PropDbAccessor_ptr self,
                                    Prop_Type type)
{
  struct ProblemDbIter iter;
  PROP_DB_ACCESSOR_CHECK_INSTANCE(self);

  PROBLEM_DB_ACCESSOR_FOREACH(self, iter) {
    Prop_ptr p = PropDbAccessor_get_prop_at_iter(self, &iter);
    if (Prop_get_type(p) == type)
      Prop_verify(p);
  }
}

VIRTUAL void PropDbAccessor_verify_all(const PropDbAccessor_ptr self)
{
  PROP_DB_ACCESSOR_CHECK_INSTANCE(self);
  self->verify_all(self);
}

void PropDbAccessor_ordered_verify_all(const PropDbAccessor_ptr self,
                                       const FlatHierarchy_ptr hierarchy)
{
  PROP_DB_ACCESSOR_CHECK_INSTANCE(self);
  PropDbAccessor_ordered_verify_all_type(self, hierarchy, Prop_NoType);
}

void PropDbAccessor_ordered_verify_all_type(
    const PropDbAccessor_ptr self,
    const FlatHierarchy_ptr hierarchy,
    const Prop_Type type)
{
  const NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(self));
  const NodeMgr_ptr nodemgr =
    NODE_MGR(NuSMVEnv_get_value(env, ENV_NODE_MGR));
  NodeList_ptr list;
  ListIter_ptr iter;

  PROP_DB_ACCESSOR_CHECK_INSTANCE(self);

  list = PropDbAccessor_get_ordered_properties(self, hierarchy);
  NODE_LIST_FOREACH(list, iter) {
    node_ptr couple = NodeList_get_elem_at(list, iter);
    Prop_ptr prop = PROP(car(couple));

    if ((Prop_NoType == type) || (Prop_get_type(prop) == type)) {
      Prop_verify(prop);
    }

    Set_ReleaseSet((Set_t)cdr(couple));
    free_node(nodemgr, couple);
  }

  NodeList_destroy(list);
}

NodeList_ptr PropDbAccessor_get_ordered_properties(
    const PropDbAccessor_ptr self,
    const FlatHierarchy_ptr hierarchy)
{
  const NuSMVEnv_ptr env = ENV_OBJECT(self)->environment;
  const OptsHandler_ptr opts =
    OPTS_HANDLER(NuSMVEnv_get_value(env, ENV_OPTS_HANDLER));
const NodeMgr_ptr nodemgr =
  NODE_MGR(NuSMVEnv_get_value(env, ENV_NODE_MGR));

  SymbTable_ptr symb_table;
  NodeList_ptr res;
  struct ProblemDbIter iter;

  PROP_DB_ACCESSOR_CHECK_INSTANCE(self);

  symb_table = FlatHierarchy_get_symb_table(hierarchy);
  res = NodeList_create();

  if (opt_verbose_level_ge(opts, 2)) {
    Logger_ptr logger = LOGGER(NuSMVEnv_get_value(env, ENV_LOGGER));
    Logger_log(logger, "Ordering properties by COI size\n");
  }

  PROBLEM_DB_ACCESSOR_FOREACH(self, iter) {
    Prop_ptr prop = PropDbAccessor_get_prop_at_iter(self, &iter);
    Set_t cone = Prop_compute_cone(prop, hierarchy, symb_table);
    int card = Set_GiveCardinality(cone);
    boolean inserted = false;
    /* Order insert into the list */
    node_ptr new_entry = cons(nodemgr, NODE_PTR(prop), NODE_PTR(cone));

    ListIter_ptr liter;
    NODE_LIST_FOREACH(res, liter) {
      node_ptr entry = NodeList_get_elem_at(res, liter);
      Set_t entry_cone = (Set_t)cdr(entry);

      if (Set_GiveCardinality(entry_cone) > card) {
        NodeList_insert_before(res, liter, new_entry);
        inserted = true;
        break;
      }
    }

    if (!inserted) NodeList_append(res, new_entry);
  }

  if (opt_verbose_level_ge(opts, 2)) {
    Logger_ptr logger = LOGGER(NuSMVEnv_get_value(env, ENV_LOGGER));
    Logger_log(logger, "Properties ordering done\n");
  }

  return res;
}

NodeList_ptr
PropDbAccessor_get_coi_grouped_properties(
    const PropDbAccessor_ptr self,
    const FlatHierarchy_ptr hierarchy)
{
  const NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(self));
  const NodeMgr_ptr nodemgr =
    NODE_MGR(NuSMVEnv_get_value(env, ENV_NODE_MGR));

  NodeList_ptr result, order;
  ListIter_ptr iter;

  PROP_DB_ACCESSOR_CHECK_INSTANCE(self);

  result = NodeList_create();
  order = PropDbAccessor_get_ordered_properties(self, hierarchy);
  NODE_LIST_FOREACH(order, iter) {
    boolean found = false;
    ListIter_ptr res_iter;
    node_ptr entry = NodeList_get_elem_at(order, iter);
    Set_t cone = (Set_t)cdr(entry);
    Prop_ptr prop = PROP(car(entry));

    NODE_LIST_FOREACH(result, res_iter) {
      node_ptr res_entry = NodeList_get_elem_at(result, res_iter);
      Set_t props = (Set_t)cdr(res_entry);
      Set_t res_cone = (Set_t)car(res_entry);
      if (Set_Equals(res_cone, cone)) {
        props = Set_AddMember(props, (Set_Element_t)prop);
        setcdr(res_entry, NODE_PTR(props));
        found = true;
        break;
      }
    }

    if (!found) {
      Set_t props = Set_MakeSingleton((Set_Element_t)prop);
      node_ptr new_entry = cons(nodemgr, NODE_PTR(cone), NODE_PTR(props));
      NodeList_append(result, new_entry);
    }
    else {
      Set_ReleaseSet(cone);
    }

    free_node(nodemgr, entry);
  }

  return result;
}

int PropDbAccessor_show_property(const PropDbAccessor_ptr self,
                                 const boolean print_props_num,
                                 const PropDb_PrintFmt fmt,
                                 const Prop_Type type,
                                 const Prop_Status status,
                                 const int prop_no,
                                 FILE* outstream)
{
  const NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(self));
  const StreamMgr_ptr streams =
   STREAM_MGR(NuSMVEnv_get_value(env, ENV_STREAM_MANAGER));
  int retval = 0;

  if (print_props_num) {
    StreamMgr_print_output(
        streams,  "Current number of stored properties: %d\n",
        ProblemDbAccessor_count_problems(PROBLEM_DB_ACCESSOR(self)));
    retval = 0;
  }
  else {
    PropDb_PrintFmt old_fmt = PropDbAccessor_set_print_fmt(self, fmt);
    OStream_ptr output = OStream_create(outstream);

    PropDbAccessor_print_list_header(self, output);
    if ((type == Prop_NoType) && (status == Prop_NoStatus) && (prop_no == -1)) {
      PropDbAccessor_print_all(self, output);
    } else if ((type != Prop_NoType) && (status != Prop_NoStatus)) {
      PropDbAccessor_print_all_status_type(self, output, status, type);
    } else if ((type != Prop_NoType) && (status == Prop_NoStatus)) {
      PropDbAccessor_print_all_type(self, output, type);
    } else if ((type == Prop_NoType) && (status != Prop_NoStatus)) {
      PropDbAccessor_print_all_status(self, output, status);
    } else if (prop_no != -1) {
      retval = PropDbAccessor_print_prop_at_index(self, output, prop_no);
    }
    PropDbAccessor_print_list_footer(self, output);

    PropDbAccessor_set_print_fmt(self, old_fmt);

    /* caller is going to take care of the stream, so use the
       destroy_safe method */
    OStream_destroy_safe(output);
  }

  return retval;
}

int PropDbAccessor_check_property(const PropDbAccessor_ptr self,
                                  const Prop_Type pt,
                                  const char* formula,
                                  const int prop_no)
{
  const NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(self));
  const ErrorMgr_ptr errmgr =
   ERROR_MGR(NuSMVEnv_get_value(env, ENV_ERROR_MANAGER));
  const OptsHandler_ptr opts =
     OPTS_HANDLER(NuSMVEnv_get_value(env, ENV_OPTS_HANDLER));

  if (pt != Prop_NoType) {
    if (formula == NIL(char)) {
      CATCH(errmgr) {
        PropDbAccessor_verify_all_type_wrapper(self, pt);
      }
      FAIL(errmgr) {
        return(1);
      }
    }
    else {
      int result;
      SymbTable_ptr st = SYMB_TABLE(NuSMVEnv_get_value(env, ENV_SYMB_TABLE));
      result = PropDbAccessor_prop_parse_and_add(self, st, formula, pt, Nil);
      if (result == -1) return(1);
      PropDbAccessor_verify_prop_at_index(self, result);
    }
  }
  else {
    if (prop_no == -1) {
      CATCH(errmgr) {
        if (opt_use_coi_size_sorting(opts)) {
          FlatHierarchy_ptr hierarchy =
            FLAT_HIERARCHY(NuSMVEnv_get_value(env, ENV_FLAT_HIERARCHY));

          PropDbAccessor_ordered_verify_all(self, hierarchy);
        }
        else PropDbAccessor_verify_all(self);
      } FAIL(errmgr) {
        return(1);
      }
    }
    else {
      CATCH(errmgr) {
        PropDbAccessor_verify_prop_at_index(self, prop_no);
      }
      FAIL(errmgr) {
        return(1);
      }
    }
  }

  return(0);
}

lsList PropDbAccessor_prepare_prop_list(const PropDbAccessor_ptr self,
                                        const Prop_Type type)
{
  const NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(self));
  const OptsHandler_ptr opts =
     OPTS_HANDLER(NuSMVEnv_get_value(env, ENV_OPTS_HANDLER));
  lsList props;

  if (opt_use_coi_size_sorting(opts)) {
    FlatHierarchy_ptr hierarchy =
      FLAT_HIERARCHY(NuSMVEnv_get_value(env, ENV_FLAT_HIERARCHY));

    props = PropDbAccessor_get_ordered_props_of_type(self, hierarchy, type);
  }
  else props = PropDbAccessor_get_props_of_type(self, type);

  nusmv_assert(props != LS_NIL);

  return props;
}

void PropDbAccessor_verify_all_type_wrapper(PropDbAccessor_ptr const self,
                                            const Prop_Type type)
{
  const NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(self));
  const OptsHandler_ptr opts =
     OPTS_HANDLER(NuSMVEnv_get_value(env, ENV_OPTS_HANDLER));

  if (opt_use_coi_size_sorting(opts)) {
    FlatHierarchy_ptr hierarchy =
      FLAT_HIERARCHY(NuSMVEnv_get_value(env, ENV_FLAT_HIERARCHY));
    PropDbAccessor_ordered_verify_all_type(self, hierarchy, type);
  }
  else {
    PropDbAccessor_verify_all_type(self, type);
  }
}

boolean PropDbAccessor_is_prop_registered(PropDbAccessor_ptr self,
                                          Prop_ptr prop)
{
  int prop_index = 0;
  Prop_ptr retrieved_prop = NULL;

  prop_index = Prop_get_index(prop);
  /* can't be registered */
  if (prop_index < 0)
    return false;

  retrieved_prop = PropDbAccessor_get_prop_at_index(self, prop_index);
  /* prop is not equal to the corresponding property in the database */
  return (retrieved_prop == prop);
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

void prop_db_accessor_init(PropDbAccessor_ptr self, ProblemDb_ptr db,
                           long long tags)
{
  /* base class initialization */
  problem_db_accessor_init(PROBLEM_DB_ACCESSOR(self), db, tags);

  /* members initialization */
  self->print_fmt = PROPDB_PRINT_FMT_DEFAULT;

  /* virtual methods settings */
  OVERRIDE(Object, finalize) = prop_db_accessor_finalize;
  OVERRIDE(PropDbAccessor, prop_create_and_add) =
      prop_db_accessor_prop_create_and_add;
  OVERRIDE(PropDbAccessor, verify_all) = prop_db_accessor_verify_all;
}

void prop_db_accessor_deinit(PropDbAccessor_ptr self)
{
  /* members deinitialization */
  /* base class deinitialization */
  problem_db_accessor_deinit(PROBLEM_DB_ACCESSOR(self));
}

int prop_db_accessor_prop_create_and_add(PropDbAccessor_ptr self,
                                         SymbTable_ptr symb_table,
                                         node_ptr spec, Prop_Type type)
{
  const NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(self));
  const StreamMgr_ptr streams =
    STREAM_MGR(NuSMVEnv_get_value(env, ENV_STREAM_MANAGER));
  const ErrorMgr_ptr errmgr =
    ERROR_MGR(NuSMVEnv_get_value(env, ENV_ERROR_MANAGER));
  const OptsHandler_ptr opts =
    OPTS_HANDLER(NuSMVEnv_get_value(env, ENV_OPTS_HANDLER));

  int retval, index;
  boolean allow_adding, allow_checking, is_ctl;
  Prop_ptr prop;

  retval = 0;
  index = ProblemDb_get_size(
      ProblemDbAccessor_get_db(PROBLEM_DB_ACCESSOR(self)));
  allow_adding = true;
  allow_checking = true;
  is_ctl = (type == Prop_Ctl);
  prop = NULL;

  /* PSL properties need to be converted to CTL or LTL specifications */
  if (type == Prop_Psl) {
    PslNode_ptr psl_prop = PslNode_convert_from_node_ptr(spec);
    /* removal of forall */
    psl_prop = PslNode_remove_forall_replicators(env, psl_prop);
    if (!PslNode_is_handled_psl(env, psl_prop)) {
      /* here the property may be either OBE or unmanageable */
      if (PslNode_is_obe(psl_prop))
        is_ctl = true;
      else {
        /* it is not supported */
        ErrorMgr_warning_psl_not_supported_feature(errmgr, spec, index);
        allow_checking = false;
      }
    }
  }

  prop = Prop_create_partial(ENV_OBJECT(self)->environment, spec, type);

  if (allow_checking) {
    nusmv_assert(SYMB_TABLE(NULL) != symb_table);

    if (!TypeChecker_check_property(SymbTable_get_type_checker(symb_table),
                                    prop)) {
      const OStream_ptr oerr = StreamMgr_get_error_ostream(streams);

      StreamMgr_print_error(streams,  "ERROR: Property \"");
      Prop_print(prop, oerr,
                 get_prop_print_method(opts));
      StreamMgr_print_error(streams,
                            "\b\" is not correct or not well typed.\n");

      return -1; /* type violation */
    }

    /* Checks for input vars */
    if (is_ctl || (type == Prop_Compute)) {
      Set_t expr_vars;

      if (opt_verbose_level_gt(opts, 5)) {
        Logger_ptr logger = LOGGER(NuSMVEnv_get_value(env, ENV_LOGGER));
        Logger_log(logger,
                "Checking %s property (index %d) for input variables. \n",
                Prop_get_type_as_string(prop), index);
      }

      /* Get list of variables in the expression, and check for inputs */
      expr_vars = Formula_GetDependencies(symb_table,
                                          Prop_get_expr_core_keep_ltl_bop(prop),
                                          Nil);

      allow_adding = !SymbTable_list_contains_input_var(
          symb_table, Set_Set2List(expr_vars));
      Set_ReleaseSet(expr_vars);
    }

    /* Check for next operators. Only invarspecs and ltl can contain next
       operators */
    {
      node_ptr core = Prop_get_expr_core_keep_ltl_bop(prop);
      node_ptr context = car(core);
      node_ptr body = cdr(core);

      if (Prop_Invar == type || Prop_Ltl == type) {
        Compile_check_next(symb_table, body, context, true);

        Compile_check_input_next(symb_table, body, context);
      }
      else {
        Compile_check_next(symb_table, body, context, false);
      }
    }

  }

  /* If no input vars present then add property to database */
  if (allow_adding) {
    if (opt_verbose_level_gt(opts, 3)) {
      Logger_ptr logger = LOGGER(NuSMVEnv_get_value(env, ENV_LOGGER));
      Logger_log(
          logger,
          "Attempting to add %s property (index %d) to property list.\n",
          Prop_get_type_as_string(prop), index);
    }
    {
      int idx = PropDbAccessor_add(self, prop);
      nusmv_assert(idx == index);
    }

    if (opt_verbose_level_gt(opts, 3)) {
      Logger_ptr logger = LOGGER(NuSMVEnv_get_value(env, ENV_LOGGER));
      Logger_log(
          logger,
          "%s property (index %d) successfully added to property list.\n",
          Prop_get_type_as_string(prop), index);
    }
  }
  else {
    /* Property contains input variables */
    retval = 1;
    ErrorMgr_error_property_contains_input_vars(errmgr, prop);
  }

  return (retval == 1) ? -1 : index;
}

void prop_db_accessor_verify_all(const PropDbAccessor_ptr self)
{
  PropDbAccessor_verify_all_type(self, Prop_Ctl);
  PropDbAccessor_verify_all_type(self, Prop_Compute);
  PropDbAccessor_verify_all_type(self, Prop_Ltl);
  PropDbAccessor_verify_all_type(self, Prop_Psl);
  PropDbAccessor_verify_all_type(self, Prop_Invar);
}


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/*!
  \brief The PropDbAccessor class virtual finalizer

  Called by the class destructor
*/
static void prop_db_accessor_finalize(Object_ptr object, void* dummy)
{
  PropDbAccessor_ptr self = PROP_DB_ACCESSOR(object);

  UNUSED_PARAM(dummy);

  prop_db_accessor_deinit(self);
  FREE(self);
}

/*!
  \brief Add a property to the database from an arg structure
  and a type

  Parses and creates a property of a given type from
  an arg structure. If the formula is correct, it is added to the
  property database and its index is returned.
  Otherwise, -1 is returned.
  Valid types are Prop_Ctl, Prop_Ltl, Prop_Psl, Prop_Invar and Prop_Compute.
*/
static int
prop_db_accessor_prop_parse_from_arg_and_add(PropDbAccessor_ptr self,
                                             SymbTable_ptr symb_table,
                                             int argc, const char** argv,
                                             const Prop_Type type)
{
  const NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(self));
  const StreamMgr_ptr streams =
    STREAM_MGR(NuSMVEnv_get_value(env, ENV_STREAM_MANAGER));
  NodeMgr_ptr nodemgr = NODE_MGR(NuSMVEnv_get_value(env, ENV_NODE_MGR));

  switch (type) {
  case Prop_Ctl:
  case Prop_Ltl:
  case Prop_Psl:
  case Prop_Invar:
  case Prop_Compute:
    /* All ok */
    break;

    /* Property name given as command argument, should use
     * PropDbAccessor_prop_parse_name */
  case Prop_CompId:
    StreamMgr_print_error(
        streams,
        "Required to parse a property of Prop_CompId. "
        "Use PropDbAccessor_prop_parse_name instead\n");
    return -1;

  case Prop_NoType:
    StreamMgr_print_error(
        streams,
        "Required to parse a property of unknonw type.\n");
    return -1;

  default:
    StreamMgr_print_error(
        streams,
        "Required to parse a property of unsupported type.\n");
    return -1;
  } /* switch */

  {
    node_ptr property;
    node_ptr parsed_command = Nil;

    if (type != Prop_Psl) {
      const char* parsing_type =
        prop_db_accessor_get_prop_type_as_parsing_string(self, type);
      int parse_result = Parser_ReadCmdFromString(env,
                                                  argc, argv,
                                                  (char*) parsing_type,
                                                  ";\n", &parsed_command);

      if (parse_result != 0 || parsed_command == Nil) {
        StreamMgr_print_error(streams,
                "Parsing error: expected an \"%s\" expression.\n",
                PropType_to_string(type));
        return -1;
      }
      property = car(parsed_command);
    }
    else {
      int parse_result = Parser_read_psl_from_string(env,
                                                     argc, argv,
                                                     &parsed_command);
      if (parse_result != 0 || parsed_command == Nil) {
        StreamMgr_print_error(streams,
                "Parsing error: expected an \"%s\" expression.\n",
                PropType_to_string(type));
        return -1;
      }
      /* makes possible context absolute */
      if (node_get_type(parsed_command) == CONTEXT) {
        node_ptr new_ctx =
            CompileFlatten_concat_contexts(env, Nil, car(parsed_command));
        property = PslNode_new_context(nodemgr, new_ctx, cdr(parsed_command));
      }
      else {
        property = PslNode_new_context(nodemgr, NULL, parsed_command);
      }
    }

    return PropDbAccessor_prop_create_and_add(
        self, symb_table, property, type);
  }
}

/*!
  \brief Returns the parsing type given the property type

  Returns the parsing type given the property type.
  The returned string must NOT be freed.
*/
static const char*
prop_db_accessor_get_prop_type_as_parsing_string(PropDbAccessor_ptr self,
                                                 const Prop_Type type)
{
  UNUSED_PARAM(self);

  switch (type) {
  case Prop_NoType: break; /* to suppress compiler's warnings */
  case Prop_Ctl: return "CTLWFF ";
  case Prop_Ltl: return "LTLWFF ";
  case Prop_Psl: return "PSLWFF ";
  case Prop_Invar: return "NEXTWFF ";
  case Prop_Compute: return "COMPWFF ";
  case Prop_CompId:  return "COMPID ";
  default: break; /* to suppress compiler's warnings */
  }

  return "SIMPWFF ";
}


/**AutomaticEnd***************************************************************/
