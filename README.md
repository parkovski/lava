# Ash - A Language Design Experimentation Shell

I'm trying to see what kinds of introspection and metaprogramming I can
accomplish by using recent compiler techniques.

- What kinds of help and program information can we provide during editing?
- What other views of the program are possible (node graph, IR, ...)?
- How well can we work with multiple different languages/type systems?

## Research Areas

### Language Frontend Framework

Developers today expect their languages to support many features that are non-trivial to implement,
making it much harder for an indie language to gain adoption. A few of these include:

- Real-time introspection: we expect information about the program to be available while editing,
  including completion, error highlighting, go to definition, etc. Supporting these features
  requires a compiler frontend that interacts with the text editor and can recover well from
  errors and partially complete programs. Supporting these features requires the compiler to
  implement some text editor functionality and a flexible parser, AST, and symbol table that
  can quickly respond to changes. Communication with the text editor takes place over Microsoft's
  LSP, requiring JSON-RPC support.
- Linting and formatting: While in the past a compiler may have completely ignored whitespace,
  we now expect tooling to provide consistent formatting across the ecosystem. This adds some
  complexity to the parser and has driven a transition from abstract to concrete syntax trees,
  where every part of the program text is represented.
- Documentation: In addition to whitespace, we want the compiler to be able to read comments,
  attach them to the appropriate syntax item, and understand some embedded documentation syntax,
  usually based on JavaDoc or XML.
- REPL: While traditionally specific to interpreted languages, there is a recent trend to provide
  these tools for even fully compiled languages: Cling for C++, Miri for Rust. For compiled
  languages, this requires an understanding of the platform ABI and FFI functionality.
- Project and package management: From the simplicity of `npm install` to the plethora of
  C++ build tools and scripting languages, we expect it to be easy to use 3rd party libraries
  and build our projects for many different platforms. Solving this problem in a generic manner
  may be somewhere between very difficult and impossible, but it should at least be possible
  to provide libraries to help build these tools.
- Debuggers: In addition to the LSP, there is a Debug Adapter Protocol.

### Trends Across Languages

Disclaimer: I have written significant amounts of C++, C#, JavaScript, and TypeScript, and
my thoughts will be heavily influenced by the evolution of those languages. However, I am also
considering developments in Rust, Shell (I use zsh and PowerShell), Haskell, and Lisp, among
others.
