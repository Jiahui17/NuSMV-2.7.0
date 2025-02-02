/* ---------------------------------------------------------------------------


  This file is part of the ``utils'' package.
  %COPYRIGHT%

-----------------------------------------------------------------------------*/

/*!
  \author Sergio Mover
  \brief Implementation of class 'AutomataPredecessorsList'

  AutomataPredecessorsList keeps for every location of an
  automaton a list of predecessors.

  A constraint is that ids of the automaton must range from 1 to location_count.


*/


#include "nusmv/core/automata/utils/AutomataPredecessorsList.h"
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

typedef struct AutomataPredecessorsList_TAG
{
  /* -------------------------------------------------- */
  /*                  Private members                   */
  /* -------------------------------------------------- */
  int location_count;
  Slist_ptr* predecessors_lists;

} AutomataPredecessorsList;



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

static void automata_predecessors_list_init(AutomataPredecessorsList_ptr self,
                                            int location_count);
static void automata_predecessors_list_deinit(AutomataPredecessorsList_ptr self);


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

AutomataPredecessorsList_ptr AutomataPredecessorsList_create(int location_count)
{
  AutomataPredecessorsList_ptr self = ALLOC(AutomataPredecessorsList, 1);
  AUTOMATA_PREDECESSORS_LIST_CHECK_INSTANCE(self);

  automata_predecessors_list_init(self, location_count);
  return self;
}

void AutomataPredecessorsList_destroy(AutomataPredecessorsList_ptr self)
{
  AUTOMATA_PREDECESSORS_LIST_CHECK_INSTANCE(self);

  automata_predecessors_list_deinit(self);
  FREE(self);
}

void AutomataPredecessorsList_insert(AutomataPredecessorsList_ptr self,
                                     AutomataLoc_ptr location,
                                     AutomataLoc_ptr predecessor)
{
  Slist_ptr predecessors_list;

  AUTOMATA_PREDECESSORS_LIST_CHECK_INSTANCE(self);
  AUTOMATA_LOC_CHECK_INSTANCE(location);
  AUTOMATA_LOC_CHECK_INSTANCE(predecessor);

  predecessors_list = AutomataPredecessorsList_get_predecessor(self, location);
  Slist_push(predecessors_list, predecessor);

  return;
}

Slist_ptr AutomataPredecessorsList_get_predecessor(AutomataPredecessorsList_ptr self,
                                                   AutomataLoc_ptr location)
{
  int list_index;

  AUTOMATA_PREDECESSORS_LIST_CHECK_INSTANCE(self);
  AUTOMATA_LOC_CHECK_INSTANCE(location);

  list_index = AutomataLoc_get_id(location) - 1;
  nusmv_assert(list_index >= 0 && list_index < self->location_count);

  if (NULL == self->predecessors_lists[list_index]) {
    self->predecessors_lists[list_index] = Slist_create();
  }

  return self->predecessors_lists[list_index];
}

void AutomataPredecessorsList_add_pred_to_list(AutomataPredecessorsList_ptr self,
                                               AutomataLoc_ptr location,
                                               Slist_ptr predeccessors_list)
{
  int list_index;

  AUTOMATA_PREDECESSORS_LIST_CHECK_INSTANCE(self);
  AUTOMATA_LOC_CHECK_INSTANCE(location);

  list_index = AutomataLoc_get_id(location) - 1;
  nusmv_assert(list_index >= 0 && list_index < self->location_count);

  if (NULL == self->predecessors_lists[list_index]) {
    self->predecessors_lists[list_index] = Slist_create();
  }
  else {
    Siter iter;
    SLIST_FOREACH(self->predecessors_lists[list_index], iter) {
      AutomataLoc_ptr reach_location = AUTOMATA_LOC(Siter_element(iter));
      if (location != reach_location)
        Slist_push(predeccessors_list, reach_location);
    }
  }

  return;
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/*!
  \brief The AutomataPredecessorsList class private initializer

  The AutomataPredecessorsList class private initializer

  \sa AutomataPredecessorsList_create
*/
static void automata_predecessors_list_init(AutomataPredecessorsList_ptr self,
                                            int location_count)
{
  /* members initialization */
  int i;

  self->location_count = location_count;
  self->predecessors_lists = ALLOC(Slist_ptr, location_count);

  for (i=0; i<self->location_count; i++)
    self->predecessors_lists[i] = NULL;
}

/*!
  \brief The AutomataPredecessorsList class private deinitializer

  The AutomataPredecessorsList class private deinitializer

  \sa AutomataPredecessorsList_destroy
*/
static void automata_predecessors_list_deinit(AutomataPredecessorsList_ptr self)
{
  /* members deinitialization */
  int i;

  /* Free all lists if they were initialized */
  for (i=0; i<self->location_count; i++) {
    if (NULL != self->predecessors_lists[i]) {
      Slist_destroy(self->predecessors_lists[i]);
    }
  }

  FREE(self->predecessors_lists);
}



/**AutomaticEnd***************************************************************/
