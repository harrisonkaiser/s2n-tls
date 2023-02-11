{
  pkgs
}:
pkgs.stdenv.mkDerivation rec {
  pname = "libressl";
  version = "3.6.1";

  src = fetchTarball {
    url = "https://s3-us-west-2.amazonaws.com/s2n-public-test-dependencies/2022-12-01_libressl-3.6.1.tar.gz";
    sha256 = "sha256:03gqcckknxcj95n6jf35arkxrn5q2530clryqni0ij6ad2qd7d8f";
  };

  buildInputs = [
    pkgs.gnumake
#    pkgs.perl534
  ];

#  patchPhase = ''
#      substitute ./config ./config --replace /usr/bin/env ${pkgs.coreutils}/bin/env
#  '';

  configurePhase = ''
      ./configure --prefix=$out
  '';

  buildPhase = ''
    make CFLAGS=-fPIC
  '';

  installPhase = ''
    make install
  '';
}
