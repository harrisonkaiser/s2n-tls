{
  pkgs
}:
pkgs.stdenv.mkDerivation rec {
  pname = "";
  version = "";

  src = fetchTarball {
    url = "https://www.openssl.org/source/old/0.9.x/openssl-0.9.8zh.tar.gz";
    sha256 = "";
  };

  buildInputs = [
    pkgs.make
  ];

  configurePhase =
    if pkgs.stdenv.hostPlatform.isLinux then ''
        ./config -d
        ./configure --prefix="$out"
        ''
    else
      (if pkgs.stdenv.hostPlatform.isDarwin
       then ''
            ./Configure darwin64-x86_64-cc
            ./configure --prefix="$out"
            ''
       else ''
           echo you need to add this platform!
           exit 1;
         ''
      );

  buildPhase = ''
    make depend
    make
  '';

  installPhase = ''
    make install
  '';
}
