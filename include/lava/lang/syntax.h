#ifndef LAVA_LANG_SYNTAX_H_
#define LAVA_LANG_SYNTAX_H_

#include "sourcemanager.h"

#include <vector>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <string_view>

namespace lava::lang {

using istring = std::string_view;

struct Program;
struct ModuleNode;

struct Node {
private:
  const ModuleNode *_module;

public:
  explicit Node(const ModuleNode *module) noexcept
    : _module{module}
  {}

  Node(const Node &) = delete;
  Node &operator=(const Node &) = delete;

  virtual ~Node() = 0;

  const ModuleNode &module() const { return *_module; }
  inline const Program &program() const;
  inline const SourceManager &source_manager() const;
};

struct SymbolicNode : Node {
  using Node::Node;

  LocRef name_loc;
};

struct FunDeclNode : SymbolicNode {
  LocRef fun_loc;
  LocRef open_paren_loc;
  LocRef close_paren_loc;
};

struct ModuleNode : Node {
private:
  friend struct Node;

  const Program *_program;

public:
  std::vector<std::unique_ptr<SymbolicNode>> nodes;

  explicit ModuleNode(const Program &program)
    noexcept
    : Node{this}
    , _program{&program}
  {}
};

inline const Program &Node::program() const {
  return *_module->_program;
}

struct Program {
private:
  SourceManager _source_manager;
  std::unordered_map<istring, ModuleNode> _modules;
  mutable std::unordered_set<std::string_view> _strings;
  mutable std::string _the_string;

public:
  SourceManager &source_manager() { return _source_manager; }
  const SourceManager &source_manager() const { return _source_manager; }

  ModuleNode *add_module(istring name) {
    auto [it, inserted] = _modules.emplace(name, *this);
    if (inserted) {
      return &it->second;
    }
    return nullptr;
  }

  istring intern(std::string_view str) const;
  const std::string &the_string() const { return _the_string; }
};

inline const SourceManager &Node::source_manager() const {
  return program().source_manager();
}

} // namespace lava::lang

#endif // LAVA_LANG_SYNTAX_H_
