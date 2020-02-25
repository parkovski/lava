#include "ash/document.h"

#include <new>

using namespace ash::doc;
using ash::data::Rope;

DocumentBase::DocumentBase()
  : _rope()
{}

DocumentBase::DocumentBase(const char_type *text)
  : _rope(text)
{
  markNewlines(0, text);
}

DocumentBase::~DocumentBase()
{}

void DocumentBase::markNewlines(size_t index, const char_type *text) {
  size_t len = 0;
  size_t i = 0;
  char_type ch = text[0];
  while (ch) {
    if (ch == '\n') {
      _newlines.insert(index + len);
    }
    ++len;
    i += utf8_codepoint_size(ch);
    ch = text[i];
  }
}

bool DocumentBase::insert(size_t index, const char_type *text) {
  auto old_len = length();
  auto old_size = size();
  if (!_rope.insert(index, text)) {
    return false;
  }
  auto delta_len = length() - old_len;
  auto delta_size = size() - old_size;
  _newlines.shift(index, delta_len);
  markNewlines(index, text);
  emit(Insert(index, text, delta_len, delta_size));
  return true;
}

size_t DocumentBase::length() const {
  return _rope.length();
}

size_t DocumentBase::size() const {
  return _rope.size();
}

bool DocumentBase::append(const char_type *text) {
  auto old_length = length();
  auto old_size = size();
  if (!_rope.append(text)) {
    return false;
  }
  markNewlines(old_length, text);
  emit(Insert(old_length, text, length() - old_length, size() - old_size));
  return true;
}

void DocumentBase::erase(size_t index, size_t count) {
  auto old_size = size();
  _rope.erase(index, count);
  _newlines.shift(index, -static_cast<ptrdiff_t>(count));
  emit(Erase(index, count, old_size - size()));
}

bool DocumentBase::replace(size_t index, size_t count, const char_type *text) {
  auto old_length = length();
  //auto old_size = static_cast<ptrdiff_t>(size());
  if (index >= old_length) {
    return insert(index, text);
  }
  if (index + count > old_length) {
    count = old_length - index;
  }
  if (!_rope.replace(index, count, text)) {
    return false;
  }

  // old - count + n = new;
  // n = new + count - old;
  auto inserted = length() + count - old_length;
  _newlines.shift(index, -static_cast<ptrdiff_t>(count));
  _newlines.shift(index, inserted);
  emit(Replace(index, count, text, inserted));
  return true;
}

void DocumentBase::clear() {
  auto old_length = length();
  auto old_size = size();
  _rope.clear();
  _newlines.clear();
  emit(Erase(0, old_length, old_size));
}

size_t DocumentBase::substr(char_type *buf, size_t *bufsize, size_t index,
                        size_t count) const {
  return _rope.substr(buf, bufsize, index, count);
}

size_t DocumentBase::c_substr(char_type *buf, size_t *bufsize, size_t index,
                         size_t count) const {
  return _rope.c_substr(buf, bufsize, index, count);
}

std::string DocumentBase::substr(size_t index, size_t count) const {
  return _rope.substr(index, count);
}

char32_t DocumentBase::operator[](size_t index) const {
  return _rope[index];
}

// The document always has at least one line.
auto DocumentBase::lines() const -> grid_size_type {
  return static_cast<grid_size_type>(_newlines.size()) + 1;
}

// Returns a 1-based line index.
auto DocumentBase::lineAt(size_t index) const -> grid_size_type {
  if (_newlines.empty()) {
    return 1;
  }
  auto ln = _newlines.upper_bound(index);
  if (ln == _newlines.end()) {
    return static_cast<grid_size_type>(_newlines.size()) + 1;
  }
  return static_cast<grid_size_type>(_newlines.index_for(ln)) + 1;
}

// Make line and column a valid pair if either is out of range.
void DocumentBase::constrain(grid_size_type &line, grid_size_type &column) const {
  if (line == 0) {
    line = 1;
  }
  if (line > _newlines.size()) {
    line = static_cast<grid_size_type>(_newlines.size()) + 1;
  }
  if (column == 0) {
    column = 1;
  }
  if (_newlines.size() == 0) {
    line = 1;
    if (column > length()) {
      column = static_cast<grid_size_type>(length()) + 1;
    }
    return;
  }

  auto next_nl = _newlines.get(line - 1);
  if (next_nl == _newlines.end()) {
    auto first_col = *_newlines.rbegin() + 1;
    auto max_col = length() - first_col + 1;
    if (column > max_col) {
      column = static_cast<grid_size_type>(max_col);
    }
    return;
  }

  auto prev_nl = next_nl - 1;
  auto max_col = *next_nl - *prev_nl + 1;
  if (column > max_col) {
    column = max_col;
  }
}

// Returns the (line, column) pair for a character position.
// If pos is out of range, the last character position is returned.
auto DocumentBase::indexToPoint(size_t index) const
  -> std::pair<grid_size_type, grid_size_type>
{
  if (index > length()) {
    index = length();
  }
  if (_newlines.empty()) {
    return {1, static_cast<grid_size_type>(index) + 1};
  }

  // Newline after the current line.
  auto nl = _newlines.upper_bound(index);
  grid_size_type line, column;
  if (nl == _newlines.end()) {
    // Last line.
    line = static_cast<grid_size_type>(_newlines.size()) + 1;
    column = static_cast<grid_size_type>(index - *_newlines.rbegin() + 1);
  } else {
    line = static_cast<grid_size_type>(_newlines.index_for(nl)) + 1;
    if (line == 1) {
      // First line.
      column = static_cast<grid_size_type>(index) + 1;
    } else {
      column = static_cast<grid_size_type>(index - *(nl - 1) + 1);
    }
  }

  return {line, column};
}

// Returns a character position for a (line, column) pair.
// Out of range values are wrapped to [1, max].
size_t DocumentBase::pointToIndex(grid_size_type line, grid_size_type column) const {
  // Make indices 0-based.
  if (line > 0) {
    --line;
  }
  if (column > 0) {
    --column;
  }

  if (_newlines.empty()) {
    // Only one line in the document.
    if (column > length()) {
      return length();
    }
    return column;
  }

  // This is the newline at the _end_ of this line.
  auto nl = _newlines.get(line);
  if (nl == _newlines.end()) {
    // Last line.
    auto index = *_newlines.rbegin() + column;
    if (index > length()) {
      return length();
    }
    return index;
  } else {
    if (_newlines.index_for(nl) == 0) {
      // First line.
      if (column > *nl) {
        return *nl;
      }
      return column;
    }

    // Any middle line: the position is the column offset from the last
    // newline.
    auto index = *(nl - 1) + column;
    if (index > *nl) {
      return *nl;
    }
    return index;
  }
}

// Line numbers start at 1. The end of the span is the index of the newline
// character, or for the last line it is the character length of the document.
std::pair<size_t, size_t> DocumentBase::spanForLine(grid_size_type line) const
{
  size_t start;
  size_t end;

  // Make it zero-based.
  --line;

  auto newlines = _newlines.size();
  if (line > newlines) {
    // Line doesn't exist - index too high.
    start = end = npos;
  } else if (newlines == 0) {
    // Whole document is one line.
    start = 0;
    end = length();
  } else if (line == 0) {
    // First line starts at char 0.
    start = 0;
    end = _newlines[0];
  } else if (line == newlines) {
    // Last line ends at the last character of the document.
    start = _newlines[newlines - 1] + 1;
    end = length();
  } else {
    // All other lines span from [previous newline + 1, next newline].
    auto it = _newlines.get(line - 1);
    start = *it + 1;
    end = *++it;
  }

  assert(end >= start);
  return std::make_pair(start, end);
}
