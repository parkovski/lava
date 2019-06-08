#ifndef ASH_TERMINAL_H_
#define ASH_TERMINAL_H_

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

} // namespace ash::term

#endif /* ASH_TERMINAL_H_ */
