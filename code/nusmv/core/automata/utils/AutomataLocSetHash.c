/* ---------------------------------------------------------------------------

  This file is part of the ``utils'' package.
  %COPYRIGHT%

-----------------------------------------------------------------------------*/

/*!
  \author Sergio Mover
  \brief Implementation of class 'AutomataLocSetHash'

  AutomataLocSetHash is an hash table from set of
  locations to a single location.

  A boolean encoding is used to compute the hash of a location set.
  Given the maximum id of a location that can be contained in a set
  (eg. the number of locations of the automaton where the locations come)
  an enumeration variable is build.
  The enumeration variable has a range from 0 to
  number_of_locations. This step is performed when AutomataLocSetHash
  is created.

  Then the boolean encoding for the variable is created. The boolean
  encoding is represented by an "IF THEN ELSE" tree where:
    - internal node of the tree are IFTHENELSE node with a condition
  node, a then node and an else node. Then and else node can be
  another IFTHENELSE node or a NUMBER node.
    - leafs are BIT or NUMBER nodes.
  A condition node is a COLON node with a BIT child node. The BIT node
  is used to specify the bit number used for the comparison. For
  example if a IFTHENELSE node has a condition with a BIT node equals
  to 1, it means that:
    - in the then branch of the node there is the subtree that
  represents all the numbers that, when encoded in binary, have the
  bit 1 set to 1.
    - in the else branch of the node there is the subtree that
  represents all the numbers that, when encoded in binary, have the
  bit 1 set to 0.

  The hash key for a location set is built using this boolean
  encoding. Details for building the key are described in the
  automata_loc_hash_build_key function.

*/


#include "nusmv/core/node/NodeMgr.h"
#include "nusmv/core/node/printers/MasterPrinter.h"
#include "nusmv/core/automata/utils/AutomataLocSetHash.h"

#include "nusmv/core/compile/compile.h"
#include "nusmv/core/enc/enc.h"
#include "nusmv/core/parser/symbols.h"
#include "nusmv/core/node/node.h"
#include "nusmv/core/utils/NodeList.h"
#include "nusmv/core/utils/assoc.h"
#include "nusmv/core/utils/utils.h"
#include "nusmv/core/utils/ustring.h"

/*!
  \brief \todo Missing synopsis

  \todo Missing description
*/
#define AUTOMATA_LOC_HASH_LAYER_NAME "__AUTOMATA_LOC_HASH_LAYER__"

/*!
  \brief \todo Missing synopsis

  \todo Missing description
*/
#define AUTOMATA_LOC_HASH_VAR_PREFIX "__AUTOMATA_LOC_HASH_VAR__"

/*!
  \brief \todo Missing synopsis

  \todo Missing description
*/
#define AUTOMATA_LOC_HASH_VAR_PREFIX_SIZE 28

/*!
  \brief \todo Missing synopsis

  \todo Missing description
*/
#define ITE_GET_COND(x) (car(car(x)))

/*!
  \brief \todo Missing synopsis

  \todo Missing description
*/
#define ITE_GET_THEN(x) (cdr(car(x)))

/*!
  \brief \todo Missing synopsis

  \todo Missing description
*/
#define ITE_GET_ELSE(x) (cdr(x))

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

typedef struct AutomataLocSetHash_TAG
{
  /* -------------------------------------------------- */
  /*                  Private members                   */
  /* -------------------------------------------------- */
  int number_of_locations;
  node_ptr var_encoding;
  SymbLayer_ptr layer;
  st_table* hash_table;
  BoolEnc_ptr bool_enc;
} AutomataLocSetHash;



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
static void automata_loc_set_hash_init(AutomataLocSetHash_ptr self,
                                         BoolEnc_ptr bool_enc,
                                         int number_of_locations);

static void automata_loc_set_hash_deinit(AutomataLocSetHash_ptr self);

static node_ptr automata_loc_set_hash_build_key(AutomataLocSetHash_ptr self,
                                                  AutomataLocSet_ptr set);

static node_ptr
automata_loc_set_hash_build_key_recursive(AutomataLocSetHash_ptr self,
                                        node_ptr encoding,
                                        AutomataLocSet_ptr set);

static void
automata_loc_set_hash_encode_var(AutomataLocSetHash_ptr self);

static node_ptr
automata_loc_set_hash_declare_var(AutomataLocSetHash_ptr self);

static node_ptr
automata_loc_set_hash_get_fresh_var(AutomataLocSetHash_ptr self);

static SymbLayer_ptr automata_loc_set_hash_get_layer(SymbTable_ptr st);

static int automata_loc_set_hash_visit_tree_rec(const NuSMVEnv_ptr env,
                                                  FILE* out_stream,
                                                  node_ptr encoding,
                                                  int node_id);

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

AutomataLocSetHash_ptr AutomataLocSetHash_create(BoolEnc_ptr enc,
                                                     int number_of_locations)
{
  AutomataLocSetHash_ptr self = ALLOC(AutomataLocSetHash, 1);
  AUTOMATA_LOC_SET_HASH_CHECK_INSTANCE(self);

  automata_loc_set_hash_init(self, enc, number_of_locations);
  return self;
}

void AutomataLocSetHash_destroy(AutomataLocSetHash_ptr self)
{
  AUTOMATA_LOC_SET_HASH_CHECK_INSTANCE(self);

  automata_loc_set_hash_deinit(self);
  FREE(self);
}

void AutomataLocSetHash_insert (AutomataLocSetHash_ptr self,
                                  AutomataLocSet_ptr location_set,
                                  AutomataLoc_ptr value)
{
  node_ptr key;

  AUTOMATA_LOC_SET_HASH_CHECK_INSTANCE(self);
  AUTOMATA_LOC_SET_CHECK_INSTANCE(location_set);
  AUTOMATA_LOC_CHECK_INSTANCE(value);

  key = automata_loc_set_hash_build_key(self, location_set);

  insert_assoc(self->hash_table, key, NODE_PTR(value));
}

AutomataLoc_ptr AutomataLocSetHash_find(AutomataLocSetHash_ptr self,
                                            AutomataLocSet_ptr location_set)
{
  node_ptr key;
  node_ptr found_node;

  AUTOMATA_LOC_SET_HASH_CHECK_INSTANCE(self);
  AUTOMATA_LOC_SET_CHECK_INSTANCE(location_set);

  key = automata_loc_set_hash_build_key(self, location_set);

  found_node = find_assoc(self->hash_table, NODE_PTR(key));

  if (Nil == found_node) {
    return NULL;
  }else{
    AUTOMATA_LOC_CHECK_INSTANCE(found_node);
    return AUTOMATA_LOC(found_node);
  }
}

boolean AutomataLocSetHash_has_key (AutomataLocSetHash_ptr self,
                                      AutomataLocSet_ptr location_set)
{
  AUTOMATA_LOC_SET_HASH_CHECK_INSTANCE(self);
  AUTOMATA_LOC_CHECK_INSTANCE(location_set);

  return AutomataLocSetHash_find(self, location_set) != NULL;
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/*!
  \brief The AutomataLocSetHash class private initializer

  The AutomataLocSetHash class private initializer

  \sa AutomataLocSetHash_create
*/
static void automata_loc_set_hash_init(AutomataLocSetHash_ptr self,
                                         BoolEnc_ptr bool_enc,
                                         int number_of_locations)
{
  /* members initialization */
  self->number_of_locations = number_of_locations;
  self->hash_table = new_assoc();
  self->bool_enc = bool_enc;

  /* creates the encoding */
  automata_loc_set_hash_encode_var(self);
}

/*!
  \brief The AutomataLocSetHash class private deinitializer

  The AutomataLocSetHash class private deinitializer

  \sa AutomataLocSetHash_destroy
*/
static void automata_loc_set_hash_deinit(AutomataLocSetHash_ptr self)
{
  /* members deinitialization */
  BaseEnc_remove_layer(BASE_ENC(self->bool_enc),
                       AUTOMATA_LOC_HASH_LAYER_NAME);

  clear_assoc(self->hash_table);
  free_assoc(self->hash_table);
}

/*!
  \brief Given a set of locations builds its hash.

  Returns the hash of the location set. The
  building of the hash is performed by the
  automata_loc_set_hash_build_key_recursive function, invoked with the
  encoding present in self->var_encoding

  \sa automata_loc_set_hash_build_key_recursive
*/

static node_ptr automata_loc_set_hash_build_key(AutomataLocSetHash_ptr self,
                                                  AutomataLocSet_ptr set)
{
  node_ptr key;

  AUTOMATA_LOC_SET_HASH_CHECK_INSTANCE(self);
  AUTOMATA_LOC_SET_CHECK_INSTANCE(set);

  key = automata_loc_set_hash_build_key_recursive(self, self->var_encoding, set);

  /* FOR DEBUG PURPOSES */
/*   automata_loc_set_hash_visit_tree_rec(errstream, key, 0); */

  return key;
}

/*!
  \brief Given a set of locations builds its hash.

  Given a set of locations builds a key suitable to
  be used in an hash table.

  The key is built from the boolean encoding self->var_encoding. The
  encoding is an IFTHENELSE tree that represent an enumeration
  variable from 0 to number_of_locations.

  A new tree is the key for the set. The tree is built traversing the
  encoding in pre-order, not considering condition nodes in the
  traversal.
  When a NUMBER leaf is reached then a set inclusion operation is
  performed, to check if the number is contained in the ids of the
  location set. If the number is in the set then a node NUMBER with value
  1 is returned. If the number is not in the set a node NUMBER with
  value 0 is returned.
  When a IFTHENELSE node is reached the build is called on the THEN
  and on the ELSE childs. The subtree obtained from THEN and ELSE
  childs are then combined: if the two subtrees are the same, one of
  the two is returned; if they are different a new IFTHENELSE is
  built, with the same condition but with the two new THEN and ELSE
  nodes.

  \sa automata_loc_set_hash_build_key
*/

static node_ptr
automata_loc_set_hash_build_key_recursive(AutomataLocSetHash_ptr self,
                                            node_ptr encoding_tree,
                                            AutomataLocSet_ptr set)
{
  int type;
  node_ptr new_node;
  boolean is_contained;

  AUTOMATA_LOC_SET_HASH_CHECK_INSTANCE(self);
  AUTOMATA_LOC_SET_CHECK_INSTANCE(set);
  nusmv_assert(encoding_tree != NULL);

  type = node_get_type(encoding_tree);

  {
    const NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(set));
    const NodeMgr_ptr nodemgr =
      NODE_MGR(NuSMVEnv_get_value(env, ENV_NODE_MGR));

    switch(type) {
    case NUMBER:
      /* BASE CASE: performs the IN operation  */
      is_contained =
        AutomataLocSet_is_member_from_id_node(set, encoding_tree);

      if(is_contained) {
        new_node = find_node(nodemgr, NUMBER, NODE_FROM_INT(1), Nil);
      }else{
        new_node = find_node(nodemgr, NUMBER, NODE_FROM_INT(0), Nil);
      }
      break;

    case IFTHENELSE:
      /* Evaluate THEN branch and then ELSE branch */
      {
        node_ptr then_node;
        node_ptr else_node;

        then_node =
          automata_loc_set_hash_build_key_recursive(self, ITE_GET_THEN(encoding_tree), set);
        else_node =
          automata_loc_set_hash_build_key_recursive(self, ITE_GET_ELSE(encoding_tree), set);

        if(then_node == else_node) {
          /* if the nodes are the same, then the current node is not
             important as key. We can keep only one of the two  */
          return then_node;
        }else{
          /* Combines the nodes */
          node_ptr colon_node;
          colon_node = find_node(nodemgr, COLON, ITE_GET_COND(encoding_tree), then_node);
          new_node = find_node(nodemgr, IFTHENELSE, colon_node, else_node);
        }
      }
      break;
    case BOOLEAN:
      /* SPECIAL CASE: when number_of_locations is 1 then the encoding is
         a BOOLEAN node. */

      is_contained =
        AutomataLocSet_is_member_from_id(set, 1);

      if(is_contained) {
        new_node = find_node(nodemgr, NUMBER, NODE_FROM_INT(1), Nil);
      }else{
        new_node = find_node(nodemgr, NUMBER, NODE_FROM_INT(0), Nil);
      }
      break;
    default:
      /* Raise an error */
      error_unreachable_code();
    }

  }
  return new_node;
}

/*!
  \brief Creates the encoding that will be used for
  building hash keys.

  
*/
static void automata_loc_set_hash_encode_var(AutomataLocSetHash_ptr self)
{
  extern cmp_struct_ptr cmps;
  node_ptr var;
  SymbTable_ptr st = BaseEnc_get_symb_table(BASE_ENC(self->bool_enc));
  node_ptr var_encoding;


  /* hierarchy must be already created */
  nusmv_assert(cmp_struct_get_flatten_hrc(cmps));

  self->layer = automata_loc_set_hash_get_layer(st);

  /* Declares my variable */
  var = automata_loc_set_hash_declare_var(self);

/*   cmp_struct_set_encode_variables(cmps); */
/*   cmp_struct_set_flatten_hrc(cmps); */

  BaseEnc_commit_layer(BASE_ENC(self->bool_enc), AUTOMATA_LOC_HASH_LAYER_NAME);
  var_encoding = BoolEnc_get_var_encoding(self->bool_enc, var);

  /* Returned encoding must not be null */
  nusmv_assert(var_encoding != NULL);


  self->var_encoding = var_encoding;
}

/*!
  \brief Declare the variable that is used to get the
  boolean encoding.

  
*/

static node_ptr automata_loc_set_hash_declare_var (AutomataLocSetHash_ptr self)
{
  const NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(self->bool_enc));
  SymbType_ptr symbolicType;
  node_ptr expanded_range;
  node_ptr var;

  /* Creates an enumeartion type from 0 to number_of_locations */
  expanded_range = CompileFlatten_expand_range(env, 0, self->number_of_locations);
  symbolicType = SymbType_create(env, SYMB_TYPE_ENUM, expanded_range);

  /* get a fresh name for the variable */
  var = automata_loc_set_hash_get_fresh_var(self);
  SymbLayer_declare_state_var(self->layer, var, symbolicType);

  return var;
}

/*!
  \brief Get a new variable for the hash table layer.

  
*/
static node_ptr automata_loc_set_hash_get_fresh_var(AutomataLocSetHash_ptr self)
{
  const NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(self->bool_enc));
  const NodeMgr_ptr nodemgr =
    NODE_MGR(NuSMVEnv_get_value(env, ENV_NODE_MGR));
  UStringMgr_ptr strings = USTRING_MGR(NuSMVEnv_get_value(env, ENV_STRING_MGR));
  int suffix_index;
  boolean can_declare_var;
  node_ptr var;
  char* var_name;

  /* maybe is better a random generation for index */
  suffix_index = 0;
  can_declare_var = false;

  /* loops until a new variable can be declared */
  while(! can_declare_var) {

    /* Creates a new name for the variable */
    /* sure there is a better way to get the size of the converted number */
    int suffix_dim = 1; /* takes in account of the end 0 */
    int suffix_division = suffix_index == 0 ? 1 : suffix_index;
    while(suffix_division != 0) {
      suffix_division = suffix_division / 10;
      suffix_dim = suffix_dim + 1;
    }

    var_name = (char *)ALLOC(char,
                             suffix_dim +
                             AUTOMATA_LOC_HASH_VAR_PREFIX_SIZE +
                             1);
    sprintf(var_name, "%s_%d", AUTOMATA_LOC_HASH_VAR_PREFIX, suffix_index);

    /* declare the unique node  */
    var = new_node(nodemgr, ATOM,(node_ptr)UStringMgr_find_string(strings, var_name), Nil);
    can_declare_var = SymbLayer_can_declare_var(self->layer, var);

    suffix_index = suffix_index + 1;
  }

  return var;
}

/*!
  \brief Creates the symbolic layer used to declare the variable

  
*/
static SymbLayer_ptr automata_loc_set_hash_get_layer(SymbTable_ptr st)
{
  extern cmp_struct_ptr cmps;
  SymbLayer_ptr layer;

  /* Tries to get the layer AUTOMATA_LOC_HASH_LAYER_NAME. If the
     layer does not exist it is created. */

  layer = SymbTable_get_layer(st, AUTOMATA_LOC_HASH_LAYER_NAME);
  if(layer == NULL) {
  layer = SymbTable_create_layer(st,
                                 AUTOMATA_LOC_HASH_LAYER_NAME,
                                 SYMB_LAYER_POS_BOTTOM);
  }

  return layer;
}

/*!
  \brief Debug function which prints the encoding (key) of a set.

  Debug function which prints the encoding (key) of a set.
*/
static int automata_loc_set_hash_visit_tree_rec(const NuSMVEnv_ptr env,
                                                  FILE* out_stream,
                                                  node_ptr encoding,
                                                  int node_id)
{
  const MasterPrinter_ptr wffprint = MASTER_PRINTER(NuSMVEnv_get_value(env, ENV_WFF_PRINTER));
  int type;

  if(encoding == NULL || encoding == Nil) {
    return node_id;
  }

  type = node_get_type(encoding);

  if (type == IFTHENELSE) {
    node_ptr condition;
    int last_id;

    condition = ITE_GET_COND(encoding);

    /* print label node */
    fprintf(out_stream,
            "node%d [ label = \"%d_%d_",
            node_id, node_id, type);

    print_node(wffprint, out_stream, condition);
    fprintf(out_stream, "\"]\n");

    /* print connection */
    fprintf(out_stream,
            "\"node%d\" -> \"node%d\"\n",
            node_id,
            (node_id + 1));

    /* recursive call */
    last_id =
      automata_loc_set_hash_visit_tree_rec(env, out_stream,
                                             ITE_GET_THEN(encoding),
                                             (node_id + 1)); /* was last_id */

    fprintf(out_stream,
            "\"node%d\" -> \"node%d\"\n",
            node_id,
            (last_id + 1));

    return automata_loc_set_hash_visit_tree_rec(env, out_stream,
                                                  ITE_GET_ELSE(encoding),
                                                  (last_id + 1));
  } else if (type == NUMBER || type == BIT) {
    /* print label node */
    fprintf(out_stream,
            "node%d [ label = \"%d_%d_",
            node_id,
            node_id,
            type);

    print_node(wffprint, out_stream, encoding);
    fprintf(out_stream,"\"]\n");

    return node_id;

  } else if (type == COLON){
    int last_id;

    fprintf(out_stream,
            "node%d [ label = \"%d_%d\"]\n",
            node_id,
            node_id,
            type);

    last_id = node_id;

    if (car(encoding) != NULL){
      fprintf(out_stream,
              "\"node%d\" -> \"node%d\"\n",
              node_id,
              (node_id + 1));
       last_id =
         automata_loc_set_hash_visit_tree_rec(env, out_stream,
                                                car(encoding),
                                                (node_id + 1));
    } else if (cdr(encoding) != NULL) {
      fprintf(out_stream,
              "\"node%d\" -> \"node%d\"\n",
              node_id,
              (last_id + 1));

      return automata_loc_set_hash_visit_tree_rec(env, out_stream,
                                                    cdr(encoding),
                                                    (last_id + 1));

    } else {
      return node_id;
    }

  } else {
    fprintf(out_stream,
              "node%d [ label = \"%d_%d\"]\n",
              node_id,
              node_id,
              type);

      return node_id;
  }

  return node_id;
}

/**AutomaticEnd***************************************************************/

