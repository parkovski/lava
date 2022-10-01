#include "ash/sym/symtab.h"
#include <cstddef>
#include <cstdlib>
#include <cstring>

using namespace ash::sym;

SymbolTable::~SymbolTable() {
  for (auto it = _dtor_list.rbegin(); it != _dtor_list.rend(); ++it) {
    if (it->second) it->second(it->first);
    free(it->first);
  }
}

void *SymbolTable::alloc(size_t align, size_t size,
                         void (*dtor)(void *) noexcept) {
  if (align == 0) align = alignof(max_align_t);
  void *p = aligned_alloc(align, size);
  _dtor_list.emplace_back(p, dtor);
  return p;
}

bool SymbolTable::find_str(std::string_view &str, bool insert) {
  auto it = _strings.find(str);
  if (it != _strings.end()) {
    str = *it;
    return true;
  }

  if (insert) {
    char *data = static_cast<char *>(alloc(1, str.size()));
    memcpy(data, str.data(), str.size());
    str = std::string_view{data, str.size()};
    _strings.insert(str);
    return true;
  }

  return false;
}

void *SymbolTable::get_attr(id_t symid, id_t attrid) noexcept {
  uint64_t key = (static_cast<uint64_t>(symid) << 32) | attrid;
  auto it = _attr_map.find(key);
  if (it == _attr_map.end()) {
    return nullptr;
  }
  return it->second;
}

std::pair<void *, bool>
SymbolTable::put_attr(id_t symid, id_t attrid, size_t size) {
  uint64_t key = (static_cast<uint64_t>(symid) << 32)
                | static_cast<uint64_t>(attrid);
  auto [it, inserted] = _attr_map.insert(std::make_pair(key, nullptr));
  if (inserted) {
    it->second = alloc(0, size);
  }
  return std::make_pair(it->second, inserted);
}
