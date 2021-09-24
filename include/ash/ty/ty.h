#ifndef TY_H_INCLUDED
#define TY_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

#ifndef TY_TARGET_CODE_TYPE
#define TY_TARGET_CODE_TYPE size_t
#endif
// Target code pointer.
typedef TY_TARGET_CODE_TYPE pcode_t;

#ifndef TY_TARGET_DATA_TYPE
#define TY_TARGET_DATA_TYPE size_t
#endif
// Target data pointer.
typedef TY_TARGET_DATA_TYPE pdata_t;

#ifndef TY_SYMBOL_TYPE
#define TY_SYMBOL_TYPE TY_TARGET_DATA_TYPE
#endif
// Symbol index.
typedef TY_SYMBOL_TYPE sid_t;

// T is a variable length struct with a char[1] field at the end.
// V is the actual type at the end of T.
// Returns the offset of V within T.
#define TY_VLS_OFS(V, T) (((sizeof(T) - 1) + (alignof(V) - 1)) & ~(alignof(V) - 1))

// V is the type after T in the variable length struct.
// T is the type at offset O.
// O is the current offset or total size.
// Returns the next aligned place for V after O.
#define TY_NEXT_OFS(V, T, O) (((O) + sizeof(T) + (alignof(V) - 1)) & ~(alignof(V) - 1))

// V is the type of the returned pointer.
// P is a pointer to a data block of size > O.
// O is the offset in bytes to add to P.
// Returns P + O in bytes, casted to V*.
#define TY_OFS_PTR(V, P, O) ((V *)((char *)(P) + (O)))

// Same as TY_OFS_PTR but returns a const V*.
#define TY_OFS_CPTR(V, P, O) ((const V *)((const char *)(P) + (O)))

// V is the type at the end of T.
// T is a variable length struct.
// P is a pointer to T.
// Returns a V* in *P. Same as TY_OFS_PTR(V, P, TY_VLS_OFS(V, T)).
#define TY_VLS_PTR(V, T, P) ((V *)((char *)(P) + TY_VLS_OFS(V, T)))

// Same as TY_VLS_PTR but returns a const V*.
#define TY_VLS_CPTR(V, T, P) ((const V *)((const char *)(P) + TY_VLS_OFS(V, T)))

typedef struct _attr {
  sid_t sid;
  struct _attr *next;
} _attr_t;

typedef struct attr {
  sid_t sid;
  struct attr *next;
  char data[1];
} attr_t;

typedef struct symbol {
  sid_t sid;
  attr_t *attr;
} symbol_t;

typedef struct aname {
  const char *name;
  size_t length;
} aname_t;

typedef uint16_t data_attr_t;
typedef struct adata {
  size_t size;
  data_attr_t alignp2 : 5; // allows up to 2GB alignment, which is... a lot.
  data_attr_t memref  : 1; // may contain pointers to memory
  data_attr_t selfref : 1; // may contain pointers to itself
  data_attr_t canread : 1; // memory is readable
  data_attr_t canwrite : 1; // memory is writable
  data_attr_t canexec : 1; // memory is executable
  data_attr_t minsize : 1; // size is minimum but may not be absolute
} adata_t;

typedef struct ascope {
  void *byname;
} ascope_t;

symbol_t ty_symbol_root();
symbol_t *ty_symbol_new(symbol_t *root);
symbol_t *ty_symbol_new_with_attrs(symbol_t *root, symbol_t *symlist);
symbol_t *ty_symbol_new_with_attrs_by_id(symbol_t *root, sid_t *sidlist);
attr_t *ty_symbol_get_attr(symbol_t *sym, sid_t attrsid);

attr_t *ty_attr_new(sid_t sid, size_t size);
attr_t *ty_attr_new_list(symbol_t *root, symbol_t *symlist);
attr_t *ty_attr_new_list_by_id(symbol_t *root, sid_t *sidlist);
#define ty_attr_get_typed_data(T, A) ((T *)((A)->data))

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* TY_H_INCLUDED */
