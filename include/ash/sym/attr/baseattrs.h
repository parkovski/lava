#ifndef ASH_ATTR_BASEATTRS_H_
#define ASH_ATTR_BASEATTRS_H_

#include "../symboltable.h"

namespace ash::sym::attr {

// MemReq is the minimal description of the memory requirements of a data type.
struct MemReq {
  static const symbol_t ash_descriptor;

  // Total known size of the data of this type. The data may exceed this size
  // but may not be smaller. If \c min_size returns 0, there may or may not
  // be data - even this is not known. When <c>min_size() == max_size()</c>,
  // the exact size of the data is known.
  size_t min_size() const
  { return _min_size; }

  // Maximum size of the data. The data may be smaller than this, but not
  // greater. If \c max_size returns 0, there is no possible data of this type.
  // When <c>min_size() == max_size()</c>, the exact size of the data is known.
  size_t max_size() const
  { return _max_size; }

  // Alignment required to safely access this type. The default is the nearest
  // power of two by rounding up, with a maximum of the machine's pointer size.
  size_t alignment() const
  { return _alignment; }

private:
  size_t _min_size;
  size_t _max_size;
  size_t _alignment;
};

// Size specifies a type that has a known, non-variable size.
struct Size {
  static const symbol_t ash_descriptor;

  // The absolute size of the object.
  size_t size() const
  { return _size; }

private:
  size_t _size;
};

// Data that requires cleanup when it goes out of scope.
struct Destroy {
  static const symbol_t ash_descriptor;

  void destroy(void *data) const
  { _destructor(data); }

private:
  void (*_destructor)(void *);
};

// Standard array - the data is split into pieces of the same type and size.
struct Array {
  static const symbol_t ash_descriptor;

  size_t count() const
  { return _count; }

  symbol_t type() const
  { return _type; }

private:
  size_t _count;
  symbol_t _type;
};

// The data is split into heterogeneous pieces.
struct Container {
  static const symbol_t ash_descriptor;

  size_t count() const
  { return _count; }

  symbol_t child(size_t index) const
  { return _children[index]; }

private:
  size_t _count;
  symbol_t *_children;
};

// Indirect storage.
struct Pointer {
  static const symbol_t ash_descriptor;

  symbol_t type() const
  { return _type; }

private:
  symbol_t _type;
};

} // namespace ash::sym::attr

#endif // ASH_ATTR_BASEATTRS_H_