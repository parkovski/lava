#ifndef LAVA_LANG_SOURCEMANAGER_H_
#define LAVA_LANG_SOURCEMANAGER_H_

#include "sourcedoc.h"

#include <string>
#include <vector>
#include <unordered_map>
#include <string_view>

namespace lava::lang {

struct SourceManager {
private:
  std::vector<SourceDoc> _docs;
  std::unordered_map<std::string_view, DocId> _paths;

public:
  bool has_doc(std::string_view path) const {
    return _paths.contains(path);
  }
  DocId add_doc(std::string_view path) {
    if (has_doc(path)) {
      return InvalidDocId;
    }
    auto &doc = _docs.emplace_back(
      static_cast<DocId>(_docs.size()), std::string{path});
    _paths.emplace(doc.path(), doc.id());
    return doc.id();
  }
  DocId id_for_path(std::string_view path) {
    auto it = _paths.find(path);
    if (it == _paths.end()) {
      return InvalidDocId;
    }
    return it->second;
  }
  SourceDoc &operator[](DocId id) { return _docs[id]; }
  const SourceDoc &operator[](DocId id) const { return _docs[id]; }

  const SourceLoc &loc(LocRef l) const { return (*this)[l.doc_id][l.loc_id]; }
  size_t offset(LocRef l) const { return loc(l).offset; }
  uint32_t line(LocRef l) const { return loc(l).line; }
  uint32_t column(LocRef l) const { return loc(l).column; }
  std::string_view text(LocRef l) const {
    return (*this)[l.doc_id].text(l.loc_id);
  }
};

} // namespace lava::lang

#endif // LAVA_LANG_SOURCEMANAGER_H_
