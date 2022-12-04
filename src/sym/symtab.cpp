#include "lava/sym/symtab.h"
#include <cstring>

using namespace lava::sym;

Symtab::Symtab() {
  lava_arena_init(&_arena);
  fill_initial_symbols();
}

Symtab::~Symtab() {
  lava_arena_fini(&_arena);
}

void Symtab::fill_initial_symbols() {
  auto void_s = intern_str("void");
  auto null_s = intern_str("null");

  static_assert(ID_root == 0);
  _symbol_data.emplace_back(ID_root, ID_undefined, CRef{});

  static_assert(ID_void == 1);
  _symbol_data.emplace_back(ID_void, ID_root, void_s);

  static_assert(ID_null == 2);
  _symbol_data.emplace_back(ID_null, ID_root, null_s);

  get_or_put_attr<ScopeMap>(ID_root, ID_ScopeMap).first->emplace(
    null_s, std::make_pair(ID_null, 0)
  );
}

CRef Symtab::find_data(const void *p, size_t size) const noexcept {
  std::string_view sv{static_cast<const char*>(p), size};
  auto it = _intern_data.find(sv);
  if (it == _intern_data.end()) {
    return {};
  }
  return {it->size(), it->data()};
}

CRef Symtab::intern_data(const void *p, size_t align, size_t size) {
  std::string_view sv{static_cast<const char*>(p), size};
  auto [it, inserted] = _intern_data.insert(sv);
  if (inserted) {
    char *pc = static_cast<char*>(
      lava_arena_alloc(&_arena, align, sv.size()));
    memcpy(pc, p, size);
    const_cast<std::string_view&>(*it) = std::string_view{pc, size};
    return {size, pc};
  }
  return {it->size(), it->data()};
}

CRef Symtab::find_str(std::string_view str) const noexcept {
  return find_data(str.data(), str.size());
}

CRef Symtab::intern_str(std::string_view str) {
  return intern_data(str.data(), 1, str.size());
}

uint32_t Symtab::id(uint32_t parent, CRef name) const noexcept {
  if (parent == ID_undefined && !name) {
    return ID_root;
  }

  auto *sm = this->get_attr<ScopeMap>(parent, ID_ScopeMap);
  if (!sm) {
    return ID_undefined;
  }
  auto it = sm->find(name);
  if (it == sm->end()) {
    return ID_undefined;
  }
  return it->second.first;
}

uint32_t Symtab::id(uint32_t parent, std::string_view name) const noexcept {
  return id(parent, find_str(name));
}

uint32_t Symtab::parent(uint32_t id) const noexcept {
  return _symbol_data[id].parent;
}

uint32_t Symtab::mksym(uint32_t parent, CRef name) {
  auto id = static_cast<uint32_t>(_symbol_data.size());
  auto sm = get_attr<ScopeMap>(parent, ID_ScopeMap);
  if (sm == nullptr) {
    return ID_undefined;
  }
  uint32_t childnum = static_cast<uint32_t>(sm->size());
  _symbol_data.emplace_back(SymbolData{id, parent, name});
  sm->emplace(name, std::make_pair(id, childnum));
  return id;
}

void *Symtab::get_attr(uint32_t id, uint32_t attr_id) noexcept {
  auto &sym = _symbol_data[id];
  for (auto &a : sym.attrs) {
    if (a.first == attr_id) return a.second;
  }
  return nullptr;
}

std::pair<void *, bool>
Symtab::get_or_put_attr(uint32_t id, uint32_t attr_id) {
  if (void *p = get_attr(id, attr_id)) return {p, false};
  size_t align = 0, size;
  for (auto &a : _symbol_data[attr_id].attrs) {
    if (a.first == ID_MemInfo32) {
      align = static_cast<MemInfo32*>(a.second)->align();
      size = static_cast<MemInfo32*>(a.second)->size;
    } else if (a.first == ID_MemInfo64) {
      align = static_cast<MemInfo64*>(a.second)->align();
      size = static_cast<MemInfo64*>(a.second)->size;
    }
  }
  if (!align) return {nullptr, false};
  void *data = lava_arena_alloc(&_arena, align, size);
  if (!data) return {nullptr, false};
  _symbol_data[id].attrs.emplace_back(attr_id, data);
  return {data, true};
}
