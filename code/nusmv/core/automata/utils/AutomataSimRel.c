/* ---------------------------------------------------------------------------

  This file is part of the ``utils'' package.
  %COPYRIGHT%

-----------------------------------------------------------------------------*/

/*!
  \author Sergio Mover
  \brief Implementation of class 'AutomataSimRel'

  This class stores a simulation relation among locations in
  an automaton and 

*/

#include "nusmv/core/automata/utils/AutomataSimRel.h"
#include "nusmv/core/automata/utils/AutomataLocHash.h"
#include "nusmv/core/node/node.h"
#include "nusmv/core/utils/assoc.h"
#include "nusmv/core/utils/utils.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

typedef struct AutomataSimRel_TAG
{
  /* -------------------------------------------------- */
  /*                  Private members                   */
  /* -------------------------------------------------- */
  hash_ptr* sim_rel_hash;
  AutomataLoc_ptr* bisim_array;
  int n_locations;

} AutomataSimRel;



/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/*!
  \brief Constant used as value in simulation relation hash.

  
*/
#define AUTOMATON_SIMREL_FLAG NODE_FROM_INT(1)

/*!
  \brief Get the location in the bisimulation array using the
  given index_location.

  
*/
#define AUTOMATON_SIMREL_GET_ARRAY(array, index_location) \
  array[AutomataLoc_get_id(index_location) - 1]

/*!
  \brief Insert a location into bisimulation array using the given
  id.

  
*/
#define AUTOMATON_SIMREL_INSERT_ARRAY(array, index_location, value_location) \
  AUTOMATON_SIMREL_GET_ARRAY(array, index_location) = value_location


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static void automata_sim_rel_init(AutomataSimRel_ptr self, int n_locations);
static void automata_sim_rel_deinit(AutomataSimRel_ptr self);

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

AutomataSimRel_ptr AutomataSimRel_create(int n_locations)
{
  AutomataSimRel_ptr self = ALLOC(AutomataSimRel, 1);
  AUTOMATA_SIM_REL_CHECK_INSTANCE(self);

  automata_sim_rel_init(self, n_locations);
  return self;
}

void AutomataSimRel_destroy(AutomataSimRel_ptr self)
{
  AUTOMATA_SIM_REL_CHECK_INSTANCE(self);

  automata_sim_rel_deinit(self);
  FREE(self);
}

void AutomataSimRel_insert(AutomataSimRel_ptr self,
                           AutomataLoc_ptr first_key,
                           AutomataLoc_ptr second_key)
{
  AUTOMATA_SIM_REL_CHECK_INSTANCE(self);

  /* Insert (first_key, second_key) in the simulation relation */

  insert_assoc(AUTOMATON_SIMREL_GET_ARRAY(self->sim_rel_hash, first_key),
               NODE_PTR(second_key),
               AUTOMATON_SIMREL_FLAG);

  /* Populates hash of representants */
  if (first_key == second_key) {
    nusmv_assert(AutomataLoc_get_id(first_key) <= self->n_locations);
    AUTOMATON_SIMREL_INSERT_ARRAY(self->bisim_array,
                                  first_key,
                                  first_key);
  }
  else {
    node_ptr inverse_couple;

    inverse_couple = find_assoc(AUTOMATON_SIMREL_GET_ARRAY(self->sim_rel_hash, second_key),
                                NODE_PTR(first_key));

    if (Nil != inverse_couple) {
      /* Two locations are bisimilar - the representative is the minimum
         key contained in bisim_array */
      AutomataLoc_ptr min_first;
      AutomataLoc_ptr min_second;

      /* locations are different -> IDs are different */
      nusmv_assert(AutomataLoc_get_id(first_key) != AutomataLoc_get_id(second_key));

      min_first = AUTOMATON_SIMREL_GET_ARRAY(self->bisim_array,
                                             first_key);
      min_second = AUTOMATON_SIMREL_GET_ARRAY(self->bisim_array,
                                              second_key);

  if (AutomataLoc_get_id(min_second) < AutomataLoc_get_id(min_first)) {
        nusmv_assert(AutomataLoc_get_id(second_key) <= self->n_locations);
        AUTOMATON_SIMREL_INSERT_ARRAY(self->bisim_array,
                                      first_key,
                                      min_second);
        AUTOMATON_SIMREL_INSERT_ARRAY(self->bisim_array,
                                      second_key,
                                      min_second);
      }
      else {
        nusmv_assert(AutomataLoc_get_id(first_key) <= self->n_locations);
        AUTOMATON_SIMREL_INSERT_ARRAY(self->bisim_array,
                                      first_key,
                                      min_first);
        AUTOMATON_SIMREL_INSERT_ARRAY(self->bisim_array,
                                      second_key,
                                      min_first);
      }
    }
  }

}

boolean AutomataSimRel_has_key(AutomataSimRel_ptr self,
                               AutomataLoc_ptr first_key,
                               AutomataLoc_ptr second_key)
{
  node_ptr found;

  AUTOMATA_SIM_REL_CHECK_INSTANCE(self);


  found = find_assoc(AUTOMATON_SIMREL_GET_ARRAY(self->sim_rel_hash, first_key),
                     NODE_PTR(second_key));

  return (Nil != found);
}

AutomataLoc_ptr AutomataSimRel_get_representative(AutomataSimRel_ptr self,
                                                    AutomataLoc_ptr location)
{
  AutomataLoc_ptr representative;

  AUTOMATA_SIM_REL_CHECK_INSTANCE(self);

  representative =
    AUTOMATA_LOC(AUTOMATON_SIMREL_GET_ARRAY(self->bisim_array, location));

  return representative;
}

Set_t AutomataSimRel_get_all_rep(AutomataSimRel_ptr self)
{
  int i;
  Set_t rep_set;

  AUTOMATA_SIM_REL_CHECK_INSTANCE(self);
  rep_set = Set_MakeEmpty();

  for (i =0; i< self->n_locations; i++) {
    rep_set = Set_AddMember(rep_set,
                            (Set_Element_t) self->bisim_array[i]);
  }

  return rep_set;
}

void AutomataSimRel_print(AutomataSimRel_ptr self, FILE* out)
{
  int i;
  boolean first;

  fprintf(out, "Simulation relation:\n");

  first = true;
  for (i = 0; i < self->n_locations; i++) {
    assoc_iter iter;
    AutomataLoc_ptr dest_location;

    ASSOC_FOREACH(self->sim_rel_hash[i], iter, &dest_location, NULL) {
      if (! first) fprintf(out, ", ");

      fprintf(out, "(%d, %d)", (i+1),
              AutomataLoc_get_id(dest_location));

      first = false;
    }
  }

  fprintf(out, "\n");
}

void AutomataSimRel_insert_bisimilar_list(AutomataSimRel_ptr self,
                                          Slist_ptr bisimilar_locations,
                                          AutomataLoc_ptr representant)
{
  Siter iter1, iter2;
  int representant_id;

  representant_id = AutomataLoc_get_id(representant);

  SLIST_FOREACH(bisimilar_locations, iter1) {
    AutomataLoc_ptr s1;
    s1 = AUTOMATA_LOC(Siter_element(iter1));

    /* Insert representant */
    nusmv_assert(representant_id <= AutomataLoc_get_id(s1));
    AUTOMATON_SIMREL_INSERT_ARRAY(self->bisim_array,
                                  s1,
                                  representant);

    /* Populates bisimulation relation */
    SLIST_FOREACH(bisimilar_locations, iter2) {
      AutomataLoc_ptr s2;
      s2 = AUTOMATA_LOC(Siter_element(iter2));

      /* Populates bisimulation relation */
      insert_assoc(AUTOMATON_SIMREL_GET_ARRAY(self->sim_rel_hash, s1),
                   NODE_PTR(s2),
                   AUTOMATON_SIMREL_FLAG);
      insert_assoc(AUTOMATON_SIMREL_GET_ARRAY(self->sim_rel_hash, s2),
                   NODE_PTR(s1),
                   AUTOMATON_SIMREL_FLAG);
    }
  }

}

void AutomataSimRel_check_consistency(AutomataSimRel_ptr self)
{
  int i;

  for (i = 0; i < self->n_locations; i++) {
    AutomataLoc_ptr rep;
    AutomataLoc_ptr rep_2;

    rep = self->bisim_array[i];

    if (rep != AUTOMATA_LOC(Nil)) {
      rep_2 = AUTOMATON_SIMREL_GET_ARRAY(self->bisim_array, rep);

      nusmv_assert(rep == rep_2);
    }
  }
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/*!
  \brief The AutomataSimRel class private initializer

  The AutomataSimRel class private initializer

  \sa AutomataSimRel_create
*/
static void automata_sim_rel_init(AutomataSimRel_ptr self, int n_locations)
{
  int i;

  /* members initialization */
  self->bisim_array = ALLOC(AutomataLoc_ptr, n_locations);

  self->sim_rel_hash = ALLOC(hash_ptr, n_locations);
  for (i = 0; i < n_locations; i++) {
    self->sim_rel_hash[i] = new_assoc();
    self->bisim_array[i] = AUTOMATA_LOC(Nil);
  }

  self->n_locations = n_locations;
}

/*!
  \brief The AutomataSimRel class private deinitializer

  The AutomataSimRel class private deinitializer

  \sa AutomataSimRel_destroy
*/
static void automata_sim_rel_deinit(AutomataSimRel_ptr self)
{
  int i;

  /* members deinitialization */
  for (i = 0; i < self->n_locations; i++) {
    free_assoc(self->sim_rel_hash[i]);
  }
  FREE((self->sim_rel_hash));

  FREE((self->bisim_array));
}

/**AutomaticEnd***************************************************************/

