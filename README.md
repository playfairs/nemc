# NEM compiler

This repository contains the official NEM compiler implementation.

It provides the compiler pipeline for the NEM language:

- lexer
- parser
- AST
- NEMantics
- diagnostics
- NEM IR
- code generation
- compiler CLI
- compiler tests

The compiler is built with Nox and is intentionally separated from the language project in the sibling `nem` repository.

## Build

```sh
nox setup build
nox build build -j4
```

## Run

```sh
./build/debug/nemc ./program.nem ./program
```

## Tests

```sh
./build/debug/nem-tests
```
