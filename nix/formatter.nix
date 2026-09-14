{ pkgs, inputs }:
let
  treefmt = inputs.treefmt-nix.lib.mkWrapper pkgs {
    programs.nixfmt.enable = true;
    programs.clang-format.enable = true;
  };
in
  treefmt
