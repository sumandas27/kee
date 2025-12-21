# **kee**

**TODO: add a short description about the game**

## **Build Instruction:**

**Prerequites:**
* `cmake`
* `vcpkg` & its relevant setup.

### **Windows:**

**Additional prerequisites:**
* `Visual Studio 2022`

**Steps:**
1. Clone the repository and navigate to the root directory.
2. Run the following command (on one line):

    ```sh
    cmake                                           
        -G "Visual Studio 17 2022"                
        -DCMAKE_TOOLCHAIN_FILE=<path-to-vcpkg-root>/scripts/buildsystems/vcpkg.cmake
        -B build/
        .
    ```

3. Open the generated `.sln` file (should be `build/kee.sln`) with `Visual Studio 2022`.
4. Set `kee` from your Solution Explorer as your Startup Project and run (`F5`).

## **macOS:**

**TODO: write installation steps**