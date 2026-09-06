# AI Agents

This document defines how you must work.

* [Documentation](docs/main.md): This path contains documentation related with the entire project.
* Do not use the STL. Any exception must be in the same file where it is used.
* Prefer a single visible path.
* Do not add dependencies. Only platform libraries are allowed (vulkan, win32, x11, etc...).
* Classes must have only one constructor.
* Prioritize consistency across platforms. If something can only be proven on one platform, it has not been proven.
* The arguments of a public interface must specify their limits and, in debug mode, verify them.
* Allocating memory must be explicit. If something allocates memory, its owner must be clarified early.
