#include "ash/sym/symtab.h"
#include <cstdlib>
#include <cstring>

using namespace ash::sym;

SymbolTable::~SymbolTable() {
  for (auto it = _dtor_list.rbegin(); it != _dtor_list.rend(); ++it) {
    it->second(it->first);
  }
  for (auto it = _free_list.rbegin(); it != _free_list.rend(); ++it) {
    free(*it);
  }
}

void *SymbolTable::alloc(size_t size) {
  void *p = malloc(size);
  _free_list.push_back(p);
  return p;
}

bool SymbolTable::find_str(std::string_view &str, bool insert) {
  auto it = _strings.find(str);
  if (it != _strings.end()) {
    str = *it;
    return true;
  }

  if (insert) {
    char *data = static_cast<char *>(alloc(str.size()));
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
    it->second = alloc(size);
  }
  return std::make_pair(it->second, inserted);
}