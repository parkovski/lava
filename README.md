# Ash - A Language Design Experimentation Shell

I'm trying to see what kinds of introspection and metaprogramming I can
accomplish by using recent compiler techniques.

- What kinds of help and program information can we provide during editing?
- What other views of the program are possible (node graph, IR, ...)?
- How well can we work with multiple different languages/type systems?

## Rationale

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

## Modern Language Trends

Disclaimer: I have written significant amounts of C++, C#, JavaScript, and TypeScript, and
my thoughts will be heavily influenced by the evolution of those languages. However, I am also
considering developments in Rust, Shell (I use zsh and PowerShell), Haskell, and Lisp, among
others.

### Functional Programming

Mainstream languages have been gradually adopting syntax and library features pioneered by
functional languages such as Lisp, ML, and Haskell. The addition of succinct lambda syntax
to JavaScript, C#, C++, and Java has enabled us to approach problems in a different, often
simpler way. Libraries such as LINQ and C++ ranges, based on standard functional operations,
approach a monadic view of containers while coroutines and async/await syntax extend the
range of monadic operations available to these languages. It appears that, similar to the
way C pointers led to the simpler object references in higher level languages, monads in a
modified form are becoming more widespread.

### Metaprogramming

The most familiar example of metaprogramming is probably the C++ template system. It is difficult
to write, difficult to debug, and has very limited abilities to obtain information about and change
the program itself, but it is powerful nonetheless.

Source code transformers for C# and JavaScript are in common use in some areas, but metaprogramming
in general remains not very user friendly. The Zig and D languages are of particuar interest here,
as they attempt to support arbitrary compile-time code execution. C++ also seems to be slowly
adding more user-friendly ways to implement compile-time execution. There is an interesting project,
the "ConstExprPreter", which aims to add a bytecode interpreter to clang to improve compile-time
execution speed. And of course there is the original meta-language, Lisp.

### Advanced Type Systems

C++, Rust, and TypeScript are mainstream languages which encode a large amount of information
in the type system. Features in C++ and Rust improve inference for their complex type systems,
while TypeScript, Flow, Python and PHP have been adding optional typing to dynamic languages.
It appears that static and dynamic languages are approaching a common point from opposite ends
of the spectrum, where types don't always need to be explicitly specified, but are available
to aid in catching errors before runtime.

### Return to Performance Emphasis

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

## Research Areas

### Interop

It still is mostly the case that parts of a program written in different languages need to
communicate over the C ABI, and C wrappers have to be written manually. The underlying ABI
is not particularly important, especially if we can use LLVM and LTO which will optimize out
any bridge code, but we shouldn't need to write wrapper code.

### Shared Data Structures

There are systems such as Thrift, *MQ, COM, and IDL that attempt to provide common interfaces
to multiple languages, but they necessarily add complexity to a project. Is there a way to work
with distributed systems, including versioned data types, without a bolt-on type solution?

### Security-First Programming

The appearance of blockchain and especially Ethereum has made visible the importance of
verifiably secure programs. What language features can be used to make security the top
priority? What can the compiler do to enforce security?

### Distributed Systems

How can we improve the experience of working on a system made up of multiple programs,
possibly using multiple languages? What can be done to reduce code duplication, boilerplate,
and improve testing and debugging across the whole system?

### Indie Language Developers

As pointed out in the first section, writing a modern language frontend takes a lot of work.
How can this experience be simplified? If more people are able to experiment with language
design, we will approach solutions to many of these problems much faster.

### Code Trust

Can we create a blockchain based system that allows us to run arbitrary binaries where
the community has decided that the binary is generally safe? What levels of safety should
we provide? How does this integrate with sandboxing, containers, app virtualization, etc?
What levels of granularity should there be (whole app, per library, others)?

The main rationale behind this thought is that the development of smart contracts and dapps
has resulted in an explosion of node type utilities, where a user must run a persistent
background process to participate in some auxiliary network. It doesn't seem feasible to me
that these networks will really be decentralized unless we are able to automatically download
and run the various node processes, but this raises the issue of running arbitrary binary
code which has been proven to be generally a bad idea.

Developing the ability to run a node process manager which automatically downloads and executes
programs that support some network would probably help a lot toward the decentralization goal
and make this paradigm viable.

## Generalized Concepts

### System

A system is made of one or more programs, language and runtime agnostic, that
share data types and communication protocols.

### Program

A program is an executable process on any runtime in any environment that supports sending
and/or receiving structured data over at least one protocol/ABI.

### ABI

An ABI defines a data encoding and function call mechanism. The function call part of the ABI
also specifies access control and error handling behavior. These are broad concepts, so while
the C ABI may only have the concepts of static/global/export access, an HTTP ABI
may define access control based on runtime lookup of session users, and a system local ABI
may specify that only certain programs can call certain functions.

### Function

A piece of executable code with a name, parameter set, return value, access control,
and error behavior, defined according to an ABI.

### Data

Anything residing in memory. One of the goals of ash is a type system flexible enough to represent
any type of data in any format. The type of data ranges from "something reachable via pointer" to
"blob with a known size" to structures with individual members, padding, alignment, inheritance, etc.
This is an area of ongoing investigation.

### Access Control

The caller must identify themselves to a function, and the function may implement various static
or dynamic methods of allowing or denying the caller to call the function. In case of access failure,
one of the ABI-defined error handling methods will be invoked.

## Prior Art

- Symbol table
  - Hierarchical file systems
  - Windows NT Object Manager
- Type systems
  - C++, especially templates and concepts
  - Rust's innovative use of memory regions
  - TypeScript's innovative representation of JavaScript's inherent, highly dynamic type system
  - Haskell
  - Lambda calculus, System U, Calculus of Constructions.
- Interactive parsers
  - .NET Roslyn
  - TypeScript
- Macros
  - Lisp (aka Granddaddy Mac, he got lots o' child'n)
  - Rust
  - Sweet.js
- IPC
  - REST
  - JSON-RPC
  - COM
  - Message brokers (MQ, Thrift, etc).
- Tooling
  - Documentation
    - JavaDoc
    - XML Doc
    - Doxygen
  - Linting
    - eslint, prettier
    - clang-tidy
    - rustfmt
  - Package management
    - npm
    - cargo
    - apt, pacman
    - vcpkg
  - Build tools
    - CMake
    - Webpack
- Interactive coding
  - Swift playgrounds
  - Jupyter Notebook
  - Shell
  - Sonic Pi
  - Various live shader editors
- Introspection and alternate code views
  - Code lens
  - Unreal Blueprints
  - Live previews for Markdown, HTML, and other document/markup type languages
