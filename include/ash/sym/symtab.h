#ifndef ASH_SYM_SYMTAB_H_
#define ASH_SYM_SYMTAB_H_

#include "sym.h"

#include <type_traits>
#include <string_view>
#include <unordered_set>
#include <unordered_map>
#include <vector>

namespace ash::sym {

class SymbolTable {
  template<class T>
  static void call_dtor(void *p) noexcept(std::is_nothrow_destructible_v<T>) {
    static_cast<T*>(p)->~T();
  }

public:
  struct InternData {
    uint32_t size() const noexcept {
      return _size;
    }

    const void *data() const noexcept {
      return reinterpret_cast<const char *>(this) + sizeof(uint32_t);
    }

    void *data() noexcept {
      return reinterpret_cast<char *>(this) + sizeof(uint32_t);
    }

    operator std::string_view() const noexcept {
      return {static_cast<const char *>(data()), static_cast<size_t>(_size)};
    }

  private:
    uint32_t _size;
    // char data[size];
  };

  typedef const InternData *idata;

  explicit SymbolTable() {}
  ~SymbolTable();

  id_t next_id() noexcept {
    return _next_id++;
  }

  void *alloc(size_t size);

  template<class T>
  T *alloc(size_t count = 1) {
    return static_cast<T *>(alloc(count * sizeof(T)));
  }

  template<class T, class... Args>
  T *cxxnew(Args &&...args) {
    T *p = alloc<T>();
    new (p) T(std::forward<Args>(args)...);
    if constexpr (!std::is_trivially_destructible_v<T>) {
      _dtor_list.push_back(std::make_pair(
        static_cast<void*>(p),
        &SymbolTable::template call_dtor<T>
      ));
    }
    return p;
  }

  //idata find_str(std::string_view str, bool insert);
  //idata find_str(std::string_view str) const noexcept;

  bool find_str(std::string_view &str, bool insert);

  bool find_str(std::string_view &str) const noexcept {
    return const_cast<SymbolTable *>(this)->find_str(str, false);
  }

  void *get_attr(id_t symid, id_t attrid) noexcept;

  const void *get_attr(id_t symid, id_t attrid) const noexcept {
    return const_cast<SymbolTable*>(this)->get_attr(symid, attrid);
  }

  template<class T>
  T *get_attr(id_t symid) noexcept {
    return static_cast<T *>(get_attr(symid, attr::type_id_v<T>));
  }

  template<class T>
  const T *get_attr(id_t symid) const noexcept {
    return static_cast<T *>(get_attr(symid, attr::type_id_v<T>));
  }

  // Returns [pointer to data, inserted].
  std::pair<void *, bool> put_attr(id_t symid, id_t attrid, size_t size);

  template<class T, id_t AttrId = attr::type_id_v<T>, size_t Size = sizeof(T)>
  std::pair<T *, bool> put_attr(id_t symid) {
    auto pair = put_attr(symid, AttrId, Size);
    return std::make_pair(static_cast<T *>(pair.first), pair.second);
  }

private:
  id_t _next_id = 0;
  std::vector<std::pair<void *, void (*)(void *) noexcept>> _dtor_list;
  std::vector<void *> _free_list;
  std::unordered_set<std::string_view> _strings;
  std::unordered_map<uint64_t, void *> _attr_map;
};

} // namespace ash::sym

#endif /* ASH_SYM_SYMTAB_H_ */
