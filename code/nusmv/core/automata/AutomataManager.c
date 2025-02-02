/* ---------------------------------------------------------------------------

  This file is part of the ``automata'' package.
  %COPYRIGHT%

-----------------------------------------------------------------------------*/

/*!
  \author Sergio Mover
  \brief Implementation of class 'AutomataManager'

  The automata manager is a data structure that is used for:
  - keep all the different options that determine the behaviour of
  library.
  - Memoize data structures.

*/

#include "nusmv/core/automata/AutomataManager.h"
#include "nusmv/core/automata/AutomataManager_private.h"

#include "nusmv/core/wff/ExprMgr.h"
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
/* See 'AutomataManager_private.h' for class 'AutomataManager' definition. */

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

static void automata_manager_finalize(Object_ptr object, void* dummy);


static Set_t
automata_manager_get_alphabet_from_iter(AutomataManager_ptr self,
                                        Set_t source_support_set,
                                        Set_Iterator_t support_iter);

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

AutomataManager_ptr AutomataManager_create(NuSMVEnv_ptr env)
{
  AutomataManager_ptr self = ALLOC(AutomataManager, 1);
  AUTOMATA_MANAGER_CHECK_INSTANCE(self);

  automata_manager_init(self, env);
  return self;
}

void AutomataManager_destroy(AutomataManager_ptr self)
{
  AUTOMATA_MANAGER_CHECK_INSTANCE(self);

  Object_destroy(OBJECT(self), NULL);
}

BoolEnc_ptr AutomataManager_get_bool_enc(AutomataManager_ptr self)
{
  AUTOMATA_MANAGER_CHECK_INSTANCE(self);

  return self->bool_enc;
}

BddEnc_ptr AutomataManager_get_bdd_enc(AutomataManager_ptr self)
{
  AUTOMATA_MANAGER_CHECK_INSTANCE(self);

  return self->bdd_enc;
}

VIRTUAL Automaton_Algorithm_Type
AutomataManager_get_algorithm_type(AutomataManager_ptr self)
{
  AUTOMATA_MANAGER_CHECK_INSTANCE(self);
  return self->get_algorithm_type(self);
}

VIRTUAL void
AutomataManager_set_algorithm_type(AutomataManager_ptr self,
                                   Automaton_Algorithm_Type type)
{
  AUTOMATA_MANAGER_CHECK_INSTANCE(self);
  self->set_algorithm_type(self, type);
}

VIRTUAL Automaton_Label_Type
AutomataManager_get_label_type(AutomataManager_ptr self)
{
  AUTOMATA_MANAGER_CHECK_INSTANCE(self);

  return self->get_label_type(self);
}

VIRTUAL void
AutomataManager_set_label_type(AutomataManager_ptr self,
                               Automaton_Label_Type label_type)
{
  AUTOMATA_MANAGER_CHECK_INSTANCE(self);
  self->set_label_type(self, label_type);
}

VIRTUAL Automaton_ReductionAlgorithm_Type
AutomataManager_get_reduction_algo_type(AutomataManager_ptr self)
{
  AUTOMATA_MANAGER_CHECK_INSTANCE(self);
  return self->get_reduction_algo_type(self);
}

VIRTUAL void
AutomataManager_set_reduction_algo_type(AutomataManager_ptr self,
                                        Automaton_ReductionAlgorithm_Type type)
{
  AUTOMATA_MANAGER_CHECK_INSTANCE(self);
  self->set_reduction_algo_type(self, type);
}

VIRTUAL boolean
AutomataManager_is_keep_automaton_type(AutomataManager_ptr self)
{
  AUTOMATA_MANAGER_CHECK_INSTANCE(self);
  return self->is_keep_automaton_type(self);
}

VIRTUAL void
AutomataManager_set_keep_automaton_type(AutomataManager_ptr self,
                                        boolean keep_automaton_type)
{
  AUTOMATA_MANAGER_CHECK_INSTANCE(self);
  self->set_keep_automaton_type(self, keep_automaton_type);
}

VIRTUAL boolean
AutomataManager_is_keep_complete(AutomataManager_ptr self)
{
  AUTOMATA_MANAGER_CHECK_INSTANCE(self);
  return self->is_keep_complete(self);
}

VIRTUAL void
AutomataManager_set_keep_complete(AutomataManager_ptr self,
                                  boolean keep_complete)
{
  AUTOMATA_MANAGER_CHECK_INSTANCE(self);
  self->set_keep_complete(self, keep_complete);
}

VIRTUAL boolean
AutomataManager_is_keep_support_set_in_location(AutomataManager_ptr self)
{
  AUTOMATA_MANAGER_CHECK_INSTANCE(self);
  return self->is_keep_support_set_in_location(self);
}

VIRTUAL void
AutomataManager_set_keep_support_set_in_location(AutomataManager_ptr self,
                                              boolean keep_support_set_in_location)
{
  AUTOMATA_MANAGER_CHECK_INSTANCE(self);
  self->set_keep_support_set_in_location(self, keep_support_set_in_location);
}

VIRTUAL boolean
AutomataManager_is_use_length_abstraction(AutomataManager_ptr self)
{
  AUTOMATA_MANAGER_CHECK_INSTANCE(self);
  return self->is_use_length_abstraction(self);
}

VIRTUAL void
AutomataManager_set_use_length_abstraction(AutomataManager_ptr self,
                                           boolean use_length_abstraction)
{
  AUTOMATA_MANAGER_CHECK_INSTANCE(self);
  self->set_use_length_abstraction(self, use_length_abstraction);
}

VIRTUAL boolean
AutomataManager_is_minimize_automaton(AutomataManager_ptr self)
{
  AUTOMATA_MANAGER_CHECK_INSTANCE(self);
  return self->is_minimize_automaton(self);
}

VIRTUAL void
AutomataManager_set_minimize_automaton(AutomataManager_ptr self,
                                       boolean minimize_automaton)
{
  AUTOMATA_MANAGER_CHECK_INSTANCE(self);
  self->set_minimize_automaton(self, minimize_automaton);
}

VIRTUAL boolean
AutomataManager_is_use_sim_reduction(AutomataManager_ptr self)
{
  AUTOMATA_MANAGER_CHECK_INSTANCE(self);
  return self->is_use_sim_reduction(self);
}

VIRTUAL void
AutomataManager_set_use_sim_reduction(AutomataManager_ptr self,
                                      boolean use_sim_reduction)
{
  AUTOMATA_MANAGER_CHECK_INSTANCE(self);
  self->set_use_sim_reduction(self, use_sim_reduction);
}

VIRTUAL Set_t
AutomataManager_get_alphabet(AutomataManager_ptr self,
                             Set_t support_set)
{
  AUTOMATA_MANAGER_CHECK_INSTANCE(self);
  return self->get_alphabet(self, support_set);
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

void automata_manager_init(AutomataManager_ptr self, NuSMVEnv_ptr env)
{
  /* base class initialization */
  env_object_init(ENV_OBJECT(self), env);

  /* members initialization */

  self->algorithm_type = TRANSITIONS_BASED;
  self->label_type = BDD_LABEL;
  self->red_algo_type = AUT_RED_ALG_BISIMULATION;
  self->keep_automaton_type = false;

  if (NuSMVEnv_has_value(env, ENV_BOOL_ENCODER)) {
    self->bool_enc = BOOL_ENC(NuSMVEnv_get_value(env, ENV_BOOL_ENCODER));
  }
  else self->bool_enc = BOOL_ENC(NULL);

  if (NuSMVEnv_has_value(env, ENV_BDD_ENCODER)) {
    self->bdd_enc = BDD_ENC(NuSMVEnv_get_value(env, ENV_BDD_ENCODER));
  }
  else self->bdd_enc = BDD_ENC(NULL);

  /* [SM] Found performance issues when using library in
     sugarNuSMV_FMCAD.
     When here suffix implication are managed a power set is
     build. With sink location and transitions to sink location this
     construction explodes. So the default value for this option is false.
   */
  self->keep_complete = false;

  /* [SM] When using transitions based approach keep the support set
     is unuseful
   */
  self->keep_support_set_in_location = false;
  self->minimize_automaton = false;
  self->use_sim_reduction = true;
  self->use_length_abstraction = false;


  /* virtual methods settings */
  OVERRIDE(Object, finalize) = automata_manager_finalize;

  /* Ovverride virtual methods: */
  OVERRIDE(AutomataManager,get_algorithm_type) =
    automata_manager_get_algorithm_type;
  OVERRIDE(AutomataManager,set_algorithm_type) =
    automata_manager_set_algorithm_type;
  OVERRIDE(AutomataManager,get_label_type) =
    automata_manager_get_label_type;
  OVERRIDE(AutomataManager,set_label_type) =
    automata_manager_set_label_type;
  OVERRIDE(AutomataManager,get_reduction_algo_type) =
    automata_manager_get_reduction_algo_type;
  OVERRIDE(AutomataManager,set_reduction_algo_type) =
    automata_manager_set_reduction_algo_type;
  OVERRIDE(AutomataManager,is_keep_automaton_type) =
    automata_manager_is_keep_automaton_type;
  OVERRIDE(AutomataManager,set_keep_automaton_type) =
    automata_manager_set_keep_automaton_type;
  OVERRIDE(AutomataManager,is_keep_complete) =
    automata_manager_is_keep_complete;
  OVERRIDE(AutomataManager,set_keep_complete) =
    automata_manager_set_keep_complete;
  OVERRIDE(AutomataManager,is_keep_support_set_in_location) =
    automata_manager_is_keep_support_set_in_location;
  OVERRIDE(AutomataManager,set_keep_support_set_in_location) =
    automata_manager_set_keep_support_set_in_location;
  OVERRIDE(AutomataManager,is_use_length_abstraction) =
    automata_manager_is_use_length_abstraction;
  OVERRIDE(AutomataManager,set_use_length_abstraction) =
    automata_manager_set_use_length_abstraction;
  OVERRIDE(AutomataManager,is_minimize_automaton) =
    automata_manager_is_minimize_automaton;
  OVERRIDE(AutomataManager,set_minimize_automaton) =
    automata_manager_set_minimize_automaton;
  OVERRIDE(AutomataManager,is_use_sim_reduction) =
    automata_manager_is_use_sim_reduction;
  OVERRIDE(AutomataManager,set_use_sim_reduction) =
    automata_manager_set_use_sim_reduction;
  OVERRIDE(AutomataManager,get_alphabet) = automata_manager_get_alphabet;
}

void automata_manager_deinit(AutomataManager_ptr self)
{
  /* members deinitialization */


  /* base class deinitialization */
  env_object_deinit(ENV_OBJECT(self));
}

Automaton_Algorithm_Type
automata_manager_get_algorithm_type(AutomataManager_ptr self)
{
  return self->algorithm_type;
}

void
automata_manager_set_algorithm_type(AutomataManager_ptr self,
                                    Automaton_Algorithm_Type type)
{
  self->algorithm_type = type;
}

Automaton_Label_Type automata_manager_get_label_type(AutomataManager_ptr self)
{
  return self->label_type;
}

void automata_manager_set_label_type(AutomataManager_ptr self,
                                     Automaton_Label_Type label_type)
{
  self->label_type = label_type;
}

Automaton_ReductionAlgorithm_Type
automata_manager_get_reduction_algo_type(AutomataManager_ptr self)
{
  return self->red_algo_type;
}

void
automata_manager_set_reduction_algo_type(AutomataManager_ptr self,
                                         Automaton_ReductionAlgorithm_Type type)
{
  self->red_algo_type = type;
}

boolean
automata_manager_is_keep_automaton_type(AutomataManager_ptr self)
{
  return self->keep_automaton_type;
}

void
automata_manager_set_keep_automaton_type(AutomataManager_ptr self,
                                         boolean keep_automaton_type)
{
  self->keep_automaton_type = keep_automaton_type;
}

boolean
automata_manager_is_keep_complete(AutomataManager_ptr self)
{
  return self->keep_complete;
}

void
automata_manager_set_keep_complete(AutomataManager_ptr self,
                                   boolean keep_complete)
{
  self->keep_complete = keep_complete;
}

boolean
automata_manager_is_keep_support_set_in_location(AutomataManager_ptr self)
{
  return self->keep_support_set_in_location;
}

void
automata_manager_set_keep_support_set_in_location(AutomataManager_ptr self,
                                               boolean keep_support_set_in_location)
{
  self->keep_support_set_in_location = keep_support_set_in_location;
}

boolean
automata_manager_is_use_length_abstraction(AutomataManager_ptr self)
{
  return self->use_length_abstraction;
}

void
automata_manager_set_use_length_abstraction(AutomataManager_ptr self,
                                            boolean use_length_abstraction)
{
  self->use_length_abstraction = use_length_abstraction;
}

boolean
automata_manager_is_minimize_automaton(AutomataManager_ptr self)
{
  return self->minimize_automaton;
}

void
automata_manager_set_minimize_automaton(AutomataManager_ptr self,
                                        boolean minimize_automaton)
{
  self->minimize_automaton = minimize_automaton;
}

boolean
automata_manager_is_use_sim_reduction(AutomataManager_ptr self)
{
  return self->use_sim_reduction;
}

void
automata_manager_set_use_sim_reduction(AutomataManager_ptr self,
                                       boolean use_sim_reduction)
{
  self->use_sim_reduction = use_sim_reduction;
}

Set_t
automata_manager_get_alphabet(AutomataManager_ptr self,
                              Set_t support_set)
{
  ListIter_ptr support_iter;

  support_iter = Set_GetFirstIter(support_set);

  return automata_manager_get_alphabet_from_iter(self,
                                                 support_set,
                                                 support_iter);
}

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/*!
  \brief The AutomataManager class virtual finalizer

  Called by the class destructor
*/
static void automata_manager_finalize(Object_ptr object, void* dummy)
{
  AutomataManager_ptr self = AUTOMATA_MANAGER(object);

  automata_manager_deinit(self);
  FREE(self);
}

/*!
  \brief Builds the alphabet that is formed by
  propositions contained in support_set_iter, an iterator on a set of
  atomic propositions

  Builds the alphabet formed by propositions
  contained in support_set_iter, an iterator over a set of atomic
  propositions.

  The iterator is used to avoid the need to copy of the original
  support set, contained in source_support_set, at every recursive call.

  The alphabet is the power set of the set represented by support_set_iter.
  The power set is built calling recursively
  automaton_build_alphabet_iterator. There are three cases to consider:
    - support_set_iter is empty: {TRUE}
    - |support_set_iter| = 1: {x,not x}, where x belongs to support_set_iter
    - |support_set_iter| > 1: compute a1, alphabet for support_set\{x}, then
  compute for every element e of a1, compute "x and a1" and "(not x)
  and a1", and adds these two set to the alphabet for support_set.

  Caller owns the returned set, so he has to free it

  \sa automata_manager_get_alphabet
*/
static Set_t
automata_manager_get_alphabet_from_iter(AutomataManager_ptr self,
                                        Set_t source_support_set,
                                        Set_Iterator_t support_iter)
{
  Set_t alphabet = Set_MakeEmpty();
  const NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(self));
  const ExprMgr_ptr exprs = EXPR_MGR(NuSMVEnv_get_value(env, ENV_EXPR_MANAGER));


  /* Content of support_iter determine three three different cases:
       - support_itert is empty: the correspondent alphabet is true
       - support_iter has one element: alphabet contains the element
       and its negation.
       - support_iter has more than one element: the alphabet for
       support set without one element is built recursively. Every
       element from this set is combined with the missing
       element and its negation to build the final alphabet.
  */

  if(Set_IsEndIter(support_iter)){
    /* support set is empty  */
    alphabet = Set_MakeSingleton(ExprMgr_true(exprs));
  }else if(Set_IsEndIter(Set_GetNextIter(support_iter))){
    /* support set has ONLY ONE proposition.
       Builds the set that contains the proposition and its negation.
    */
    Set_Element_t set_element;
    Expr_ptr proposition;
    Expr_ptr negated_proposition;

    set_element = Set_GetMember(source_support_set, support_iter);
    EXPR_CHECK_INSTANCE(set_element);

    proposition = EXPR(set_element);
    negated_proposition = ExprMgr_not(exprs, proposition);

    alphabet = Set_MakeSingleton(proposition);
    alphabet = Set_AddMember(alphabet, negated_proposition);
  }else{
    /* support_iter contains still more than one element. */
    Set_t prev_alphabet;
    Set_Iterator_t next_iter;
    Set_Element_t set_element;
    Expr_ptr proposition;
    Expr_ptr negated_proposition;
    Set_Iterator_t alphabet_iter;

    /* Extract the first proposition and generates
       negated_proposition */
    set_element = Set_GetMember(source_support_set, support_iter);
    EXPR_CHECK_INSTANCE(set_element);
    proposition = EXPR(set_element);
    negated_proposition = ExprMgr_not(exprs, proposition);

    /* builds recursively the alphabet for support\{atom} */
    next_iter = Set_GetNextIter(support_iter);
    prev_alphabet = automata_manager_get_alphabet_from_iter(self,
                                                            source_support_set,
                                                            next_iter);

    /* Combine every element in support\{proposition} alphabet with
       proposition and NOT(proposition) */
    SET_FOREACH(prev_alphabet, alphabet_iter){
      Set_Element_t alphabet_elem;
      Expr_ptr old_symbol;
      Expr_ptr new_symbol;
      Expr_ptr new_symbol_negated;

      alphabet_elem = Set_GetMember(prev_alphabet, alphabet_iter);
      EXPR_CHECK_INSTANCE(alphabet_elem);
      old_symbol = EXPR(alphabet_elem);

      new_symbol = ExprMgr_and(exprs, old_symbol, proposition);
      alphabet = Set_AddMember(alphabet, new_symbol);

      new_symbol_negated = ExprMgr_and(exprs, old_symbol, negated_proposition);
      alphabet = Set_AddMember(alphabet, new_symbol_negated);
    }

    /* Destroy the previous alphabet */
    Set_ReleaseSet(prev_alphabet);
  }

  return alphabet;
}


/**AutomaticEnd***************************************************************/

