/* ---------------------------------------------------------------------------


  This file is part of the ``hrc'' package of NuSMV version 2.
  Copyright (C) 2009 by FBK.

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
  \author Sergio Mover
  \brief Utility functions used to concatenate/remove prefixes from
  names.

  Utility functions used to concatenate/remove prefixes from
  names.

*/

#include "nusmv/core/node/NodeMgr.h"
#include "nusmv/core/node/printers/MasterPrinter.h"
#include "nusmv/core/hrc/hrcPrefixUtils.h"
#include "nusmv/core/parser/symbols.h"
#include "nusmv/core/utils/ustring.h"
#include "nusmv/core/compile/compile.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/


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

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

Set_t hrc_prefix_utils_get_prefix_symbols(Set_t symbol_set,
                                          node_ptr prefix)
{
  Set_t result_set;
  Set_Iterator_t iter;

  result_set = Set_MakeEmpty();

  SET_FOREACH(symbol_set, iter) {
    node_ptr symbol;

    symbol = NODE_PTR(Set_GetMember(symbol_set, iter));

    if (NODE_PTR(Nil) != symbol) {
      if (hrc_prefix_utils_is_subprefix(prefix, car(symbol))) {
        result_set = Set_AddMember(result_set, symbol);
      }
    }
  }

  return result_set;
}

boolean hrc_prefix_utils_is_subprefix(node_ptr subprefix, node_ptr prefix)
{
  if (subprefix == prefix) {
    /* subprefix and prefix are the same */
    return true;
  } else if (NODE_PTR(Nil) == prefix) {
    /* prefix is Nil while subprefix is not Nil */
    return false;
  } else {
    /* recursive call */
    return hrc_prefix_utils_is_subprefix(subprefix, car(prefix));
  }
}

node_ptr hrc_prefix_utils_add_context(NodeMgr_ptr nodemgr,
                                      node_ptr context, node_ptr expression)
{
  node_ptr return_expression;

  nusmv_assert(NODE_PTR(Nil) != expression);

  if (DOT != node_get_type(expression) &&
      CONTEXT != node_get_type(expression)) {
    /* base case: concatenate context and expression */
    return_expression = find_node(nodemgr, DOT,
                                  context,
                                  find_atom(nodemgr, expression));
  } else {
    /* Recursively build the context with car(expression) and prefix
       it to cdr(expression) */
    context = hrc_prefix_utils_add_context(nodemgr, context,
                                       car(expression));
    return_expression = find_node(nodemgr, DOT,
                                  context,
                                  find_atom(nodemgr, cdr(expression)));
  }

  return return_expression;
}

node_ptr hrc_prefix_utils_get_first_subcontext(node_ptr symbol)
{
  node_ptr context;

  context = symbol;

  /* Search the first DOT or context node following the leftmost path
     of symbol.

     This is needed to get rid of nodes that can appear on top of DOT,
     like ARRAYOF.

     From this loop context will be a Nil node, a DOT node or a
     CONTEXT node.
  */
  while (NODE_PTR(Nil) != context &&
         DOT != node_get_type(context) &&
         CONTEXT != node_get_type(context)) {
    context = car(context);
  }

  /* Now context contains NIL or the first context of symbol.

     Get the first subcontext if it exsists.
   */
  if (NODE_PTR(Nil) != context) {
    /* get the first subcontext */
    if (NODE_PTR(Nil) == car(context)) {
      context = NODE_PTR(Nil);
    } else if (DOT == node_get_type(context) ||
               CONTEXT == node_get_type(context)) {
      /* found the subcontext */
      context = car(context);
    } else {
      /* subcontext does not exists. This should not happen! */
      context = NODE_PTR(Nil);
    }
  }

  return context;
}

node_ptr hrc_prefix_utils_remove_context(NodeMgr_ptr nodemgr,
                                         node_ptr identifier,
                                         node_ptr context)
{
  node_ptr identifier_no_context;

  if (Nil == identifier) {
    identifier_no_context = Nil;
  } else if (car(identifier) == context) {
    identifier_no_context = cdr(identifier);
  } else {
    node_ptr new_context;

    new_context = hrc_prefix_utils_remove_context(nodemgr, car(identifier), context);
    identifier_no_context = find_node(nodemgr, DOT, new_context, cdr(identifier));
  }

  return identifier_no_context;
}

node_ptr hrc_prefix_utils_assign_module_name(HrcNode_ptr instance,
                                             node_ptr instance_name)
{
  SymbTable_ptr st = HrcNode_get_symbol_table(instance);
  NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(st));
  const NodeMgr_ptr nodemgr =
    NODE_MGR(NuSMVEnv_get_value(env, ENV_NODE_MGR));
  UStringMgr_ptr strings = USTRING_MGR(NuSMVEnv_get_value(env, ENV_STRING_MGR));
  const MasterPrinter_ptr wffprint =
    MASTER_PRINTER(NuSMVEnv_get_value(env, ENV_WFF_PRINTER));

  node_ptr module_name;
  node_ptr new_name;
  char* module_name_chr;
  char* instance_name_chr;
  char* new_name_chr;
  char* tmp;
  boolean found_double_quote;


  module_name = HrcNode_get_name(instance);

  /* Get char* representation of module and instance name */
  module_name_chr = sprint_node(wffprint, module_name);
  instance_name_chr = sprint_node(wffprint, instance_name);

  /* creates the new module name */
  new_name_chr = ALLOC(char,
                       strlen(module_name_chr) +
                       strlen(instance_name_chr) +
                       2);

  if (NODE_PTR(Nil) == instance_name) {
    /* main module */
    sprintf(new_name_chr, "%s", module_name_chr);
  } else {
    sprintf(new_name_chr,
            "%s_%s",
            module_name_chr,
            instance_name_chr);
  }

  found_double_quote = false;

  /* substitutes . with _ inside the module name.
     substitutes " with _ inside the module name.
   */
  for (tmp = new_name_chr; *tmp != 0; tmp++) {
    if ('.' == *tmp) *tmp = '_';
    if ('"' == *tmp) {
      /* [SM] found_double_quote is true iff the first character of
         new_name_chr is a double quote that must NOT be substituted.

         If we find a double quite inside the string we can substitute
         it.
      */
      if (tmp == new_name_chr) {
        found_double_quote = true;
      }
      *tmp = '_';
    }
  }

  /* manages double quote */
  if (found_double_quote) {
    char* copy_str;
    char* app;
    unsigned int copy_size;

    copy_size = strlen(new_name_chr) + 3;
    copy_str = ALLOC(char, copy_size);

    /* copy in copy_str new_name_chr, leaving free the first character.
       Pay attention that this operation does not copy terminator.
     */
    strncpy((copy_str+1) , new_name_chr, strlen(new_name_chr));
    copy_str[0] = '"';
    /* Add ending double quote */
    copy_str[copy_size-2] = '"';
    copy_str[copy_size-1] = '\0';

    /* replace new_name_chr with copy_str */
    app = new_name_chr;
    new_name_chr = copy_str;
    FREE(app);
  }

  /* Creates the new module name */
  new_name = find_node(nodemgr, ATOM, (node_ptr) UStringMgr_find_string(strings, new_name_chr), Nil);

  FREE(module_name_chr);
  FREE(instance_name_chr);
  FREE(new_name_chr);

  return new_name;
}

/*!
  \brief Contatenates 2 contexts

  Contatenates 2 contexts
*/

node_ptr hrc_prefix_utils_concat_context(const NuSMVEnv_ptr env,
                                         node_ptr ctx1, node_ptr ctx2)
{
  return CompileFlatten_concat_contexts(env, ctx1, ctx2);
}

/*!
  \brief Pop a context

  Pop a context
*/
node_ptr hrc_prefix_utils_pop_context(const NuSMVEnv_ptr env,
                                      node_ptr ctx)
{
  return CompileFlatten_pop_context(env, ctx);
}

/*!
  \brief Returns a contextualized version of the expression

  Returns a contextualized version of the expression

  \sa hrc_prefix_utils_concat_context
*/

node_ptr hrc_prefix_utils_contextualize_expr(const NuSMVEnv_ptr env,
                                             node_ptr expr,
                                             node_ptr context)
{
  const NodeMgr_ptr nodemgr =
    NODE_MGR(NuSMVEnv_get_value(env, ENV_NODE_MGR));
  node_ptr _expr;
  if (Nil == expr) return Nil;

  if (CONTEXT == node_get_type(expr)) {
    /* Concatenate contexts */
    context = hrc_prefix_utils_concat_context(env, context, car(expr));
    _expr = cdr(expr);
  }
  else {
    _expr = expr;
  }

  return find_node(nodemgr, CONTEXT, context, _expr);
}



/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/
