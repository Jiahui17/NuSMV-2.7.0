/* ---------------------------------------------------------------------------


  This file is part of the ``node.printers'' package of NuSMV version 2.
  Copyright (C) 2006 by FBK-irst.

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
  \author Roberto Cavada
  \brief Implementaion of class 'MasterPrinter', derived from
  MasterNodeWalker

  \todo: Missing description

*/


#if HAVE_CONFIG_H
#  include "nusmv-config.h"
#endif

#include "nusmv/core/node/printers/MasterPrinter.h"
#include "nusmv/core/node/printers/MasterPrinter_private.h"
#include "nusmv/core/node/printers/PrinterBase.h"
#include "nusmv/core/node/printers/PrinterBase_private.h"
#include "nusmv/core/node/MasterNodeWalker_private.h"
#include "nusmv/core/node/printers/MasterPrinter.h"
#include "nusmv/core/opt/opt.h"
#include "nusmv/core/node/printers/printersInt.h"
#include "nusmv/core/utils/ErrorMgr.h"
#include "nusmv/core/utils/NodeList.h"
#include "nusmv/core/utils/error.h"
#include "nusmv/core/utils/utils.h"
#include "nusmv/core/parser/symbols.h"

#if NUSMV_HAVE_STRING_H
#  include <string.h> /* for strdup */
#else
#  ifndef strdup
char* strdup(const char*); /* forward declaration */
#  endif
#endif

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

typedef struct MasterPrinter_TAG
{
  /* this MUST stay on the top */
  INHERITS_FROM(MasterNodeWalker);

  /* -------------------------------------------------- */
  /*                  Private members                   */
  /* -------------------------------------------------- */
  StreamType stream_type;
  union StreamTypeArg stream_arg;

  NodeList_ptr indent_stack; /* internal indentation levels stack */
  int current_ofs;  /* current offset on the current line */

  /* string stream is handled by using these varaibles: */
  char* sstream;       /* string stream */
  size_t sstream_cap;  /* sstream allocation capacity */
  size_t sstream_len;  /* sstream current usage */
  size_t sstream_grow_sum; /* sstream grow sum */
  int sstream_grow_num; /* number of resizes */

  StreamMgr_ptr streams;

  int (*inner_stream_print)(MasterPrinter_ptr self, const char* str);
  int (*external_stream_print)(void* stream, const char* str);

} MasterPrinter;

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/*!
  \brief \todo Missing synopsis

  \todo Missing description
*/
#define BLANK_STR " "

/*!
  \brief \todo Missing synopsis

  \todo Missing description
*/
#define NEWLINE_CHR '\n'

/*!
  \brief \todo Missing synopsis

  \todo Missing description
*/
#define NEWLINE_STR "\n"

/*!
  \brief \todo Missing synopsis

  \todo Missing description
*/
#define TERM_CHR '\0'

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static void master_printer_init(MasterPrinter_ptr self,
                                const NuSMVEnv_ptr env);

static void master_printer_deinit(MasterPrinter_ptr self);

static void master_printer_finalize(Object_ptr object, void* dummy);

static int
master_printer_fprint(MasterPrinter_ptr self, const char* str);

static int
master_printer_sprint(MasterPrinter_ptr self, const char* str);

static int
master_printer_get_level(MasterPrinter_ptr self);

static void
master_printer_reset_string_stream(MasterPrinter_ptr self);

/*---------------------------------------------------------------------------*/
/* Definition of TOP LEVEL exported functions                                */
/*---------------------------------------------------------------------------*/

void debug_print_node(NuSMVEnv_ptr env, node_ptr node)
{
  const MasterPrinter_ptr wffprint =
    MASTER_PRINTER(NuSMVEnv_get_value(env, ENV_WFF_PRINTER));
  StreamMgr_ptr streams =
    STREAM_MGR(NuSMVEnv_get_value(env, ENV_STREAM_MANAGER));
  FILE* out = StreamMgr_get_output_stream(streams);

  print_node(wffprint, out, node);
}

void debug_print_sexp(NuSMVEnv_ptr env, node_ptr node)
{
  const MasterPrinter_ptr sexpprint =
    MASTER_PRINTER(NuSMVEnv_get_value(env, ENV_SEXP_PRINTER));
  StreamMgr_ptr streams =
    STREAM_MGR(NuSMVEnv_get_value(env, ENV_STREAM_MANAGER));
  FILE* out = StreamMgr_get_output_stream(streams);

  print_node(sexpprint, out, node);
}

void debug_print_node_type(const NuSMVEnv_ptr env, node_ptr n) 
{
  const MasterPrinter_ptr wffprint =
    MASTER_PRINTER(NuSMVEnv_get_value(env, ENV_WFF_PRINTER));
  StreamMgr_ptr streams =
    STREAM_MGR(NuSMVEnv_get_value(env, ENV_STREAM_MANAGER));
  FILE* out = StreamMgr_get_output_stream(streams);
  union StreamTypeArg sta;
  
  sta.file = out;
  MasterPrinter_set_stream_type(wffprint, STREAM_TYPE_FILE, &sta);
  MasterPrinter_reset_stream(wffprint, 0);

  switch (node_get_type(n)) 
  {
    case NUSMV_CORE_SYMBOL_FIRST:
      MasterPrinter_print_string(wffprint, "NUSMV_CORE_SYMBOL_FIRST");
      break;
    case TRANS:
      MasterPrinter_print_string(wffprint, "TRANS");
      break;
    case INIT:
      MasterPrinter_print_string(wffprint, "INIT");
      break;
    case INVAR:
      MasterPrinter_print_string(wffprint, "INVAR");
      break;
    case ASSIGN:
      MasterPrinter_print_string(wffprint, "ASSIGN");
      break;
    case FAIRNESS:
      MasterPrinter_print_string(wffprint, "FAIRNESS");
      break;
    case JUSTICE:
      MasterPrinter_print_string(wffprint, "JUSTICE");
      break;
    case COMPASSION:
      MasterPrinter_print_string(wffprint, "COMPASSION");
      break;
    case SPEC:
      MasterPrinter_print_string(wffprint, "SPEC");
      break;
    case LTLSPEC:
      MasterPrinter_print_string(wffprint, "LTLSPEC");
      break;
    case PSLSPEC:
      MasterPrinter_print_string(wffprint, "PSLSPEC");
      break;
    case INVARSPEC:
      MasterPrinter_print_string(wffprint, "INVARSPEC");
      break;
    case COMPUTE:
      MasterPrinter_print_string(wffprint, "COMPUTE");
      break;
    case DEFINE:
      MasterPrinter_print_string(wffprint, "DEFINE");
      break;
    case ISA:
      MasterPrinter_print_string(wffprint, "ISA");
      break;
    case GOTO:
      MasterPrinter_print_string(wffprint, "GOTO");
      break;
    case CONSTRAINT:
      MasterPrinter_print_string(wffprint, "CONSTRAINT");
      break;
    case MODULE:
      MasterPrinter_print_string(wffprint, "MODULE");
      break;
    case PROCESS:
      MasterPrinter_print_string(wffprint, "PROCESS");
      break;
    case MODTYPE:
      MasterPrinter_print_string(wffprint, "MODTYPE");
      break;
    case LAMBDA:
      MasterPrinter_print_string(wffprint, "LAMBDA");
      break;
    case CONSTANTS:
      MasterPrinter_print_string(wffprint, "CONSTANTS");
      break;
    case URGENT:
      MasterPrinter_print_string(wffprint, "URGENT");
      break;
    case PRED:
      MasterPrinter_print_string(wffprint, "PRED");
      break;
    case ATTIME:
      MasterPrinter_print_string(wffprint, "ATTIME");
      break;
    case PREDS_LIST:
      MasterPrinter_print_string(wffprint, "PREDS_LIST");
      break;
    case MIRROR:
      MasterPrinter_print_string(wffprint, "MIRROR");
      break;
    case PSYNTH_PROB:
      MasterPrinter_print_string(wffprint, "PSYNTH_PROB");
      break;
    case KLTL_PROB:
      MasterPrinter_print_string(wffprint, "KLTL_PROB");
      break;
    case DEFINE_PROPERTY:
      MasterPrinter_print_string(wffprint, "DEFINE_PROPERTY");
      break;
    case SYNTAX_ERROR:
      MasterPrinter_print_string(wffprint, "SYNTAX_ERROR");
      break;
    case NUSMV_STATEMENTS_SYMBOL_LAST:
      MasterPrinter_print_string(wffprint, "NUSMV_STATEMENTS_SYMBOL_LAST");
      break;
    case NUSMV_EXPR_SYMBOL_FIRST:
      MasterPrinter_print_string(wffprint, "NUSMV_EXPR_SYMBOL_FIRST");
      break;
    case FAILURE:
      MasterPrinter_print_string(wffprint, "FAILURE");
      break;
    case CONTEXT:
      MasterPrinter_print_string(wffprint, "CONTEXT");
      break;
    case EU:
      MasterPrinter_print_string(wffprint, "EU");
      break;
    case AU:
      MasterPrinter_print_string(wffprint, "AU");
      break;
    case EBU:
      MasterPrinter_print_string(wffprint, "EBU");
      break;
    case ABU:
      MasterPrinter_print_string(wffprint, "ABU");
      break;
    case MINU:
      MasterPrinter_print_string(wffprint, "MINU");
      break;
    case MAXU:
      MasterPrinter_print_string(wffprint, "MAXU");
      break;
    case VAR:
      MasterPrinter_print_string(wffprint, "VAR");
      break;
    case FROZENVAR:
      MasterPrinter_print_string(wffprint, "FROZENVAR");
      break;
    case IVAR:
      MasterPrinter_print_string(wffprint, "IVAR");
      break;
    case BOOLEAN:
      MasterPrinter_print_string(wffprint, "BOOLEAN");
      break;
    case CLOCK:
      MasterPrinter_print_string(wffprint, "CLOCK");
      break;
    case NON_CONTINUOUS:
      MasterPrinter_print_string(wffprint, "NON_CONTINUOUS");
      break;
    case ARRAY:
      MasterPrinter_print_string(wffprint, "ARRAY");
      break;
    case SCALAR:
      MasterPrinter_print_string(wffprint, "SCALAR");
      break;
    case CONS:
      MasterPrinter_print_string(wffprint, "CONS");
      break;
    case BDD:
      MasterPrinter_print_string(wffprint, "BDD");
      break;
    case SEMI:
      MasterPrinter_print_string(wffprint, "SEMI");
      break;
    case EQDEF:
      MasterPrinter_print_string(wffprint, "EQDEF");
      break;
    case TWODOTS:
      MasterPrinter_print_string(wffprint, "TWODOTS");
      break;
    case FALSEEXP:
      MasterPrinter_print_string(wffprint, "FALSEEXP");
      break;
    case TRUEEXP:
      MasterPrinter_print_string(wffprint, "TRUEEXP");
      break;
    case SELF:
      MasterPrinter_print_string(wffprint, "SELF");
      break;
    case CASE:
      MasterPrinter_print_string(wffprint, "CASE");
      break;
    case COLON:
      MasterPrinter_print_string(wffprint, "COLON");
      break;
    case IFTHENELSE:
      MasterPrinter_print_string(wffprint, "IFTHENELSE");
      break;
    case SIMPWFF:
      MasterPrinter_print_string(wffprint, "SIMPWFF");
      break;
    case NEXTWFF:
      MasterPrinter_print_string(wffprint, "NEXTWFF");
      break;
    case LTLWFF:
      MasterPrinter_print_string(wffprint, "LTLWFF");
      break;
    case CTLWFF:
      MasterPrinter_print_string(wffprint, "CTLWFF");
      break;
    case COMPWFF:
      MasterPrinter_print_string(wffprint, "COMPWFF");
      break;
    case ATOM:
      MasterPrinter_print_string(wffprint, "ATOM");
      break;
    case NUMBER:
      MasterPrinter_print_string(wffprint, "NUMBER");
      break;
    case COMMA:
      MasterPrinter_print_string(wffprint, "COMMA");
      break;
    case IMPLIES:
      MasterPrinter_print_string(wffprint, "IMPLIES");
      break;
    case IFF:
      MasterPrinter_print_string(wffprint, "IFF");
      break;
    case OR:
      MasterPrinter_print_string(wffprint, "OR");
      break;
    case XOR:
      MasterPrinter_print_string(wffprint, "XOR");
      break;
    case XNOR:
      MasterPrinter_print_string(wffprint, "XNOR");
      break;
    case AND:
      MasterPrinter_print_string(wffprint, "AND");
      break;
    case NOT:
      MasterPrinter_print_string(wffprint, "NOT");
      break;
    case EX:
      MasterPrinter_print_string(wffprint, "EX");
      break;
    case AX:
      MasterPrinter_print_string(wffprint, "AX");
      break;
    case EF:
      MasterPrinter_print_string(wffprint, "EF");
      break;
    case AF:
      MasterPrinter_print_string(wffprint, "AF");
      break;
    case EG:
      MasterPrinter_print_string(wffprint, "EG");
      break;
    case AG:
      MasterPrinter_print_string(wffprint, "AG");
      break;
    case SINCE:
      MasterPrinter_print_string(wffprint, "SINCE");
      break;
    case BOUNDED_SINCE:
      MasterPrinter_print_string(wffprint, "BOUNDED_SINCE");
      break;
    case UNTIL:
      MasterPrinter_print_string(wffprint, "UNTIL");
      break;
    case BOUNDED_UNTIL:
      MasterPrinter_print_string(wffprint, "BOUNDED_UNTIL");
      break;
    case TRIGGERED:
      MasterPrinter_print_string(wffprint, "TRIGGERED");
      break;
    case RELEASES:
      MasterPrinter_print_string(wffprint, "RELEASES");
      break;
    case EBF:
      MasterPrinter_print_string(wffprint, "EBF");
      break;
    case EBG:
      MasterPrinter_print_string(wffprint, "EBG");
      break;
    case ABF:
      MasterPrinter_print_string(wffprint, "ABF");
      break;
    case ABG:
      MasterPrinter_print_string(wffprint, "ABG");
      break;
    case OP_NEXT:
      MasterPrinter_print_string(wffprint, "OP_NEXT");
      break;
    case OP_GLOBAL:
      MasterPrinter_print_string(wffprint, "OP_GLOBAL");
      break;
    case OP_FUTURE:
      MasterPrinter_print_string(wffprint, "OP_FUTURE");
      break;
    case OP_PREC:
      MasterPrinter_print_string(wffprint, "OP_PREC");
      break;
    case OP_NOTPRECNOT:
      MasterPrinter_print_string(wffprint, "OP_NOTPRECNOT");
      break;
    case OP_HISTORICAL:
      MasterPrinter_print_string(wffprint, "OP_HISTORICAL");
      break;
    case OP_ONCE:
      MasterPrinter_print_string(wffprint, "OP_ONCE");
      break;
    case EQUAL:
      MasterPrinter_print_string(wffprint, "EQUAL");
      break;
    case NOTEQUAL:
      MasterPrinter_print_string(wffprint, "NOTEQUAL");
      break;
    case LT:
      MasterPrinter_print_string(wffprint, "LT");
      break;
    case GT:
      MasterPrinter_print_string(wffprint, "GT");
      break;
    case LE:
      MasterPrinter_print_string(wffprint, "LE");
      break;
    case GE:
      MasterPrinter_print_string(wffprint, "GE");
      break;
    case UNION:
      MasterPrinter_print_string(wffprint, "UNION");
      break;
    case SETIN:
      MasterPrinter_print_string(wffprint, "SETIN");
      break;
    case MOD:
      MasterPrinter_print_string(wffprint, "MOD");
      break;
    case PLUS:
      MasterPrinter_print_string(wffprint, "PLUS");
      break;
    case MINUS:
      MasterPrinter_print_string(wffprint, "MINUS");
      break;
    case TIMES:
      MasterPrinter_print_string(wffprint, "TIMES");
      break;
    case DIVIDE:
      MasterPrinter_print_string(wffprint, "DIVIDE");
      break;
    case UMINUS:
      MasterPrinter_print_string(wffprint, "UMINUS");
      break;
    case NEXT:
      MasterPrinter_print_string(wffprint, "NEXT");
      break;
    case SMALLINIT:
      MasterPrinter_print_string(wffprint, "SMALLINIT");
      break;
    case DOT:
      MasterPrinter_print_string(wffprint, "DOT");
      break;
    case BIT:
      MasterPrinter_print_string(wffprint, "BIT");
      break;
    case RANGE:
      MasterPrinter_print_string(wffprint, "RANGE");
      break;
    case UNSIGNED_WORD:
      MasterPrinter_print_string(wffprint, "UNSIGNED_WORD");
      break;
    case SIGNED_WORD:
      MasterPrinter_print_string(wffprint, "SIGNED_WORD");
      break;
    case INTEGER:
      MasterPrinter_print_string(wffprint, "INTEGER");
      break;
    case REAL:
      MasterPrinter_print_string(wffprint, "REAL");
      break;
    case CONTINUOUS:
      MasterPrinter_print_string(wffprint, "CONTINUOUS");
      break;
    case NUMBER_UNSIGNED_WORD:
      MasterPrinter_print_string(wffprint, "NUMBER_UNSIGNED_WORD");
      break;
    case NUMBER_SIGNED_WORD:
      MasterPrinter_print_string(wffprint, "NUMBER_SIGNED_WORD");
      break;
    case NUMBER_FRAC:
      MasterPrinter_print_string(wffprint, "NUMBER_FRAC");
      break;
    case NUMBER_REAL:
      MasterPrinter_print_string(wffprint, "NUMBER_REAL");
      break;
    case NUMBER_EXP:
      MasterPrinter_print_string(wffprint, "NUMBER_EXP");
      break;
    case LSHIFT:
      MasterPrinter_print_string(wffprint, "LSHIFT");
      break;
    case RSHIFT:
      MasterPrinter_print_string(wffprint, "RSHIFT");
      break;
    case LROTATE:
      MasterPrinter_print_string(wffprint, "LROTATE");
      break;
    case RROTATE:
      MasterPrinter_print_string(wffprint, "RROTATE");
      break;
    case BIT_SELECTION:
      MasterPrinter_print_string(wffprint, "BIT_SELECTION");
      break;
    case CONCATENATION:
      MasterPrinter_print_string(wffprint, "CONCATENATION");
      break;
    case CAST_BOOL:
      MasterPrinter_print_string(wffprint, "CAST_BOOL");
      break;
    case CAST_WORD1:
      MasterPrinter_print_string(wffprint, "CAST_WORD1");
      break;
    case CAST_SIGNED:
      MasterPrinter_print_string(wffprint, "CAST_SIGNED");
      break;
    case CAST_UNSIGNED:
      MasterPrinter_print_string(wffprint, "CAST_UNSIGNED");
      break;
    case EXTEND:
      MasterPrinter_print_string(wffprint, "EXTEND");
      break;
    case WORDARRAY:
      MasterPrinter_print_string(wffprint, "WORDARRAY");
      break;
    case WAREAD:
      MasterPrinter_print_string(wffprint, "WAREAD");
      break;
    case WAWRITE:
      MasterPrinter_print_string(wffprint, "WAWRITE");
      break;
    case UWCONST:
      MasterPrinter_print_string(wffprint, "UWCONST");
      break;
    case SWCONST:
      MasterPrinter_print_string(wffprint, "SWCONST");
      break;
    case WRESIZE:
      MasterPrinter_print_string(wffprint, "WRESIZE");
      break;
    case WSIZEOF:
      MasterPrinter_print_string(wffprint, "WSIZEOF");
      break;
    case CAST_TOINT:
      MasterPrinter_print_string(wffprint, "CAST_TOINT");
      break;
    case COMPID:
      MasterPrinter_print_string(wffprint, "COMPID");
      break;
    case ARRAY_TYPE:
      MasterPrinter_print_string(wffprint, "ARRAY_TYPE");
      break;
    case ARRAY_DEF:
      MasterPrinter_print_string(wffprint, "ARRAY_DEF");
      break;
    case NFUNCTION:
      MasterPrinter_print_string(wffprint, "NFUNCTION");
      break;
    case NFUNCTION_TYPE:
      MasterPrinter_print_string(wffprint, "NFUNCTION_TYPE");
      break;
    case FUN:
      MasterPrinter_print_string(wffprint, "FUN");
      break;
    case COUNT:
      MasterPrinter_print_string(wffprint, "COUNT");
      break;
    case FLOOR:
      MasterPrinter_print_string(wffprint, "FLOOR");
      break;
    case ITYPE:
      MasterPrinter_print_string(wffprint, "ITYPE");
      break;
    case WORDARRAY_TYPE:
      MasterPrinter_print_string(wffprint, "WORDARRAY_TYPE");
      break;
    case INTERNAL_ARRAY_STRUCT:
      MasterPrinter_print_string(wffprint, "INTERNAL_ARRAY_STRUCT");
      break;
    case CONST_ARRAY:
      MasterPrinter_print_string(wffprint, "CONST_ARRAY");
      break;
    case INTARRAY:
      MasterPrinter_print_string(wffprint, "INTARRAY");
      break;
    case INTARRAY_TYPE:
      MasterPrinter_print_string(wffprint, "INTARRAY_TYPE");
      break;
    case TYPEOF:
      MasterPrinter_print_string(wffprint, "TYPEOF");
      break;
    case CAST_TO_UNSIGNED_WORD:
      MasterPrinter_print_string(wffprint, "CAST_TO_UNSIGNED_WORD");
      break;
    case TIME:
      MasterPrinter_print_string(wffprint, "TIME");
      break;
    case TIME_UNTIL:
      MasterPrinter_print_string(wffprint, "TIME_UNTIL");
      break;
    case TIME_SINCE:
      MasterPrinter_print_string(wffprint, "TIME_SINCE");
      break;
    case OP_TIMED_NEXT:
      MasterPrinter_print_string(wffprint, "OP_TIMED_NEXT");
      break;
    case OP_TIMED_PREC:
      MasterPrinter_print_string(wffprint, "OP_TIMED_PREC");
      break;
    case NUSMV_EXPR_SYMBOL_LAST:
      MasterPrinter_print_string(wffprint, "NUSMV_EXPR_SYMBOL_LAST");
      break;
    case NUSMV_CORE_SYMBOL_LAST:
      MasterPrinter_print_string(wffprint, "NUSMV_CORE_SYMBOL_LAST");
      break;
    default:
      MasterPrinter_print_string(wffprint, "UNKNOWN_SYMBOL");
      break;
    }

     /* ensure proper flushing */
    MasterPrinter_flush_stream(wffprint);
}

int print_node(MasterPrinter_ptr wff_printer, FILE *stream, node_ptr n)
{
  union StreamTypeArg sta;
  sta.file = stream;

  MasterPrinter_set_stream_type(wff_printer, STREAM_TYPE_FILE, &sta);
  MasterPrinter_reset_stream(wff_printer, 0);

  return
    MasterPrinter_print_node(wff_printer, n)  &&

    /* ensure proper flushing */
    MasterPrinter_flush_stream(wff_printer);
}

char* sprint_node(MasterPrinter_ptr mp, node_ptr n)
{
  boolean success;
  MasterPrinter_set_stream_type(mp,
                                STREAM_TYPE_STRING,
                                STREAM_TYPE_ARG_UNUSED);
  MasterPrinter_reset_stream(mp, 0);

  success = (MasterPrinter_print_node(mp, n) &&
             MasterPrinter_flush_stream(mp));

  return (success)? strdup(MasterPrinter_get_streamed_string(mp)) : NULL;
}

int print_node_indent_at(MasterPrinter_ptr mp,
                         FILE *stream, node_ptr n, int ofs)
{
  union StreamTypeArg sta;
  sta.file = stream;

  MasterPrinter_set_stream_type(mp, STREAM_TYPE_FILE, &sta);
  MasterPrinter_reset_stream(mp, ofs);

  return
    MasterPrinter_print_node(mp, n) &&

    /* ensure proper flushing */
    MasterPrinter_flush_stream(mp);
}

char* sprint_node_indent_at(MasterPrinter_ptr mp, node_ptr n, int ofs)
{
  boolean success;

  MasterPrinter_set_stream_type(mp,
                                STREAM_TYPE_STRING,
                                STREAM_TYPE_ARG_UNUSED);
  MasterPrinter_reset_stream(mp, ofs);

  success = (MasterPrinter_print_node(mp, n) &&
             /* ensure proper flushing */
             MasterPrinter_flush_stream(mp));

  return (success)? strdup(MasterPrinter_get_streamed_string(mp)) : NULL;
}

int print_node_indent(MasterPrinter_ptr mp, FILE *stream, node_ptr n)
{
  return print_node_indent_at(mp, stream, n, 0);
}

char* sprint_node_indent(MasterPrinter_ptr mp, node_ptr n)
{
  return sprint_node_indent_at(mp, n, 0);
}


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

MasterPrinter_ptr MasterPrinter_create(const NuSMVEnv_ptr env)
{
  MasterPrinter_ptr self = ALLOC(MasterPrinter, 1);
  MASTER_PRINTER_CHECK_INSTANCE(self);

  master_printer_init(self, env);
  return self;
}

int MasterPrinter_print_node(MasterPrinter_ptr self, node_ptr n)
{
  MASTER_PRINTER_CHECK_INSTANCE(self);
  return master_printer_print_node(self, n, 0);
}

int MasterPrinter_print_string(MasterPrinter_ptr self, const char* str)
{
  int res = 1;    /* defaults to ok */
  char* s = strdup(str);
  char* p = s;  /* beginning, end pointers to the current chunk */
  char* q;

  MASTER_PRINTER_CHECK_INSTANCE(self);

  /* if there are newlines in the string to be printed, cut the string where
   * first newline occurs, then print the fragment followed by a newline and
   * a number of spaces according to internal indentation stack head. Repeat
   * until all fragments have been printed.
   **/

  do {
    boolean pad = false;

    if ((q=strchr(p, NEWLINE_CHR))) {
      /* side effects on p */
      (*q) = TERM_CHR; q++;
      pad = true;
    }

    /* original printout */
    switch (self->stream_type) {
    case STREAM_TYPE_DEFAULT:
    case STREAM_TYPE_STDOUT:
    case STREAM_TYPE_STDERR:
    case STREAM_TYPE_FILE:
      if (!master_printer_fprint(self, p)) {
        res = 0; /* failure */
        goto leave;
      }
      break;

    case STREAM_TYPE_STRING:
      if (!master_printer_sprint(self, p)) {
        res = 0; /* failure */
        goto leave;
      }
      break;

    case STREAM_TYPE_FUNCTION:
      {
        void* arg = self->stream_arg.function.argument;
        if (!self->stream_arg.function.func_ptr(p, arg)) {
          res = 0; /* failure */
          goto leave;
        }
        break;
      }

    default:
      {
        const NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(self));
        const ErrorMgr_ptr errmgr =
          ERROR_MGR(NuSMVEnv_get_value(env, ENV_ERROR_MANAGER));
        ErrorMgr_internal_error(errmgr, "MasterPrinter::print_string: Invalid stream type\n");
      }
    }

    /* update current ofs */
    self->current_ofs += strlen(p);

    /* post-processing padding */
    if (pad) {
      int i, padding = master_printer_get_level(self);

      /* add a newline and #padding spaces */
      for (i=0; i<=padding; i++) {
        char *tmp = (!i) ? NEWLINE_STR : BLANK_STR;

        switch (self->stream_type) {
        case STREAM_TYPE_DEFAULT:
        case STREAM_TYPE_STDOUT:
        case STREAM_TYPE_STDERR:
        case STREAM_TYPE_FILE:
          if (!master_printer_fprint(self, tmp)) {
            res = 0; /* failure */
            goto leave;
          }
          break;

        case STREAM_TYPE_STRING:
          if (!master_printer_sprint(self, tmp)) {
            res = 0; /* failure */
            goto leave;
          }
          break;

        case STREAM_TYPE_FUNCTION:
          {
            void* arg = self->stream_arg.function.argument;
            if (!self->stream_arg.function.func_ptr(tmp, arg)) {
              res = 0; /* failure */
              goto leave;
            }
            break;
          }

        default:
          {
            const NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(self));
            const ErrorMgr_ptr errmgr =
              ERROR_MGR(NuSMVEnv_get_value(env, ENV_ERROR_MANAGER));
            ErrorMgr_internal_error(errmgr, "MasterPrinter::print_string: Invalid stream type\n");
          }
        }
      }

      /* update current ofs */
      self->current_ofs = padding;
    }

    p=q;
  } while (p); /* is there anything left to print? */

 leave:
  FREE(s);
  return res; /* 1 if no failures occurred */
}

const char* MasterPrinter_get_streamed_string(const MasterPrinter_ptr self)
{
  MASTER_PRINTER_CHECK_INSTANCE(self);
  nusmv_assert(self->stream_type == STREAM_TYPE_STRING);

  return (const char*) (self->sstream);
}

void MasterPrinter_set_stream_type(MasterPrinter_ptr self,
                                   StreamType type,
                                   const union StreamTypeArg* arg)
{
  MASTER_PRINTER_CHECK_INSTANCE(self);

  {
    const NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(self));
    const ErrorMgr_ptr errmgr =
      ERROR_MGR(NuSMVEnv_get_value(env, ENV_ERROR_MANAGER));

    switch (type) {
    case STREAM_TYPE_DEFAULT:
    case STREAM_TYPE_STDOUT:
      self->stream_arg.file = StreamMgr_get_output_stream(self->streams);
      break;

    case STREAM_TYPE_STDERR:
      self->stream_arg.file = StreamMgr_get_error_stream(self->streams);
      break;

    case STREAM_TYPE_STRING:
      if (self->stream_type != STREAM_TYPE_STRING) {
        /* we start with a new string stream, but only if it is not
           currently set a string stream */
        master_printer_reset_string_stream(self);
      }
      break;

    case STREAM_TYPE_FILE:
      self->stream_arg.file = ((STREAM_TYPE_ARG_UNUSED != arg)?
                               arg->file : NULL);
      break;

    case STREAM_TYPE_FUNCTION:
      /*
        Copy the structure 'function' (Both function pointer and void* argument)
      */
      nusmv_assert(STREAM_TYPE_ARG_UNUSED != arg);
      self->stream_arg.function = arg->function;
      break;

    default:
      ErrorMgr_internal_error(errmgr, "MasterPrinter::set_stream_type: Invalid stream type\n");
    }

    self->stream_type = type;
  }
}

StreamType MasterPrinter_get_stream_type(const MasterPrinter_ptr self)
{
  MASTER_PRINTER_CHECK_INSTANCE(self);
  return self->stream_type;
}

void  MasterPrinter_reset_stream(MasterPrinter_ptr self, int initial_offset)
{
  ListIter_ptr p;
  MASTER_PRINTER_CHECK_INSTANCE(self);

  self->current_ofs = (0<initial_offset) ? initial_offset : 0;

  /* clear stack (should be empty anyway) */
  p = NodeList_get_first_iter(self->indent_stack);
  while (NULL != p) {
    NodeList_remove_elem_at(self->indent_stack, p);
    p = NodeList_get_first_iter(self->indent_stack);
  }

  /* stream dependant operations */
  switch (self->stream_type) {

  case STREAM_TYPE_DEFAULT:
  case STREAM_TYPE_STDOUT:
  case STREAM_TYPE_STDERR:
  case STREAM_TYPE_FILE:
  case STREAM_TYPE_FUNCTION:
    break;

  case STREAM_TYPE_STRING:
    master_printer_reset_string_stream(self);
    break;

  default:
    error_unreachable_code(); /* no other possible cases */
  }
}

void MasterPrinter_close_stream(MasterPrinter_ptr self)
{
  MASTER_PRINTER_CHECK_INSTANCE(self);

  switch (self->stream_type) {
  case STREAM_TYPE_DEFAULT:
  case STREAM_TYPE_STDOUT:
  case STREAM_TYPE_STDERR:
    break;

  case STREAM_TYPE_STRING:
    /* resets the string stream */
    master_printer_reset_string_stream(self);
    break;

  case STREAM_TYPE_FILE:
      /* closes the file stream if not nusmv_std{out,err} */
      if ((self->stream_arg.file != StreamMgr_get_output_stream(self->streams)) &&
          (self->stream_arg.file != StreamMgr_get_error_stream(self->streams))) {
        fclose(self->stream_arg.file);
      }
      break;

  case STREAM_TYPE_FUNCTION:
    break;

  default:
    error_unreachable_code(); /* no other possible cases */
  }

  /* sets the default stream type */
  MasterPrinter_set_stream_type(self, STREAM_TYPE_DEFAULT,
                                STREAM_TYPE_ARG_UNUSED);
}

int MasterPrinter_flush_stream(MasterPrinter_ptr self)
{
  MASTER_PRINTER_CHECK_INSTANCE(self);

  switch (self->stream_type) {
  case STREAM_TYPE_DEFAULT:
    /* not applicable here */
    break;

  case STREAM_TYPE_STDOUT:
    return !fflush(StreamMgr_get_output_stream(self->streams));
    break;

  case STREAM_TYPE_STDERR:
    return !fflush(StreamMgr_get_error_stream(self->streams));
    break;

  case STREAM_TYPE_STRING:
    /* not applicable here */
    break;

  case STREAM_TYPE_FILE:
    return !fflush(self->stream_arg.file);
    break;

  case STREAM_TYPE_FUNCTION:
    /* not applicable here */
    break;

  default:
    error_unreachable_code(); /* no other possible cases */
  }

  return 1;
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

int master_printer_print_node(MasterPrinter_ptr self,
                              node_ptr n, int priority)
{
  ListIter_ptr iter;
  iter = NodeList_get_first_iter(MASTER_NODE_WALKER(self)->walkers);
  while (!ListIter_is_end(iter)) {
    PrinterBase_ptr pr =
      PRINTER_BASE(NodeList_get_elem_at(MASTER_NODE_WALKER(self)->walkers,
                                        iter));

    if (NodeWalker_can_handle(NODE_WALKER(pr), n)) {

      return PrinterBase_print_node(pr, n, priority);
    }

    iter = ListIter_get_next(iter);
  }

  return 1;
}

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/*!
  \brief The MasterPrinter class private initializer

  The MasterPrinter class private initializer

  \sa MasterPrinter_create
*/
static void master_printer_init(MasterPrinter_ptr self, const NuSMVEnv_ptr env)
{
  const StreamMgr_ptr streams =
    STREAM_MGR(NuSMVEnv_get_value(env, ENV_STREAM_MANAGER));

  /* base class initialization */
  master_node_walker_init(MASTER_NODE_WALKER(self), env);

  /* allocates a minimal string stream */
  self->streams = streams;
  self->sstream_cap = 1;
  self->sstream_len = 0;
  self->sstream_grow_sum = 0;
  self->sstream_grow_num = 0;
  self->sstream = ALLOC(char, self->sstream_cap);
  self->sstream[0] = '\0';

  /* initialize internal structure for pretty printing */
  self->indent_stack = NodeList_create();
  self->current_ofs  = 0;

  /* default stream */
  MasterPrinter_set_stream_type(self,
                                STREAM_TYPE_DEFAULT,
                                STREAM_TYPE_ARG_UNUSED);

  /* virtual methods settings */
  OVERRIDE(Object, finalize) = master_printer_finalize;

}

/*!
  \brief The MasterPrinter class private deinitializer

  The MasterPrinter class private deinitializer

  \sa MasterPrinter_destroy
*/
static void master_printer_deinit(MasterPrinter_ptr self)
{
  NodeList_destroy(self->indent_stack);
  self->indent_stack = NODE_LIST(NULL);
  FREE(self->sstream);
  self->sstream = (char*) NULL;

  /* base class deinitialization */
  master_node_walker_deinit(MASTER_NODE_WALKER(self));
}

/*!
  \brief The PrinterBase class virtual finalizer

  Called by the class destructor
*/
static void master_printer_finalize(Object_ptr object, void* dummy)
{
  MasterPrinter_ptr self = MASTER_PRINTER(object);

  master_printer_deinit(self);
  FREE(self);
}

/*!
  \brief Appends a string to the internal string stream

  Warning: current stream type must be STREAM_TYPE_STRING
*/
static int master_printer_sprint(MasterPrinter_ptr self, const char* str)
{
  int len;

  nusmv_assert(self->stream_type == STREAM_TYPE_STRING);

  /* ignore empty strings */
  len = strlen(str);
  if (0 != len) {
    self->sstream_len += len;

    if (self->sstream_len >= self->sstream_cap) {
      /* resizes the sstream */
      self->sstream_grow_sum += self->sstream_len;
      self->sstream_grow_num += 1;
      self->sstream_cap = self->sstream_len + 1 +
        (self->sstream_grow_sum / self->sstream_grow_num);

      self->sstream = (char*) REALLOC(char, self->sstream, self->sstream_cap);
      nusmv_assert(self->sstream != (char*) NULL);
    }

    strcat(self->sstream, str);
  }

  return 1;
}

/*!
  \brief Stream the given string to the internally set file
  stream

  Warning: current stream type must be compatible
*/
static int master_printer_fprint(MasterPrinter_ptr self, const char* str)
{
  nusmv_assert( (self->stream_type == STREAM_TYPE_DEFAULT) ||
                (self->stream_type == STREAM_TYPE_STDOUT) ||
                (self->stream_type == STREAM_TYPE_STDERR) ||
                (self->stream_type == STREAM_TYPE_FILE));

  if (str[0] != '\0') {
    nusmv_assert(NULL != self->stream_arg.file);

    return fprintf(self->stream_arg.file, "%s", str);
  }

  else return 1;
}

int master_printer_indent(MasterPrinter_ptr self)
{
  MASTER_PRINTER_CHECK_INSTANCE(self);

  NodeList_prepend(self->indent_stack,
                   NODE_FROM_INT (self->current_ofs));

  /* never fails */
  return 1;
}

int master_printer_deindent(MasterPrinter_ptr self)
{
  ListIter_ptr head;
  MASTER_PRINTER_CHECK_INSTANCE(self);

  head = NodeList_get_first_iter(self->indent_stack);
  if (NULL == head) {
    const NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(self));
    const ErrorMgr_ptr errmgr =
      ERROR_MGR(NuSMVEnv_get_value(env, ENV_ERROR_MANAGER));

    ErrorMgr_internal_error(errmgr, "printout failure: empty indentation stack!");
  }

  NodeList_remove_elem_at(self->indent_stack, head);

  /* never fails */
  return 1;

}

/*!
  \brief Get current level of indentation

  Returns an integer representing the current level of
  indentation (i.e. the number of whitespaces to pad the string
  with). If the internal stack is empty, assume indentation level is 0
  for backward compatibility.
*/

static int
master_printer_get_level(MasterPrinter_ptr self)
{
  ListIter_ptr head;

  MASTER_PRINTER_CHECK_INSTANCE(self);

  head = NodeList_get_first_iter(self->indent_stack);
  return (NULL != head)
    ? NODE_TO_INT(NodeList_get_elem_at(self->indent_stack, head))
    : 0 ;
}

/*!
  \brief Cleans up the stream that have been read so far.
  Any previoulsy read stream will be lost



  \sa get_streamed_string
*/

static void master_printer_reset_string_stream(MasterPrinter_ptr self)
{
  MASTER_PRINTER_CHECK_INSTANCE(self);

  self->sstream[0] = '\0';
  self->sstream_len = 0;
}

/**AutomaticEnd***************************************************************/
