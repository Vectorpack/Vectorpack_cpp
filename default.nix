{ 
    #pkgs ? import (fetchTarball "https://github.com/NixOS/nixpkgs/archive/21.11.tar.gz") {}
    pkgs ? import <nixpkgs> {} # To be able to use the shell without internet
}:
let
    jobs = rec {
        vectorpack_lib = pkgs.stdenv.mkDerivation {
            name = "vectorpack_cpp";

            nativeBuildInputs = with pkgs; [
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

            enableParallelBuilding = true;
        };

        vectorpack = vectorpack_lib.overrideAttrs (attr: {
            cmakeFlags = ''-Dbuild_executable=ON'';
        });
    };
in
    jobs
