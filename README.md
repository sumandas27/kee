# **kee**

**TODO: add a short description about the game**

## **Build Instructions:**

**Prerequites:**
* `cmake`
* `vcpkg` & its relevant setup.

**Steps:**
1. Clone the repository and navigate to the root directory.
2. Run `git submodule update --init --recursive`

    > **_NOTE:_** See `NOTE:` comment in `CMakeLists.txt` for why git submodules are used in conjunction with `vcpkg`.

3. Follow further installation steps below depending on your platform:

### **Windows:**

**Additional prerequisites:**
* `Visual Studio 2022`

**Additional Steps:**
4. Run the following command (on one line):

    ```sh
    cmake                                           
        -G "Visual Studio 17 2022"                
        -DCMAKE_TOOLCHAIN_FILE=<path-to-vcpkg-root>/scripts/buildsystems/vcpkg.cmake
        -B build/
    ```

5. Open the generated `.sln` file (should be `build/kee.sln`) with `Visual Studio 2022`.
6. Set `kee` from your Solution Explorer as your Startup Project and run (`F5`).

### **macOS:**

**Additional prerequisites:**
* `make`

**Steps:**
4. Clone the repository and navigate to the root directory.
5. Run the following command (on one line):

    ```sh
    cmake 
        -DCMAKE_C_COMPILER=/opt/homebrew/opt/llvm/bin/clang 
        -DCMAKE_CXX_COMPILER=/opt/homebrew/opt/llvm/bin/clang++ 
        -DCMAKE_CXX_STANDARD=23 
        -DCMAKE_OSX_SYSROOT=$(xcrun --sdk macosx --show-sdk-path) 
        -DCMAKE_EXE_LINKER_FLAGS="-L/opt/homebrew/opt/llvm/lib/c++ -L/opt/homebrew/opt/llvm/lib/unwind -lunwind" 
        -DCMAKE_CXX_FLAGS="-I/opt/homebrew/opt/llvm/include/c++/v1" 
        -DCMAKE_TOOLCHAIN_FILE=<path-to-vcpkg-root>/scripts/buildsystems/vcpkg.cmake 
        -B build/
    ```

6. `cd build`
7. Compile the program: `make`
8. `cd ..`
9. Run the program: `build/kee`