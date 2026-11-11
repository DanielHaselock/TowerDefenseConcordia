# COMP345-A1

## Libraries used

### SDL2
SDL2 is used as the graphical library. Chosen because it is cross platform, widely used, easy to find documentation and examples.

## Catch2
Catch2 is used as the unit testing library. It was chosen due to its simplicty, header only and no deep integration needed with the build system. The code to test is simple, the simpler testing library the better.

## Linux grader instructions

These commands install the required tools and libraries, build and run the
program, run the tests, and generate the documentation. CMake finds SDL2 and
SDL2_image from the system installation, using pkg-config for SDL2_image when
the package does not provide a CMake config file.

### Install dependencies

On Ubuntu or a compatible Debian-based distribution:

```bash
sudo apt update
sudo apt upgrade
sudo apt install build-essential cmake doxygen catch2 libxext-dev libsdl2-dev libsdl2-image-dev
```

### Configure and build

Enable tests and documentation when configuring the project:

```bash
cmake -S . -B build -DBUILD_TESTS=ON -DBUILD_DOCUMENTATION=ON
```

### Build and run the Main program

```bash
cmake --build build --target Main
./build/Driver/Main
```

### Build and run the tests

```bash
cmake --build build --target Tests
ctest --test-dir build --output-on-failure
```

### Build the documentation

```bash
cmake --build build --target docs
```

The generated HTML documentation is at `build/docs/html/index.html`. On Linux
with a desktop environment, the build may also open it automatically.

## To Run - Windows

- Install Visual Studio or an IDE compatible with CMake.
- Install CMake and Doxygen.
- Run CMake or run `build.bat`.
- Open `build/TowerDefense.slnx` (or another generated project executable).

## To Run - macOS

- Install/update tools (Xcode Command Line Tools and Homebrew):
  ```bash
  xcode-select --install
  brew update
  brew install cmake doxygen catch2 sdl2 sdl2_image
  ```
- Build the program:
  ```bash
  cmake -S . -B build -DBUILD_DOCUMENTATION=OFF
  cmake --build build --target Main
  ```
- Run the program:
  ```bash
  ./build/Driver/Main
  ```

## Documentation - Windows

Build the `Docs` target in the IDE or run `OpenDocs.bat`.

## Documentation - macOS

- Install Doxygen (and optional Graphviz for diagrams) via Homebrew:
  ```bash
  brew install doxygen
  ```
- Build the docs:
  ```bash
  cmake -S . -B build -DBUILD_DOCUMENTATION=ON
  cmake --build build --target docs
  ```
- Open `build/docs/html/index.html` in the default browser:
  ```bash
  open build/docs/html/index.html
  ```
