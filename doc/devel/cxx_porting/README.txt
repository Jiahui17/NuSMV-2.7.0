Code
====
The code in SymbTable_.{h,cc} and SymbCache_.{h,cc} contains
explanatory code used to experiment the porting. However the porting
is partial, as the activity was stopped to do more urgent tasks.

Containers Structures
=====================

char* --> std::string
NodeList --> std::vector
LRUCache --> rewrite/adapt in c++
OAHash --> rewrite/adapt in c++
Pair --> std::pair
Triple --> std::tuple      [!!c++11]
Tuple5 --> std::tuple      [!!c++11]
array --> std::vector
assoc --> std::map         [check ordering!]
list --> ---               [DEPRECATED]
stack --> std::stack
set --> std::set           [check ordering!]
heap --> std::priority_queue
avl --> rewrite/adapt in c++
sset --> implement based on std::set
Slist --> std::vector (or std::forward_list, TBD)
Olist --> std::vector (or std::list, TBD)
BiMap --> rewrite in c++
DLlist --> std::list
NodeGraph --> adapt in c++
boolean --> bool


Services
========

Logging --> specific library TBD (e.g. google logging c++ lib)


Problems
========

- void* --> enumeration type (enum = (T) (size_t) p)
- c++ keywords (e.g. 'new')
- when partially porting: leave C names in outscope namespace until
  all enters in c++


Practices
=========

- typedef struct <Name_TAG> {...} Name ---->  struct Name {...};
- Use references for objects, and pointers only when there's no other option
- type ClassName_method_name([const] ClassName_ptr self, ...) --> type
  method_name(...) [const]
- use <algorithm> when applicable, not loops
- never use "using namespace std"
- documentation in header files for methods and functions
  /** Title
    * Details */
