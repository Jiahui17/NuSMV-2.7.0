// WARNING: not compilable, the activity is not done yet

#ifndef __SYMB_TABLE_HH__
#define __SYMB_TABLE_HH__

#if HAVE_CONFIG_H
#  include "nusmv-config.h"
#endif

#include "nusmv/core/utils/EnvObject_.h"
#include "nusmv/core/compile/symb_table/SymbCache_.h"
#include <vector>

extern "C" {
#include "nusmv/core/compile/symb_table/SymbLayer.h"
#include "nusmv/core/cinit/NuSMVEnv.h"
#include "nusmv/core/compile/type_checking/TypeChecker.h"
#include "nusmv/core/utils/utils.h"
#include "nusmv/core/utils/array.h"
#include "nusmv/core/utils/assoc.h"
#include "nusmv/core/node/node.h"
#include "nusmv/core/compile/symb_table/NFunction.h"
#include "nusmv/core/set/set.h"
#include "nusmv/core/compile/symb_table/ResolveSymbol.h"
#include "nusmv/core/utils/UStringMgr.h"
#include "nusmv/core/utils/Pair.h"
#include "nusmv/core/opt/OptsHandler.h"
#include "nusmv/core/utils/StreamMgr.h"
}


namespace nusmv {

  typedef class SymbTable* SymbTable_ptr;


  typedef void (*SymbTableForeachFun)(const SymbTable* st,
				      const node_ptr sym,
				      void* arg);


  struct SymbTableIter {
    unsigned int index;
    unsigned int mask;
    SymbTableIterFilterFun filter;
    SymbTable* st;
    void* arg;
  };

#define SYMB_TABLE_FOREACH(self, iter, mask)	\
  for (SymbTable_gen_iter(self, &iter, mask);	\
       !SymbTable_iter_is_end(self, &iter);	\
       SymbTable_iter_next(self, &iter))

#define SYMB_TABLE_FOREACH_FILTER(self, iter, mask, filter, arg)        \
  for (SymbTable_gen_iter(self, &iter, mask),                           \
         SymbTable_iter_set_filter(self, &iter, filter, arg);           \
       !SymbTable_iter_is_end(self, &iter);                             \
       SymbTable_iter_next(self, &iter))



  class SymbTable : public EnvObject
  {
  public:
    SymbTable(const NuSMVEnv_ptr env);

    virtual ~SymbTable();

    TypeChecker_ptr get_type_checker() const;

    void gen_iter(SymbTableIter* iter, unsigned int mask) const;
    void iter_next(SymbTableIter* iter) const;
    bool iter_is_end(const SymbTableIter* iter) const;
    node_ptr iter_get_symbol(const SymbTableIter* iter) const;

    void iter_set_filter(SymbTableIter* iter, SymbTableIterFilterFun fun,
			 void* arg) const;

    void foreach(SymbTableForeachFun fun, void* arg) const;

    Set_t iter_to_set(SymbTableIter iter) const;

    NodeList_ptr iter_to_list(SymbTableIter iter) const;

    size_t iter_count(SymbTableIter iter) const;

    void add_trigger(SymbTableTriggerFun trigger,
		     SymbTableTriggerAction action,
		     void* arg1, bool must_free_arg) const;

    void remove_trigger(SymbTableTriggerFun trigger,
			SymbTableTriggerAction action) const;

    /* -------------------------------------- */
    /*            Built-in filters            */
    /* -------------------------------------- */
    bool iter_filter_i_symbols(const node_ptr sym, void* arg) const;
    bool iter_filter_sf_i_symbols(const node_ptr sym, void* arg) const;
    bool iter_filter_sf_symbols(const node_ptr sym, void* arg) const;

    /* -------------------------------------- */
    /*            Layers handling             */
    /* -------------------------------------- */
    SymbLayer_ptr create_layer(const string layer_name,
			       const LayerInsertPolicy ins_policy);
    void remove_layer(SymbLayer_ptr layer);
    SymbLayer_ptr get_layer(const string layer_name) const;
    bool has_layer(const string layer_name) const;
    void rename_layer(const string layer_name, const string new_name) const;
    const std::vector<SymbLayer_ptr>& get_layers() const;

    /* -------------------------------------- */
    /*                Symbols                 */
    /* -------------------------------------- */

    /* Lists of symbols: */
    std::vector<node_ptr> get_layers_sf_symbols(const vector<const string>& layer_names);
    std::vector<node_ptr> get_layers_sf_vars(const vector<const string>& layer_names);
    std::vector<node_ptr> get_layers_i_symbols(const vector<const string>& layer_names);
    std::vector<node_ptr> get_layers_i_vars(const vector<const string>& layer_names);
    std::vector<node_ptr> get_layers_sf_i_symbols(const vector<const string>& layer_names);
    std::vector<node_ptr> get_layers_sf_i_vars(const vector<const string>& layer_names);

    /* Number of symbols: */
    size_t get_vars_num() const;
    size_t get_state_vars_num() const;
    size_t get_frozen_vars_num() const;
    size_t get_input_vars_num() const;
    size_t get_defines_num() const;
    size_t get_array_defines_num() const;
    size_t get_parameters_num() const;
    size_t get_constants_num() const;
    size_t get_functions_num() const;

    /* Classes of layers: */
    void create_layer_class(const string class_name);
    bool layer_class_exists(const string class_name);
    void layer_add_to_class(const string layer_name,
			    const string class_name);
    void layer_remove_from_class(const string layer_name,
				 const string class_name);
    const vector<const string>& get_class_layer_names(const string class_name);
    bool is_layer_in_class(const string layer_name, const string class_name);
    void set_default_layers_class_name(const string class_name);
    const string get_default_layers_class_name() const;


    // ======================================================================
    // RC: From here on not fixed, ignore!
    // ======================================================================

    /* Symbols related info: */
    SymbType_ptr get_var_type(const node_ptr name) const;
    node_ptr get_define_body(const node_ptr name) const;
    NFunction_ptr get_function(const node_ptr name) const;
    node_ptr get_actual_parameter(const node_ptr name) const;

node_ptr get_array_define_body(    const node_ptr name) const;
SymbType_ptr get_variable_array_type(    const node_ptr name) const;

node_ptr get_define_flatten_body(const node_ptr name) const;

node_ptr get_flatten_actual_parameter(   const node_ptr name) const;

node_ptr get_array_define_flatten_body(   const node_ptr name) const;

node_ptr get_define_context(const node_ptr name) const;

node_ptr get_function_context(    const node_ptr name) const;

node_ptr get_actual_parameter_context(const node_ptr name) const;

node_ptr get_array_define_context(  const node_ptr name) const;

SymbCategory get_symbol_category( const node_ptr name) const;

/* Queries: */
bool is_symbol_state_var( const node_ptr name) const;
bool is_symbol_frozen_var(  const node_ptr name) const;

bool is_symbol_state_frozen_var(   const node_ptr name) const;

bool is_symbol_input_var( const node_ptr name) const;

bool is_symbol_var(

bool is_symbol_bool_var(const node_ptr name) const;

bool is_symbol_declared(const node_ptr name) const;

bool is_symbol_define(
                                 const node_ptr name) const;

bool is_symbol_function(  const node_ptr name) const;

bool is_symbol_parameter(
                                 const node_ptr name) const;

bool is_symbol_array_define(const node_ptr name) const;

bool is_symbol_variable_array(   const node_ptr name) const;

bool is_symbol_constant(const node_ptr name) const;

bool is_var_finite(


bool list_contains_input_var(const NodeList_ptr var_list) const;

bool list_contains_state_frozen_var(   const NodeList_ptr var_list) const;

bool list_contains_undef_var(const NodeList_ptr var_list) const;

Set_t get_type_tags() const;

bool contains_infinite_precision_variables() const;

bool contains_enum_variables() const;

SymbLayer_ptr variable_get_layer(node_ptr name);

SymbLayer_ptr define_get_layer(node_ptr name);

SymbLayer_ptr symbol_get_layer(node_ptr name);

SymbLayer_ptr function_get_layer(node_ptr name);

node_ptr get_determinization_var_name() const;

node_ptr get_fresh_symbol_name(   const char * tplate);

hash_ptr get_simplification_hash(SymbTable_ptr self);

const string get_class_of_layer(const string layer_name) const;

ResolveSymbol_ptr resolve_symbol(
                               node_ptr expr, node_ptr context);
 ptr copy(  Set_t blacklist);

hash_ptr get_handled_hash_ptr
(
      const string key,
      ST_PFICPCP compare_func,
      ST_PFICPI hash_func,
      ST_PFSR destroy_func,
      SymbTableTriggerFun add_action,
      SymbTableTriggerFun remove_action,
      SymbTableTriggerFun redeclare_action
      );

void clear_handled_remove_action_hash(const node_ptr sym,     SymbTableTriggerAction action,     void* arg) const;

  private:
    OptsHandler_ptr options;
    StreamMgr_ptr streams;
    NodeMgr_ptr nodes;
    MasterPrinter_ptr printer;
    UStringMgr_ptr strings;

    SymbCache cache;

    hash_ptr categories;

    int temp_layer_name_suffix; /* used to create temporary names for layers */

    std::vector<SymbLayer_ptr> layers; /* the list of owned layers */

    hash_ptr class_layers; /* the list of layers organized by class */
    NodeList_ptr class_names; /* the list of class names */
    const string default_class_name; /* name of the default class name */

    hash_ptr name2layer; /* to associate layers and names */

    /* A counter for declaration of determinization variables */
    size_t det_counter;

    TypeChecker_ptr tc; /* this is the type checker owned by the ST */

    hash_ptr expr_simplify_hash; /* hash for function Expr_simplify */
    ResolveSymbol_ptr resolver;
    hash_ptr handled_structures;

  };


}

#endif /* __SYMB_TABLE_HH__ */
