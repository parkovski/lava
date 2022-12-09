#include "lava/sym/symtab.h"

#include <cstring>

using namespace lava::sym;

Symtab::Symtab()
  : _symbols{lava::data::arena_allocator<Symbol>{&_arena}}
{
  lava_arena_init(&_arena);
  add_initial_symbols();
}

Symtab::~Symtab() {
  lava_arena_fini(&_arena);
}

void Symtab::add_initial_symbols() {
  constexpr uint32_t align_native = sizeof(void*) == 8 ? 3 : 2;

  auto &s_root = _symbols.emplace_back(ID_root, ID_undefined, intern("(root)"));
  auto *root_scopemap = static_cast<ScopeMap*>(
    lava_arena_alloc(&_arena, alignof(ScopeMap), sizeof(ScopeMap)));
  new (root_scopemap) ScopeMap;
  s_root.attrs.emplace_back(ID_ScopeMap, root_scopemap);

  auto &s_never = _symbols.emplace_back(ID_never, ID_root, intern("never"));

  auto &s_void = _symbols.emplace_back(ID_void, ID_root, intern("void"));
  auto *mi = static_cast<MemInfo32*>(s_void.attrs.emplace_back(
    ID_MemInfo32,
    lava_arena_alloc(&_arena, alignof(MemInfo32), sizeof(MemInfo32))
  ).second);
  mi->_size = 0;
  mi->_alignshift = 0;

  auto &s_null = _symbols.emplace_back(ID_null, ID_root, intern("null"));
  mi = static_cast<MemInfo32*>(s_null.attrs.emplace_back(
    ID_MemInfo32,
    lava_arena_alloc(&_arena, alignof(MemInfo32), sizeof(MemInfo32))
  ).second);
  mi->_size = sizeof(void*);
  mi->_alignshift = align_native;

  auto &s_MemInfo32 = _symbols.emplace_back(
    ID_MemInfo32, ID_root, intern("MemInfo32"));
  mi = static_cast<MemInfo32*>(s_MemInfo32.attrs.emplace_back(
    ID_MemInfo32,
    lava_arena_alloc(&_arena, alignof(MemInfo32), sizeof(MemInfo32))
  ).second);
  mi->_size = sizeof(MemInfo32);
  mi->_alignshift = 2;

  auto &s_MemInfo64 = _symbols.emplace_back(
    ID_MemInfo64, ID_root, intern("MemInfo64"));
  mi = static_cast<MemInfo32*>(s_MemInfo64.attrs.emplace_back(
    ID_MemInfo32,
    lava_arena_alloc(&_arena, alignof(MemInfo32), sizeof(MemInfo32))
  ).second);
  mi->_size = sizeof(MemInfo64);
  mi->_alignshift = 3;

  auto &s_ScopeMap = _symbols.emplace_back(
    ID_ScopeMap, ID_root, intern("ScopeMap"));
  mi = static_cast<MemInfo32*>(s_ScopeMap.attrs.emplace_back(
    ID_MemInfo32,
    lava_arena_alloc(&_arena, alignof(MemInfo32), sizeof(MemInfo32))
  ).second);
  mi->_size = sizeof(ScopeMap);
  mi->_alignshift = align_native;

  *reinterpret_cast<void(**)(void*)>(s_ScopeMap.attrs.emplace_back(
    ID_init,
    lava_arena_alloc(&_arena, alignof(void(*)(void*)), sizeof(void(*)(void*)))
  ).second) = [](void *data) {
    new (data) ScopeMap;
  };

  *reinterpret_cast<void(**)(void*)>(s_ScopeMap.attrs.emplace_back(
    ID_fini,
    lava_arena_alloc(&_arena, alignof(void(*)(void*)), sizeof(void(*)(void*)))
  ).second) = [](void *data) {
    static_cast<ScopeMap*>(data)->~ScopeMap();
  };

  auto &s_Symbol = _symbols.emplace_back(
    ID_Symbol, ID_root, intern("Symbol"));
  mi = static_cast<MemInfo32*>(s_Symbol.attrs.emplace_back(
    ID_MemInfo32,
    lava_arena_alloc(&_arena, alignof(MemInfo32), sizeof(MemInfo32))
  ).second);
  mi->_size = sizeof(Symbol);
  mi->_alignshift = align_native;

  *reinterpret_cast<void(**)(void*)>(s_Symbol.attrs.emplace_back(
    ID_init,
    lava_arena_alloc(&_arena, alignof(void(*)(void*)), sizeof(void(*)(void*)))
  ).second) = [](void *data) {
    new (data) Symbol;
  };

  *reinterpret_cast<void(**)(void*)>(s_Symbol.attrs.emplace_back(
    ID_fini,
    lava_arena_alloc(&_arena, alignof(void(*)(void*)), sizeof(void(*)(void*)))
  ).second) = [](void *data) {
    static_cast<Symbol*>(data)->~Symbol();
  };

  auto &s_prototype = _symbols.emplace_back(
    ID_prototype, ID_root, intern("prototype"));
  s_prototype.metas.emplace_back(ID_Symbol);

  auto &s_GenericFn = _symbols.emplace_back(
    ID_GenericFn, ID_root, intern("GenericFn"));
  mi = static_cast<MemInfo32*>(s_GenericFn.attrs.emplace_back(
    ID_MemInfo32,
    lava_arena_alloc(&_arena, alignof(MemInfo32), sizeof(MemInfo32))
  ).second);
  mi->_size = sizeof(GenericFn);
  mi->_alignshift = align_native;

  auto &s_ContextFn = _symbols.emplace_back(
    ID_ContextFn, ID_root, intern("ContextFn"));
  mi = static_cast<MemInfo32*>(s_ContextFn.attrs.emplace_back(
    ID_MemInfo32,
    lava_arena_alloc(&_arena, alignof(MemInfo32), sizeof(MemInfo32))
  ).second);
  mi->_size = sizeof(ContextFn);
  mi->_alignshift = align_native;

  auto &s_init = _symbols.emplace_back(
    ID_init, ID_root, intern("init"));
  s_init.metas.emplace_back(ID_GenericFn);

  auto &s_fini = _symbols.emplace_back(
    ID_fini, ID_root, intern("fini"));
  s_fini.metas.emplace_back(ID_GenericFn);

  root_scopemap->emplace(std::make_pair(s_never.name, std::make_pair(ID_never, 0)));
  root_scopemap->emplace(std::make_pair(s_void.name, std::make_pair(ID_void, 1)));
  root_scopemap->emplace(std::make_pair(s_null.name, std::make_pair(ID_null, 2)));
  root_scopemap->emplace(std::make_pair(s_MemInfo32.name, std::make_pair(ID_MemInfo32, 3)));
  root_scopemap->emplace(std::make_pair(s_MemInfo64.name, std::make_pair(ID_MemInfo64, 4)));
  root_scopemap->emplace(std::make_pair(s_ScopeMap.name, std::make_pair(ID_ScopeMap, 5)));
  root_scopemap->emplace(std::make_pair(s_Symbol.name, std::make_pair(ID_Symbol, 6)));
  root_scopemap->emplace(std::make_pair(s_prototype.name, std::make_pair(ID_prototype, 7)));
  root_scopemap->emplace(std::make_pair(s_GenericFn.name, std::make_pair(ID_GenericFn, 8)));
  root_scopemap->emplace(std::make_pair(s_ContextFn.name, std::make_pair(ID_ContextFn, 9)));
  root_scopemap->emplace(std::make_pair(s_init.name, std::make_pair(ID_init, 10)));
  root_scopemap->emplace(std::make_pair(s_fini.name, std::make_pair(ID_fini, 11)));
}

InternRef Symtab::find_interned(const void *p, size_t size) const noexcept {
  std::string_view sv{static_cast<const char*>(p), size};
  auto it = _intern_data.find(sv);
  if (it == _intern_data.end()) {
    return InternRef{};
  }
  return InternRef{it->size(), it->data()};
}

InternRef Symtab::intern(const void *p, size_t align, size_t size) {
  std::string_view sv{static_cast<const char*>(p), size};
  auto [it, inserted] = _intern_data.insert(sv);
  if (inserted) {
    char *pc = static_cast<char*>(
      lava_arena_alloc(&_arena, align, sv.size()));
    memcpy(pc, p, size);
    const_cast<std::string_view&>(*it) = std::string_view{pc, size};
    return InternRef{size, pc};
  }
  return InternRef{it->size(), it->data()};
}

std::pair<uint32_t, bool>
Symtab::add_symbol(uint32_t parent, InternRef name) {
  auto *map = get_own_attr<ScopeMap>(parent, ID_ScopeMap);
  if (!map) {
    return std::make_pair(ID_undefined, false);
  }
  auto id = static_cast<uint32_t>(_symbols.size());
  auto index = static_cast<uint32_t>(map->size());
  auto [it, inserted] = map->emplace(name, std::make_pair(id, index));
  if (inserted) {
    _symbols.emplace_back(id, parent, name);
  } else {
    id = it->second.first;
  }
  return std::make_pair(id, inserted);
}

bool Symtab::add_meta(uint32_t symid, uint32_t metaid) {
  if (get_meta_index(symid, metaid) != ID_undefined) {
    return false;
  }
  _symbols[symid].metas.emplace_back(metaid);
  return true;
}

uint32_t
Symtab::get_meta_index(uint32_t symid, uint32_t metaid) const noexcept {
  auto const &metas = _symbols[symid].metas;
  for (uint32_t i = 0; i < metas.size(); ++i) {
    if (metas[i] == metaid) {
      return i;
    }
  }
  return ID_undefined;
}

void *Symtab::add_attr(uint32_t symid, uint32_t attrid) {
  if (get_own_attr(symid, attrid)) {
    return nullptr;
  }

  size_t align = 0;
  size_t size;
  void (*init)(void*) = nullptr;
  enum_attrs(attrid, [&](uint32_t id, void *attr) -> bool {
    if (id == ID_MemInfo32) {
      auto &mi = *static_cast<MemInfo32*>(attr);
      align = mi.align();
      size = mi.size();
      if (init) return false;
    } else if (id == ID_MemInfo64) {
      auto &mi = *static_cast<MemInfo64*>(attr);
      align = static_cast<size_t>(mi.align());
      size = static_cast<size_t>(mi.size());
      if (init) return false;
    } else if (id == ID_init) {
      init = *reinterpret_cast<void(**)(void*)>(attr);
      if (align) return false;
    }
    return true;
  });

  if (align == 0) {
    return nullptr;
  }
  void *mem = lava_arena_alloc(&_arena, align, size);
  if (init) {
    init(mem);
  }
  _symbols[symid].attrs.emplace_back(attrid, mem);
  return mem;
}

uint32_t
Symtab::get_own_attr_index(uint32_t symid, uint32_t attrid) const noexcept {
  auto const &attrs = _symbols[symid].attrs;
  for (uint32_t i = 0; i < attrs.size(); ++i) {
    if (attrs[i].first == attrid) {
      return i;
    }
  }
  return ID_undefined;
}

void *Symtab::get_own_attr(uint32_t symid, uint32_t attrid) noexcept {
  auto &sym = _symbols[symid];
  for (auto const &attr : sym.attrs) {
    if (attr.first == attrid) {
      return attr.second;
    }
  }
  return nullptr;
}

std::pair<uint32_t, void *>
Symtab::get_rec_attr(uint32_t symid, uint32_t attrid) noexcept {
  if (auto a = get_own_attr(symid, attrid)) {
    return std::pair<uint32_t, void*>{symid, a};
  }
  for (auto metaid : _symbols[symid].metas) {
    if (auto pair = get_rec_attr(metaid, attrid); pair.second) {
      return pair;
    }
  }
  return std::pair<uint32_t, void*>{ID_undefined, nullptr};
}

uint32_t Symtab::get_parent(uint32_t symid) const noexcept {
  return _symbols[symid].parent;
}

std::pair<uint32_t, uint32_t>
Symtab::get_own_child_index(uint32_t parent, InternRef name) const noexcept {
  auto *map = get_own_attr<ScopeMap>(parent, ID_ScopeMap);
  if (!map) {
    return std::pair<uint32_t, uint32_t>{ID_undefined, ID_undefined};
  }
  auto it = map->find(name);
  if (it == map->end()) {
    return std::pair<uint32_t, uint32_t>{ID_undefined, ID_undefined};
  }
  return it->second;
}

std::tuple<uint32_t, uint32_t, uint32_t>
Symtab::get_rec_child(uint32_t parent, InternRef name) const noexcept {
  auto [ownerid, map] = get_rec_attr<ScopeMap>(parent, ID_ScopeMap);
  if (!map) {
    return {ID_undefined, ID_undefined, ID_undefined};
  }
  auto it = map->find(name);
  if (it == map->end()) {
    return {ownerid, ID_undefined, ID_undefined};
  }
  return {ownerid, it->second.first, it->second.second};
}
