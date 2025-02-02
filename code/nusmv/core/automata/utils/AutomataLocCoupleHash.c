/* ---------------------------------------------------------------------------

  This file is part of the ``utils'' package.
  %COPYRIGHT%

-----------------------------------------------------------------------------*/

/*!
  \author Sergio Mover
  \brief Implementation of class 'AutomataLocCoupleHash'

  AutomataLocCoupleHash provides an hash map from a
  couple of AutomataLoc pointers to a pointer.

  Given a couple of location (first_location, second_location) creates a key
  for the couple, that is a list of nodes that contains two element,
  the first is the id of the first location and the second is the id of
  the second location.

  This key is used for hashing the pair.

*/

#include "nusmv/core/node/NodeMgr.h"
#include "nusmv/core/automata/utils/AutomataLocCoupleHash.h"
#include "nusmv/core/node/node.h"
#include "nusmv/core/parser/symbols.h"
#include "nusmv/core/utils/assoc.h"
#include "nusmv/core/utils/utils.h"
#include "nusmv/core/utils/EnvObject_private.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

typedef struct AutomataLocCoupleHash_TAG
{
  INHERITS_FROM(EnvObject);

  /* -------------------------------------------------- */
  /*                  Private members                   */
  /* -------------------------------------------------- */
  st_table* location_hash;

} AutomataLocCoupleHash;



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
static void
automata_loc_couple_hash_init(AutomataLocCoupleHash_ptr self,
                                const NuSMVEnv_ptr env);

static void
automata_loc_couple_hash_deinit(AutomataLocCoupleHash_ptr self);

static void automata_loc_couple_hash_finalize(Object_ptr object, void* dummy);

static node_ptr
automata_loc_couple_hash_generate_key(AutomataLocCoupleHash_ptr self,
                                        AutomataLoc_ptr location1,
                                        AutomataLoc_ptr location2);

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

AutomataLocCoupleHash_ptr AutomataLocCoupleHash_create(const NuSMVEnv_ptr env)
{
  AutomataLocCoupleHash_ptr self = ALLOC(AutomataLocCoupleHash, 1);
  AUTOMATA_LOC_COUPLE_HASH_CHECK_INSTANCE(self);

  automata_loc_couple_hash_init(self, env);
  return self;
}

void AutomataLocCoupleHash_destroy(AutomataLocCoupleHash_ptr self)
{
  AUTOMATA_LOC_COUPLE_HASH_CHECK_INSTANCE(self);
  Object_destroy(OBJECT(self), NULL);
}

void AutomataLocCoupleHash_insert(AutomataLocCoupleHash_ptr self,
                                    AutomataLoc_ptr first_key,
                                    AutomataLoc_ptr second_key,
                                    void* value)
{
  node_ptr key;

  AUTOMATA_LOC_COUPLE_HASH_CHECK_INSTANCE(self);
  AUTOMATA_LOC_CHECK_INSTANCE(first_key);
  AUTOMATA_LOC_CHECK_INSTANCE(second_key);
  nusmv_assert (Nil != value);

  /* generates the key for the couple */
  key = automata_loc_couple_hash_generate_key(self, first_key, second_key);

  /* Insert value in the second_hash with key second_value */
  insert_assoc(self->location_hash, key, NODE_PTR(value));

  return;
}

void* AutomataLocCoupleHash_find(AutomataLocCoupleHash_ptr self,
                                   AutomataLoc_ptr first_key,
                                   AutomataLoc_ptr second_key)
{
  node_ptr found_node = NULL;
  node_ptr key = NULL;

  AUTOMATA_LOC_COUPLE_HASH_CHECK_INSTANCE(self);
  AUTOMATA_LOC_CHECK_INSTANCE(first_key);
  AUTOMATA_LOC_CHECK_INSTANCE(second_key);

  /* generates the key for the couple */
  key = automata_loc_couple_hash_generate_key(self, first_key, second_key);

  found_node = find_assoc(self->location_hash, key);
  if(Nil == found_node) {
    /* no value found for key  */
    return NULL;
  }else{
    nusmv_assert((void*) Nil != found_node);
    return (void*) found_node;
  }
}

boolean AutomataLocCoupleHash_has_key(AutomataLocCoupleHash_ptr self,
                                        AutomataLoc_ptr first_key,
                                        AutomataLoc_ptr second_key)
{
  AUTOMATA_LOC_COUPLE_HASH_CHECK_INSTANCE(self);
  AUTOMATA_LOC_CHECK_INSTANCE(first_key);
  AUTOMATA_LOC_CHECK_INSTANCE(second_key);

  return AutomataLocCoupleHash_find(self, first_key, second_key) != NULL;
}

node_ptr AutomataLocCoupleHash_get_keys(AutomataLocCoupleHash_ptr self)
{
  const NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(self));
  const NodeMgr_ptr nodemgr =
    NODE_MGR(NuSMVEnv_get_value(env, ENV_NODE_MGR));

  node_ptr keys = assoc_get_keys(self->location_hash, nodemgr, false);

  return keys;
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/*!
  \brief The AutomataLocCoupleHash class private initializer

  The AutomataLocCoupleHash class private initializer

  \sa AutomataLocCoupleHash_create
*/
static void automata_loc_couple_hash_init(AutomataLocCoupleHash_ptr self,
                                            const NuSMVEnv_ptr env)
{
  env_object_init(ENV_OBJECT(self), env);

  /* members initialization */
  self->location_hash = new_assoc();

  OVERRIDE(Object, finalize) = automata_loc_couple_hash_finalize;
}

/*!
  \brief Builds the key used in the hash table formed
  from two locations.

  Builds a key from the ids of the two location. The
  key is a CONS node that has as first element the NUMBER node created
  for the id of the first location, and as second element the NUMBER node
  created for the id of the second location.
*/
static node_ptr automata_loc_couple_hash_generate_key(AutomataLocCoupleHash_ptr self,
                                                        AutomataLoc_ptr location1,
                                                        AutomataLoc_ptr location2)
{
  const NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(self));
  const NodeMgr_ptr nodemgr =
    NODE_MGR(NuSMVEnv_get_value(env, ENV_NODE_MGR));

  node_ptr first_id_node;
  node_ptr second_id_node;
  node_ptr couple;

  first_id_node = find_node(nodemgr, NUMBER,
                            NODE_FROM_INT(AutomataLoc_get_id(location1)),
                            Nil);
  second_id_node = find_node(nodemgr, NUMBER,
                             NODE_FROM_INT(AutomataLoc_get_id(location2)),
                             Nil);

  /* key is the concatenation of second node and first node */
  couple = find_node(nodemgr, CONS, first_id_node, second_id_node);

  return couple;
}

/*!
  \brief The AutomataLocCoupleHash class private deinitializer

  The AutomataLocCoupleHash class private deinitializer

  \sa AutomataLocCoupleHash_destroy
*/
static void automata_loc_couple_hash_deinit(AutomataLocCoupleHash_ptr self)
{
  /* members deinitialization */
  clear_assoc(self->location_hash);
  free_assoc(self->location_hash);

  env_object_deinit(ENV_OBJECT(self));
}

/*!
  \brief \todo Missing synopsis

  \todo Missing description
*/
static void automata_loc_couple_hash_finalize(Object_ptr object, void* dummy)
{
  AutomataLocCoupleHash_ptr self = AUTOMATA_LOC_COUPLE_HASH(object);

  automata_loc_couple_hash_deinit(self);

  FREE(self);
}

/**AutomaticEnd***************************************************************/

