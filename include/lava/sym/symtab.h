#ifndef LAVA_SYM_SYMTAB_H_
#define LAVA_SYM_SYMTAB_H_

#include "core.h"
#include "lava/data/arena.h"

#include <cstdint>
#include <type_traits>
#include <utility>
#include <string_view>
#include <unordered_set>
#include <tuple>

namespace lava::sym {

struct Symtab {
  Symtab();
  ~Symtab();

  void add_initial_symbols();

  // Find an interned data block.
  InternRef find_interned(const void *p, size_t size) const noexcept;

  // Intern a chunk of data.
  InternRef intern(const void *p, size_t align, size_t size);

  // Find an interned string (equivalent to
  // `find_data(str.data(), str.size())`).
  InternRef find_interned(std::string_view str) const noexcept
  { return find_interned(str.data(), str.size()); }

  // Intern a string (equivalent to `intern_data(str.data(), 1, str.size())`).
  InternRef intern(std::string_view str)
  { return intern(str.data(), 1, str.size()); }

  /// Insert a new symbol under `parent` with a unique name. `parent` must have
  /// a `ScopeMap` attribute.
  /// @returns A pair containing the ID of the new symbol, or `ID_undefined` if
  /// the symbol could not be inserted, and a flag indicating whether the
  /// symbol was newly inserted (true) or already existed (false).
  std::pair<uint32_t, bool> add_symbol(uint32_t parent, InternRef name);

  std::pair<uint32_t, bool> add_symbol(uint32_t parent, std::string_view name)
  { return add_symbol(parent, intern(name)); }

  Symbol &operator[](uint32_t id) noexcept
  { return _symbols[id]; }

  const Symbol &operator[](uint32_t id) const noexcept
  { return _symbols[id]; }

  // Add a symbol to the meta list for `symid`.
  bool add_meta(uint32_t symid, uint32_t metaid);

  // Gets the number of metasymbols defined for `symid`.
  uint32_t get_meta_count(uint32_t symid) const noexcept
  { return static_cast<uint32_t>(_symbols[symid].metas.size()); }

  // Gets the ID of the metasymbol for `symid` at `index`.
  uint32_t get_meta_at(uint32_t symid, uint32_t index) const noexcept
  { return _symbols[symid].metas[index]; }

  // Finds the index of `metaid` in `symid`, or `ID_undefined` if not present.
  uint32_t get_meta_index(uint32_t symid, uint32_t metaid) const noexcept;

  // Add an instance to a symbol's attribute list.
  void *add_attr(uint32_t symid, uint32_t attrid);

  template<class T>
  T *add_attr(uint32_t symid, uint32_t attrid)
  { return static_cast<T*>(add_attr(symid, attrid)); }

  uint32_t get_own_attr_count(uint32_t symid) const noexcept
  { return static_cast<uint32_t>(_symbols[symid].attrs.size()); }

  std::pair<uint32_t, void *>
  get_own_attr_at(uint32_t symid, uint32_t index) const noexcept
  { return _symbols[symid].attrs[index]; }

  uint32_t get_own_attr_index(uint32_t symid, uint32_t attrid) const noexcept;

  // Looks for an attribute in the symbol's own attribute list.
  void *get_own_attr(uint32_t symid, uint32_t attrid) noexcept;

  const void *get_own_attr(uint32_t symid, uint32_t attrid) const noexcept
  { return const_cast<Symtab*>(this)->get_own_attr(symid, attrid); }

  template<class T>
  T *get_own_attr(uint32_t symid, uint32_t attrid) noexcept
  { return static_cast<T*>(get_own_attr(symid, attrid)); }

  template<class T>
  const T *get_own_attr(uint32_t symid, uint32_t attrid) const noexcept
  { return const_cast<Symtab*>(this)->get_own_attr<T>(symid, attrid); }

  /// Looks for an attribute in the symbol's attribute list, and if not found,
  /// searches the symbols in the meta list.
  /// @returns A pair of the symbol ID that actually owns the attribute and a
  ///          pointer to the attribute value.
  std::pair<uint32_t, void *>
  get_rec_attr(uint32_t symid, uint32_t attrid) noexcept;

  std::pair<uint32_t, const void *>
  get_rec_attr(uint32_t symid, uint32_t attrid) const noexcept {
    auto [a, b] = const_cast<Symtab*>(this)->get_rec_attr(symid, attrid);
    return std::pair<uint32_t, const void*>(a, b);
  }

  template<class T>
  std::pair<uint32_t, T*>
  get_rec_attr(uint32_t symid, uint32_t attrid) noexcept {
    auto [a, b] = get_rec_attr(symid, attrid);
    return std::pair<uint32_t, T*>(a, static_cast<T*>(b));
  }

  template<class T>
  std::pair<uint32_t, const T *>
  get_rec_attr(uint32_t symid, uint32_t attrid) const noexcept {
    auto [a, b] = const_cast<Symtab*>(this)->get_rec_attr<T>(symid, attrid);
    return std::pair<uint32_t, const T*>(a, b);
  }

  /// Recursively enumerates all properties on symbol `symid` and its
  /// metasymbols.
  /// @param f Receives the attribute ID and value and returns true to keep
  ///          enumerating or false to stop.
  template<class F>
  std::enable_if_t<
    std::is_invocable_r_v<bool, F, uint32_t, void*>,
    bool
  >
  enum_attrs(uint32_t symid, const F &f) {
    auto &sym = _symbols[symid];
    for (auto &a : sym.attrs) {
      if (!f(a.first, a.second)) return false;
    }
    for (auto m : sym.metas) {
      if (!enum_attrs<F>(m, f)) return false;
    }
    return true;
  }

  /// Recursively enumerates all properties on symbol `symid` and its
  /// metasymbols.
  /// @param f Receives the attribute ID and value and returns true to keep
  ///          enumerating or false to stop.
  template<class F>
  std::enable_if_t<
    std::is_invocable_r_v<bool, F, uint32_t, const void*>,
    bool
  >
  enum_attrs(uint32_t symid, const F &f) const {
    auto const &sym = _symbols[symid];
    for (auto const &a : sym.attrs) {
      if (!f(a.first, a.second)) return false;
    }
    for (auto m : sym.metas) {
      if (!enum_attrs<F>(m, f)) return false;
    }
    return true;
  }

  // Get the ID of the symbol's parent (its enclosing scope).
  uint32_t get_parent(uint32_t symid) const noexcept;

  // Get a pair `[id, index]` for a named child symbol of `parent`.
  std::pair<uint32_t, uint32_t>
  get_own_child_index(uint32_t parent, InternRef name) const noexcept;

  // Get a pair `[id, index]` for a named child symbol of `parent` with
  // convenience intern string lookup.
  std::pair<uint32_t, uint32_t>
  get_own_child_index(uint32_t parent, std::string_view name) const noexcept
  { return get_own_child_index(parent, find_interned(name)); }

  // Get the ID of a named symbol.
  uint32_t get_own_child(uint32_t parent, InternRef name) const noexcept
  { return get_own_child_index(parent, name).first; }

  // Get the ID of a named symbol with intern string lookup.
  uint32_t get_own_child(uint32_t parent, std::string_view name) const noexcept
  { return get_own_child(parent, find_interned(name)); }

  // Returns a tuple `[owner_id, child_id, index]` where `owner_id` is the
  // owning symbol of the child and a meta symbol of `parent`, `child_id` is
  // the own symbol ID of `parent` with name `name`, and `index` is the index
  // of `child_id` in `parent_id`.
  std::tuple<uint32_t, uint32_t, uint32_t>
  get_rec_child(uint32_t parent, InternRef name) const noexcept;

  // Returns a tuple `[parent_id, child_id, index]` where `parent_id` is the
  // owning symbol of the child and a meta symbol of `parent`, `child_id` is
  // the own symbol ID of `parent` with name `name`, and `index` is the index
  // of `child_id` in `parent_id`.
  std::tuple<uint32_t, uint32_t, uint32_t>
  get_rec_child(uint32_t parent, std::string_view name) const noexcept
  { return get_rec_child(parent, find_interned(name)); }

private:
  lava_arena _arena;
  std::vector<Symbol, lava::data::arena_allocator<Symbol>> _symbols;
  std::unordered_set<std::string_view> _intern_data;
};

} // namespace lava::sym

#endif /* LAVA_SYM_SYMTAB_H_ */
