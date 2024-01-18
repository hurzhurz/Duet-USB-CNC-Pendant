Import("env")
from sys import platform

# workaround for https://github.com/hurzhurz/Duet-USB-CNC-Pendant/issues/1

if platform == "linux":
    env.Append(CPPPATH=[env["PROJECT_DIR"]+"/linux-compile-workaround/include"])