#ifndef ASH_LINEEDITOR_H_
#define ASH_LINEEDITOR_H_

#include <string>

namespace ash {

class LineEditor {
public:
  using PromptFn = void (*)();

  explicit LineEditor() noexcept;

  static void defaultPrompt();

  /// Set the prompt function.
  /// \param prompt The new prompt function.
  void setPrompt(PromptFn prompt) {
    if (prompt) {
      _prompt = prompt;
    } else {
      _prompt = LineEditor::defaultPrompt;
    }
  }

  /// \param line The text that was read on a true return.
  /// \returns True on end of line, false on end of input.
  bool readLine(std::string &line);

private:
  int cursorPos;
  int cursorLeft;
  int cursorTop;
  PromptFn _prompt;
};

} // namespace ash

#endif /* ASH_LINEEDITOR_H_ */

