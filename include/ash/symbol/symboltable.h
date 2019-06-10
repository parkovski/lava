#ifndef ASH_SYM_SYMBOLTABLE_H_
#define ASH_SYM_SYMBOLTABLE_H_

#include "symbol.h"

#include <vector>
#include <unordered_map>
#include <string>
#include <string_view>
#include <cstring>

namespace ash::sym {

struct SymbolData {
private:
  friend class SymbolTable;

  /// Full name of the symbol. Because of quirks in unordered_map, it actually
  /// contains an owned pointer and may not be changed.
  std::string_view _name;

  symbol_t _descriptor;

  size_t _attribute_count;
  void **_attributes;

  /// Pointer to the actual data.
  void *_data = nullptr;

  /// Dummy constructor for \c SymbolTable.
  explicit SymbolData() noexcept
    : _name(nullptr, 0), _data(nullptr)
  {}

public:
  /// Construct the SymbolData by copying the name.
  explicit SymbolData(std::string_view name, symbol_t descriptor)
    : _descriptor(descriptor),
      _attribute_count(0),
      _attributes(nullptr)
  {
    auto length = name.length();
    char *name_p = new char[length];
    memcpy(name_p, name.data(), length);
    _name = std::string_view(name_p, length);
  }

  /// \c SymbolData may not be copied.
  SymbolData(const SymbolData &) = delete;

  /// Standard move constructor. \c other is invalid after this point.
  SymbolData(SymbolData &&other) noexcept {
    *this = std::move(other);
  }

  /// Frees data owned by the symbol.
  ~SymbolData() {
    if (auto name_p = _name.data()) {
      delete[] name_p;
    }
  }

  /// \c SymbolData may not be copied.
  SymbolData &operator=(const SymbolData &) = delete;

  /// Standard move assignment. \c other is invalid after this point.
  SymbolData &operator=(SymbolData &&other) noexcept {
    _name = other._name;
    _descriptor = other._descriptor;
    _attribute_count = other._attribute_count;
    _attributes = other._attributes;
    _data = other._data;

    other._name = std::string_view(nullptr, 0);
    other._descriptor = symbol_t();
    other._attribute_count = 0;
    other._attributes = nullptr;
    other._data = nullptr;

    return *this;
  }

  std::string_view name() const {
    return _name;
  }

  symbol_t descriptor() const {
    return _descriptor;
  }

  size_t attributeCount() const {
    return _attribute_count;
  }

  void *attribute(size_t n) const {
    return _attributes[n];
  }

  const void *data() const {
    return _data;
  }

  void *data() {
    return _data;
  }
};

/**
 * \brief Holds string->symbol and symbol->data mappings.
 */
class SymbolTable {
public:
  using data_type = SymbolData;

  /// Constructs an empty symbol table.
  explicit SymbolTable() noexcept {}

  /// Symbol tables cannot be copied.
  SymbolTable(const SymbolTable &) = delete;

  /// Default move constructor.
  SymbolTable(SymbolTable &&) = default;

  /// Symbol tables cannot be copied.
  SymbolTable &operator=(const SymbolTable &) = delete;

  /// Default move assignment operator.
  SymbolTable &operator=(SymbolTable &&) = default;

  template<typename T>
  symbol_t registerType(std::string_view path) {
    return const_cast<symbol_t>(T::ash_descriptor) = findOrInsertSymbol(path);
  }

  /// Return the symbol for the given string. The return value will be
  /// invalid if the symbol does not exist.
  symbol_t findSymbol(std::string_view str) const {
    auto it = _symbolMap.find(str);
    if (it == _symbolMap.end()) {
      return symbol_t();
    }
    return it->second;
  }

  /// Return the symbol for the given string, or insert a new symbol if it
  /// did not previously exist. The returned symbol value will always be valid.
  symbol_t findOrInsertSymbol(std::string_view str) const {
    auto [it, inserted] = _symbolMap.try_emplace(str, symbol_t());
    if (inserted) {
      auto index = _symbolData.size();
      auto sym = symbol_t::fromSize(index);
      _symbolData.emplace_back(SymbolData{str, symbol_t()});
      // This looks sketchy, however we're just swapping pointers to an
      // equivalent string with a longer lifetime.
      const_cast<std::string_view &>(it->first) = _symbolData[index]._name;
      it->second = sym;
      return sym;
    }
    return it->second;
  }

  /// Check if the symbol exists in this symbol table.
  bool isSymbolValid(symbol_t sym) const {
    return static_cast<size_t>(sym.id()) < _symbolData.size();
  }

  const data_type *symbol(symbol_t sym) const {
    size_t index = sym.id();
    if (index >= _symbolData.size()) {
      return nullptr;
    }
    return &_symbolData[index];
  }

  data_type *symbol(symbol_t sym) {
    size_t index = sym.id();
    if (index >= _symbolData.size()) {
      return nullptr;
    }
    return &_symbolData[index];
  }

  /// Get the data for the given symbol, or \c nullptr if the symbol does not
  /// exist.
  const void *data(symbol_t sym) const {
    size_t index = sym.id();
    if (index >= _symbolData.size()) {
      return nullptr;
    }
    return _symbolData[index].data();
  }

  /// Get the data for the given symbol, or \c nullptr if the symbol does not
  /// exist.
  void *data(symbol_t sym) {
    size_t index = sym.id();
    if (index >= _symbolData.size()) {
      return nullptr;
    }
    return _symbolData[index].data();
  }

  /// Equivalent to \c *symbol(sym). Behavior is undefined if the symbol does
  /// not exist.
  const data_type &operator[](symbol_t sym) const {
    return _symbolData[sym.id()];
  }

  /// Equivalent to \c *symbol(sym). Behavior is undefined if the symbol does
  /// not exist.
  data_type &operator[](symbol_t sym) {
    return _symbolData[sym.id()];
  }

  /// Equivalent to \c *symbol(findOrInsertSymbol(str)). Because of the risk of
  /// undefined behavior, the symbol is inserted if it did not previously
  /// exist.
  const data_type &operator[](std::string_view str) const {
    return (*this)[findOrInsertSymbol(str)];
  }

  /// Equivalent to \c *symbol(findOrInsertSymbol(str)). Because of the risk of
  /// undefined behavior, the symbol is inserted if it did not previously
  /// exist.
  data_type &operator[](std::string_view str) {
    return (*this)[findOrInsertSymbol(str)];
  }

private:
  mutable std::unordered_map<std::string_view, symbol_t> _symbolMap;
  mutable std::vector<data_type> _symbolData;
};

} // namespace ash::sym

#endif /* ASH_SYM_SYMBOLTABLE_H_ */

