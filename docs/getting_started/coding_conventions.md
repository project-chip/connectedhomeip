# Coding Conventions

-   c++17
-   single-threaded event loop
-   no self-expanding std:: in the embedded core
    -   allowed for certain pieces, examples, controller (non-embedded), but be
        careful
    -   Generally no heap (no std::string, std::function, std::list, ….).
        std::array is ok
-   src/lib/support for embedded friendly wrappers
-   Delegate layer to hook the lower and platform levels
    -   most platforms have this implemented already, but you will need to
        implement this if you are porting to a new platform
-   consider using delegate layers for cluster hooks as well
