{
  description = "A flake for s2n-tls";

  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-22.11";

  outputs = { self, nix, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let pkgs = nixpkgs.legacyPackages.${system};
          # TODO: We have parts of our CI that rely on clang-format-15, but that is only avalible on github:nixos/nixpkgs/nixos-unstable
          llvmPkgs = pkgs.llvmPackages_14;
          pythonEnv = import ./tests/integrationv2/pyenv.nix { pkgs = pkgs; };
          openssl_0_9_8 = import codebuild/bin/install_openssl_0_9_8.nix {pkgs=pkgs;};
          openssl_1_0_2 = import codebuild/bin/install_openssl_1_0_2.nix {pkgs=pkgs;};
          openssl_1_1_1 = import codebuild/bin/install_openssl_1_1_1.nix {pkgs=pkgs;};
          openssl_3_0 = import codebuild/bin/install_openssl_3_0.nix {pkgs=pkgs;};
      in rec {
        packages.s2n-tls = pkgs.stdenv.mkDerivation {
          src = self;
          name = "s2n-tls";
          inherit system;

          nativeBuildInputs = [ pkgs.cmake ];
          buildInputs = [ pkgs.openssl ];

          cmakeFlags = [
            "-DBUILD_SHARED_LIBS=ON"
            "-DCMAKE_BUILD_TYPE=RelWithDebInfo"
            "-DS2N_NO_PQ=1" # TODO: set when system like aarch64/mips,etc
          ];

          propagatedBuildInputs = [ pkgs.openssl ];
        };
        devShells.default = pkgs.mkShell rec {
          packages = [
            #llvmPkgs.llvm
            #llvmPkgs.llvm-manpages
            #llvmPkgs.libclang
            # llvmPkgs.clangUseLLVM -- wrapper to overwrite default compiler with clang
            #llvmPkgs.clang-manpages
            #pkgs.cppcheck
            #pythonEnv
            # TODO: can we use the version in bindings/rust/rust-toolchain
            # it goes against the spirit of nix to use rustup... but we might
            # have to -- using a new rust is liable to get us in trouble.
            #pkgs.rustc
            #pkgs.cargo
            #pkgs.openjdk8
            #openssl_0_9_8
            #openssl_1_0_2
            #openssl_1_1_1
            openssl_3_0
          ];
        };
        packages.default = packages.s2n-tls;
        packages.s2n-tls-openssl3 = packages.s2n-tls.overrideAttrs
          (finalAttrs: previousAttrs: { doCheck = true; });
        packages.s2n-tls-openssl11 = packages.s2n-tls.overrideAttrs
          (finalAttrs: previousAttrs: {
            doCheck = true;
            buildInputs = [ pkgs.openssl_1_1 ];
          });
        packages.s2n-tls-libressl = packages.s2n-tls.overrideAttrs
          (finalAttrs: previousAttrs: {
            doCheck = true;
            buildInputs = [ pkgs.libressl ];
          });
        formatter = pkgs.nixfmt;
      });
}
