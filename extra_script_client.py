Import("env")

import os
project_package_dir = env.subst('$PROJECT_PACKAGES_DIR')
# Ensure the correct libraries and include paths are used
print(os.path.join(project_package_dir, "tool-simavr", "include", "simavr").replace("\\", "/"))
env.Append(
    CPPPATH=[os.path.join(project_package_dir, "tool-simavr", "include", "simavr").replace("\\", "/")],
    CPPPATH=[os.path.join(project_package_dir, "tool-simavr", "include", "SDL2").replace("\\", "/")],
    LIBPATH=[os.path.join(project_package_dir, "tool-simavr", "lib").replace("\\", "/")],
    LIBS=["elf", "simavr", "SDL2"]
)
