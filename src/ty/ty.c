#include "ash/ty/ty.h"

#include <stdbool.h>
#include <stdalign.h>
#include <string.h>
#include <stdlib.h>
//#include <malloc.h>

enum {
  sid_root = 0,
  sid_attr,
  sid_aname,
  sid_adata,
};

symbol_t ty_symbol_root() {
  size_t oname = TY_VLS_OFS(aname_t, attr_t);
  size_t oscope = TY_NEXT_OFS(ascope_t, aname_t, oname);
  size_t total = oscope + sizeof(ascope_t);
  symbol_t root = {
    .sid = 0,
    .attr = malloc(total),
  };
  return root;
}

symbol_t *ty_symbol_new(symbol_t *root) {
  return NULL;
}

symbol_t *ty_symbol_new_with_attrs(symbol_t *root, symbol_t *symlist) {
  return NULL;
}

symbol_t *ty_symbol_new_with_attrs_by_id(symbol_t *root, sid_t *sidlist) {
  return NULL;
}

attr_t *ty_symbol_get_attr(symbol_t *sym, sid_t attrsid) {
  attr_t *a = sym->attr;
  while (a) {
    if (a->sid == attrsid) {
      return a;
    }
    a = a->next;
  }
  return NULL;
}

attr_t *ty_attr_new(sid_t sid, size_t size) {
  return NULL;
}

attr_t *ty_attr_new_list(symbol_t *root, symbol_t *symlist) {
  return NULL;
}

attr_t *ty_attr_new_list_by_id(symbol_t *root, sid_t *sidlist) {
  return NULL;
}
