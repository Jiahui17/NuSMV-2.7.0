/* ---------------------------------------------------------------------------

  This file is part of the ``utils'' package.
  %COPYRIGHT%

-----------------------------------------------------------------------------*/

/*!
  \author Sergio Mover
  \brief Implementation of class 'AutomataLocHash'

  AutomataLocHash is an hash table that has as key
  location pointers and as values location pointers.

  The implementation uses the assoc and wraps its
  functionalities, hiding details. 
  

*/

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

typedef struct AutomataLocHash_TAG
{
  /* -------------------------------------------------- */
  /*                  Private members                   */
  /* -------------------------------------------------- */
  st_table* hash;  

} AutomataLocHash;



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

static void automata_loc_hash_init(AutomataLocHash_ptr self);
static void automata_loc_hash_deinit(AutomataLocHash_ptr self);


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

AutomataLocHash_ptr AutomataLocHash_create()
{
  AutomataLocHash_ptr self = ALLOC(AutomataLocHash, 1);
  AUTOMATA_LOC_HASH_CHECK_INSTANCE(self);

  automata_loc_hash_init(self);
  return self;
}

void AutomataLocHash_destroy(AutomataLocHash_ptr self)
{
  AUTOMATA_LOC_HASH_CHECK_INSTANCE(self);

  automata_loc_hash_deinit(self);
  FREE(self);
}

void AutomataLocHash_insert(AutomataLocHash_ptr self, 
                              AutomataLoc_ptr key,
                              AutomataLoc_ptr value)
{
  AUTOMATA_LOC_HASH_CHECK_INSTANCE(self);
  AUTOMATA_LOC_CHECK_INSTANCE(key);
  AUTOMATA_LOC_CHECK_INSTANCE(value);

  insert_assoc(self->hash, NODE_PTR(key), NODE_PTR(value));
}

AutomataLoc_ptr AutomataLocHash_find(AutomataLocHash_ptr self, 
                                         AutomataLoc_ptr key)
{
  node_ptr found_node;

  AUTOMATA_LOC_HASH_CHECK_INSTANCE(self);
  AUTOMATA_LOC_CHECK_INSTANCE(key);
  found_node = find_assoc(self->hash, NODE_PTR(key));

  if (Nil == found_node) {
    return NULL;
  }else{
    AUTOMATA_LOC_CHECK_INSTANCE(found_node);  
    return AUTOMATA_LOC(found_node);    
  }
}

boolean AutomataLocHash_has_key(AutomataLocHash_ptr self, 
                                  AutomataLoc_ptr key)
{
  AUTOMATA_LOC_HASH_CHECK_INSTANCE(self);
  AUTOMATA_LOC_CHECK_INSTANCE(key);

  return AutomataLocHash_find(self, key) != NULL;
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/*!
  \brief The AutomataLocHash class private initializer

  The AutomataLocHash class private initializer

  \sa AutomataLocHash_create
*/
static void automata_loc_hash_init(AutomataLocHash_ptr self)
{
  /* members initialization */
  self->hash = new_assoc();
}

/*!
  \brief The AutomataLocHash class private deinitializer

  The AutomataLocHash class private deinitializer

  \sa AutomataLocHash_destroy
*/
static void automata_loc_hash_deinit(AutomataLocHash_ptr self)
{
  /* members deinitialization */
  clear_assoc(self->hash); 

  free_assoc(self->hash); 
}



/**AutomaticEnd***************************************************************/

