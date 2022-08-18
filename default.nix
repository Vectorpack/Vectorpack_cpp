{ 
    #pkgs ? import (fetchTarball "https://github.com/NixOS/nixpkgs/archive/21.11.tar.gz") {}
    pkgs ? import <nixpkgs> {} # To be able to use the shell without internet
}:
let
    jobs = rec {
        shell = pkgs.mkShell { # Dev shell TODO remove it
            name = "shell";
            buildInputs = vectorpack.buildInputs ++
                [
                  pkgs.valgrind
                ];
        };

        vectorpack_lib = pkgs.stdenv.mkDerivation {
            name = "vectorpack_cpp";

            buildInputs = with pkgs; [
                cmake
            ];

            src = pkgs.lib.sourceByRegex ./. [
                "^CMakeLists.txt"
                "^src"
                "^src/main_vectorpack.cpp"
                 "^src/lib"
                 "^src/lib/.*\.?pp"
                 "^src/algos"
                 "^src/algos/.*\.?pp"
            ];
        };

        vectorpack = vectorpack_lib.overrideAttrs (attr: rec {
            configurePhase = ''
                mkdir build && cd build
                cmake -Dbuild_executable=ON -DCMAKE_INSTALL_PREFIX=$out ..
            '';
        });
    };
in
    jobs
