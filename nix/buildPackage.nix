{ pkgs ? import <nixpkgs> { }, }:
pkgs.stdenv.mkDerivation {
  pname = "nemc";
  version = pkgs.lib.strings.trim (builtins.readFile ../VERSION);
  src = ../.;

  nativeBuildInputs = [ pkgs.clang ];

  installPhase = ''
    mkdir -p $out/bin
    cc -std=c11 -Wall -Wextra \
      cli/main.c \
      compiler/support/source.c \
      compiler/ast/ast.c \
      compiler/lexer/lexer.c \
      compiler/parser/parser.c \
      compiler/nemantics/nemantics.c \
      compiler/diagnostics/diagnostics.c \
      compiler/ir/ir.c \
      compiler/codegen/codegen.c \
      compiler/driver/driver.c \
      -Iinclude \
      -o $out/bin/nemc
  '';

  meta = {
    mainProgram = "nemc";
  };
}
