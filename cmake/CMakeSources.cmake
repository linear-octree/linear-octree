# ---  S O U R C E S  --- #
# ----------------------- #

# Gather all source files
file(GLOB_RECURSE sources CONFIGURE_DEPENDS src/*.cpp)

set(lib_sources ${sources})

# Include directories
include_directories(
        "inc/"
        "src/readers"
        )