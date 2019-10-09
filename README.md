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

### Background and Modern Language Trends

Disclaimer: I have written significant amounts of C++, C#, JavaScript, and TypeScript, and
my thoughts will be heavily influenced by the evolution of those languages. However, I am also
considering developments in Rust, Shell (I use zsh and PowerShell), Haskell, and Lisp, among
others.

#### Functional Programming

Mainstream languages have been gradually adopting syntax and library features pioneered by
functional languages such as Lisp, ML, and Haskell. The addition of succinct lambda syntax
to JavaScript, C#, C++, and Java has enabled us to approach problems in a different, often
simpler way. Libraries such as LINQ and C++ ranges, based on standard functional operations,
approach a monadic view of containers while coroutines and async/await syntax extend the
range of monadic operations available to these languages. It appears that, similar to the
way C pointers led to the simpler object references in higher level languages, monads in a
modified form are becoming more widespread.

#### Metaprogramming

The most familiar example of metaprogramming is probably the C++ template system. It is difficult
to write, difficult to debug, and has very limited abilities to obtain information about and change
the program itself, but it is powerful nonetheless.

Source code transformers for C# and JavaScript are in common use in some areas, but metaprogramming
in general remains not very user friendly. The Zig and D languages are of particuar interest here,
as they attempt to support arbitrary compile-time code execution. C++ also seems to be slowly
adding more user-friendly ways to implement compile-time execution. There is an interesting project,
the "ConstExprPreter", which aims to add a bytecode interpreter to clang to improve compile-time
execution speed. And of course there is the original meta-language, Lisp.

#### Advanced Type Systems

C++, Rust, and TypeScript are mainstream languages which encode a large amount of information
in the type system. Features in C++ and Rust improve inference for their complex type systems,
while TypeScript, Flow, Python and PHP have been adding optional typing to dynamic languages.
It appears that static and dynamic languages are approaching a common point from opposite ends
of the spectrum, where types don't always need to be explicitly specified, but are available
to aid in catching errors before runtime.

#### Return to Performance Emphasis

As the internet became popular, so did slow dynamic languages such as PHP, Python, and Ruby.
I don't include JavaScript because its popularity was driven more by necessity. At first,
the speed of these languages wasn't very important as early web apps relied on them mostly
to run database queries and format HTML pages. Additionally, the languages traded runtime
safety for developer productivity, which was really feasible for the first time as websites
could be patched much easier than physical copy software.

Over time, websites became more complicated and the server side scripts took on more
responsibilities. Their slowness started to present more of a problem: Reddit, running
on Python, used to often suffer from overloaded servers. Facebook began transpiling their
PHP code to C++ and eventually implemented a PHP JIT. Sites running on Java and C# seemed
to not have as many issues as sites running mainly on dynamic languages.

With the invention of LLVM, it became drastically easier to write a production grade optimizing
compiler. New experimental compiled languages began to appear. Rust and C++ popularized the idea
of zero-cost abstractions, and languages like Go and Swift tried to retain much of the developer
productivity that traditionally was only available to interpreted languages, while staying close
to the speed of C. The traditional triangle problem, "fast, safe, easy to write" is approaching
fulfilling all three criteria.

### Unsolved Issues

#### Interop

It still is mostly the case that parts of a program written in different languages need to
communicate over the C ABI, and C wrappers have to be written manually. The underlying ABI
is not particularly important, especially if we can use LLVM and LTO which will optimize out
any bridge code, but we shouldn't need to write wrapper code.

#### Shared Data Structures

There are systems such as Thrift, *MQ, COM, and IDL that attempt to provide common interfaces
to multiple languages, but they necessarily add complexity to a project. Is there a way to work
with distributed systems, including versioned data types, without a bolt-on type solution?

#### Security-First Programming

The appearance of blockchain and especially Ethereum has made visible the importance of
verifiably secure programs. What language features can be used to make security the top
priority? What can the compiler do to enforce security?

#### Distributed Systems

How can we improve the experience of working on a system made up of multiple programs,
possibly using multiple languages? What can be done to reduce code duplication, boilerplate,
and improve testing and debugging across the whole system?

#### Indie Language Developers

As pointed out in the first section, writing a modern language frontend takes a lot of work.
How can this experience be simplified? If more people are able to experiment with language
design, we will approach solutions to many of these problems much faster.
