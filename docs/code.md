# Coding Conventions
## C++ Runtime State
Implementing the C++ runtime is required; errors are common during this stage.

## C++ General:
* The C++ standard used is C++23; any C/C++ standard library function is avoided; prefer engine implementations.
* The C++ runtime was removed, which can lead to several problems when reading the codebase.
* new/delete and their variants must be avoided; their usage will trigger a debugbreak() ([see](../src/engine/engine_runtime.cpp)).
* The compiler flags trigger a lot of warnings and errors, please do not ignore them. [See BREAD_COMPILE_OPTIONS](../cmake/engine.cmake)
* Initializing structs requires each field to have a value.
* Braces are always separated from the line of the declaration (this applies do conditionals, functions and classes, variables, etc...):
    ```c++
    if (cond) { ... } // Bad
    if (cond) // Okey
    {
        ... Okey
    }
    ```
    
## C++(Engine):
* Constructors are only used for primitive types. A class can only have a unique constructor.
* OOP is avoided in the engine, we use a more explicit approach where paths can be easily drawn. The engine will still provide collections that let you use OOP easily.

## C++(Client):
* The project is designed to allow any approach for building your app. The runtime is built to support any tool, and the engine provides OOP classes to assist.