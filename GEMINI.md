# Layout
- The src/, Game/ and Game/cpp directories contain C++ code for integrating the Flecs ECS library with Godot, the Godot project, and game-specific ECS code, respectively. Don't modify files inside flecs/ or godot-cpp/.
- Refer to the .md files in flecs/docs the directory, and the .h & .cpp files in flecs/examples/cpp the directory for code samples that demonstrate Flecs API usage patterns.
- Don't modify files other than those with .h, .cpp, .gd and .gdshader extensions. When changes or validations on other files are needed, give me instructions instead.

# Rules
- Use descriptive names for identifiers for variables and similar constructs over short ones.
- Use explicit type names instead of `auto`.
- Use single-line blocks when the total line length is less than 120.
- Add code comments sparingly, only when the code is not self-explanatory.
- Don't write defensive code (checks etc.) unless really necessary.

# Validation
- After you make modifications, run the build via `scons` to verify that the project compiles successfully.
