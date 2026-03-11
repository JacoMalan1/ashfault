{
  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs?ref=nixos-unstable";
    flake-parts.url = "github:hercules-ci/flake-parts";
  };

  outputs =
    inputs@{ flake-parts, ... }:
    flake-parts.lib.mkFlake { inherit inputs; } (
      { ... }:
      {
        systems = [ "x86_64-linux" ];
        perSystem =
          { pkgs, ... }:
          {
            devShells.default = pkgs.mkShell {
              nativeBuildInputs = with pkgs; [ pkg-config ];
              buildInputs = with pkgs; [
		libGL
		cmake
		vulkan-tools
                vulkan-headers
		vulkan-validation-layers
                vulkan-loader
                wayland-scanner
                wayland-protocols
                egl-wayland
                wayland
                libxkbcommon
		libx11
		libffi
		libxrandr
		libxinerama
		libxcursor
		libxi
		shaderc
		spirv-tools
		clang-tools
		xwayland
              ];

	      env = {
		LD_LIBRARY_PATH = pkgs.lib.makeLibraryPath [ pkgs.wayland pkgs.libxkbcommon pkgs.libx11 ];
		MAKEFLAGS = "-j16";
	      };
            };
          };
      }
    );
}
