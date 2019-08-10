#ifndef ASH_TERMINAL_TERMINAL_H_
#define ASH_TERMINAL_TERMINAL_H_

#include <cstddef>
#include <utility>

namespace ash::term {

/// Initializes the terminal library and saves the previous state.
void initialize();

/// Is stdin a TTY?
bool isTTYInput();

/// Is stdout a TTY?
bool isTTYOutput();

/// Is stderr a TTY?
bool isTTYError();

/// Save the current terminal state.
void saveState();

/// Set the terminal to shell line edit mode.
void setShellState();

/// Restore from the saved terminal state.
void restoreState();

/// Read the next character from the input.
int getChar();

/// Read at least min and at most max chars into buf. Returns the number of
/// characters read.
size_t getChars(char *buf, size_t min, size_t max);

/// Get the terminal buffer size.
std::pair<short, short> getSize();

/// Register a callback when the window is resized.
void onResize(void (*handler)(short x, short y));

} // namespace ash::term

#endif /* ASH_TERMINAL_TERMINAL_H_ */
