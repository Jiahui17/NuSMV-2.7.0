/* ---------------------------------------------------------------------------

  This file is part of the ``utils'' package.
  %COPYRIGHT%

-----------------------------------------------------------------------------*/

/*!
  \author Sergio Mover
  \brief Implementation of class 'AutomataLocSet'

  Set implementation for locations of automaton. This
  implementation is needed to make hashing of set of locations fast.

  Set of locations are kept in a Set_t structure.
  A set of location id is mantained in a NodeList_ptr structure. The list
  is used when an hash of the set of locations is needed.

  Services provided by this class are:
    - Insertion of a new location in the set
    - Perform the union of two sets.

*/


#include "nusmv/core/node/NodeMgr.h"
#include "nusmv/core/automata/utils/AutomataLocSet.h"

#include "nusmv/core/parser/symbols.h"
#include "nusmv/core/utils/NodeList.h"
#include "nusmv/core/utils/utils.h"
#include "nusmv/core/utils/EnvObject_private.h"
#include "nusmv/core/utils/EnvObject.h"


/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

typedef struct AutomataLocSet_TAG
{
  INHERITS_FROM(EnvObject);

  /* -------------------------------------------------- */
  /*                  Private members                   */
  /* -------------------------------------------------- */
  Set_t location_set;
  NodeList_ptr id_list;
  int max_id; /* the max id in the set */


} AutomataLocSet;



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

static void automata_loc_set_init(AutomataLocSet_ptr self,
                                    const NuSMVEnv_ptr env);
static void automata_loc_set_deinit(AutomataLocSet_ptr self);

static void automata_loc_set_finalize(Object_ptr object, void* dummy);

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

AutomataLocSet_ptr AutomataLocSet_create(const NuSMVEnv_ptr env)
{
  AutomataLocSet_ptr self = ALLOC(AutomataLocSet, 1);
  AUTOMATA_LOC_SET_CHECK_INSTANCE(self);

  automata_loc_set_init(self, env);
  return self;
}

void AutomataLocSet_destroy(AutomataLocSet_ptr self)
{
  AUTOMATA_LOC_SET_CHECK_INSTANCE(self);

  Object_destroy(OBJECT(self), NULL);
}

void AutomataLocSet_add_element(AutomataLocSet_ptr self,
                                  AutomataLoc_ptr location)
{
  NuSMVEnv_ptr env;
  NodeMgr_ptr nodemgr;
  int location_id;
  node_ptr node_id;

  AUTOMATA_LOC_SET_CHECK_INSTANCE(self);
  AUTOMATA_LOC_CHECK_INSTANCE(location);

  env = EnvObject_get_environment(ENV_OBJECT(self));
  nodemgr = NODE_MGR(NuSMVEnv_get_value(env, ENV_NODE_MGR));

  /* Adds the id of the location to the NodeList and the location to the
     location set */

  /* Creates the node_ptr correspondent to the location id */
  location_id = AutomataLoc_get_id(location);
  node_id = find_node(nodemgr, NUMBER, NODE_FROM_INT(location_id), Nil);

  if(! NodeList_belongs_to(self->id_list, node_id)) {
    NodeList_append(self->id_list, node_id);

    /* no location must have the same id */
    self->location_set = Set_AddMember(self->location_set, (Set_Element_t) location);

    /* Update max_id */
    if(self->max_id < location_id)
      self->max_id = location_id;
  }
}

void AutomataLocSet_union(AutomataLocSet_ptr self,
                            AutomataLocSet_ptr location_set)
{
  AUTOMATA_LOC_SET_CHECK_INSTANCE(self);
  AUTOMATA_LOC_SET_CHECK_INSTANCE(location_set);

  /* performs the union of location_set */
  self->location_set = Set_Union(self->location_set, location_set->location_set);
  /* performs the union of ids */
  NodeList_concat_unique(self->id_list, location_set->id_list);

  /* update max_id if necessary */
  if(self->max_id < location_set->max_id)
    self->max_id = location_set->max_id;
}

boolean AutomataLocSet_is_empty (AutomataLocSet_ptr self)
{
  AUTOMATA_LOC_SET_CHECK_INSTANCE(self);

  return Set_IsEmpty(self->location_set);
}

Set_t AutomataLocSet_get_set (AutomataLocSet_ptr self)
{
  AUTOMATA_LOC_SET_CHECK_INSTANCE(self);
  return self->location_set;
}

Set_Iterator_t AutomataLocSet_get_first_iter (AutomataLocSet_ptr self)
{
  AUTOMATA_LOC_SET_CHECK_INSTANCE(self);

  return Set_GetFirstIter(self->location_set);
}

AutomataLoc_ptr AutomataLocSet_get_member (AutomataLocSet_ptr self,
                                               Set_Iterator_t iter)
{
  AUTOMATA_LOC_SET_CHECK_INSTANCE(self);
  nusmv_assert(iter != NULL);

  return AUTOMATA_LOC(Set_GetMember(self->location_set, iter));
}

boolean AutomataLocSet_is_member_from_id (AutomataLocSet_ptr self,
                                            int id)
{
  node_ptr node_id;
  NuSMVEnv_ptr env;
  NodeMgr_ptr nodemgr;

  nusmv_assert(id >= 0);

  env = EnvObject_get_environment(ENV_OBJECT(self));
  nodemgr = NODE_MGR(NuSMVEnv_get_value(env, ENV_NODE_MGR));
  node_id = find_node(nodemgr, NUMBER, NODE_FROM_INT(id), Nil);

  return AutomataLocSet_is_member_from_id_node(self, node_id);
}

boolean AutomataLocSet_is_member_from_id_node (AutomataLocSet_ptr self,
                                                 node_ptr id_node)

{
  AUTOMATA_LOC_SET_CHECK_INSTANCE(self);
  nusmv_assert(id_node != NULL);

  return NodeList_belongs_to(self->id_list, id_node);
}

boolean AutomataLocSet_is_memeber (AutomataLocSet_ptr self,
                                     AutomataLoc_ptr location)
{
  AUTOMATA_LOC_SET_CHECK_INSTANCE(self);
  AUTOMATA_LOC_CHECK_INSTANCE(location);

  return Set_IsMember(self->location_set, (Set_Element_t) location);
}

int AutomataLocSet_get_max_id(AutomataLocSet_ptr self)
{
  AUTOMATA_LOC_SET_CHECK_INSTANCE(self);
  return self->max_id;
}

AutomataLocSet_ptr AutomataLocSet_copy (AutomataLocSet_ptr self)
{
  NuSMVEnv_ptr env;
  AutomataLocSet_ptr copy;

  AUTOMATA_LOC_SET_CHECK_INSTANCE(self);

  env = EnvObject_get_environment(ENV_OBJECT(self));
  copy = AutomataLocSet_create(env);

  /* copy the location set */
  copy->location_set = Set_Copy(self->location_set);

  /* copy the node list */
  copy->id_list = NodeList_copy(self->id_list);

  /* copy the max id */
  copy->max_id = self->max_id;

  return copy;
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/*!
  \brief The AutomataLocSet class private initializer

  The AutomataLocSet class private initializer

  \sa AutomataLocSet_create
*/
static void automata_loc_set_init(AutomataLocSet_ptr self,
                                    const NuSMVEnv_ptr env)
{
  env_object_init(ENV_OBJECT(self), env);

  /* members initialization */
  self->location_set = Set_MakeEmpty();
  self->id_list = NodeList_create();
  self->max_id = 0;
}

/*!
  \brief The AutomataLocSet class private deinitializer

  The AutomataLocSet class private deinitializer

  \sa AutomataLocSet_destroy
*/
static void automata_loc_set_deinit(AutomataLocSet_ptr self)
{
  env_object_deinit(ENV_OBJECT(self));

  /* members deinitialization */
  Set_ReleaseSet(self->location_set);
  NodeList_destroy(self->id_list);

  OVERRIDE(Object, finalize) = automata_loc_set_finalize;
}

/*!
  \brief The AutomataLocSet class virtual deinitializer

  The AutomataLocSet class virtual deinitializer

  \sa AutomataLocSet_destroy
*/
static void automata_loc_set_finalize(Object_ptr object, void* dummy)
{
  AutomataLocSet_ptr self = AUTOMATA_LOC_SET(object);

  automata_loc_set_deinit(self);

  FREE(self);
}


/**AutomaticEnd***************************************************************/

