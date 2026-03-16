# ---  B U I L D I N G  --- #
# ------------------------- #

# Define a macro to link core library dependencies (no benchmarking deps)
macro(link_core_dependencies target_name)
    if (TARGET OpenMP::OpenMP_CXX)
        target_link_libraries(${target_name} PUBLIC OpenMP::OpenMP_CXX)
    endif ()

    target_link_libraries(${target_name} PUBLIC ${LASLIB_LIBRARIES})
    
    # CUDA (only if CUDA is enabled)
    if(WITH_CUDA)
        target_compile_definitions(${target_name} PUBLIC HAVE_CUDA_RUNTIME=1 __CUDA_RUNTIME__)
        
        # Link CUDA runtime if available
        if(CUDAToolkit_FOUND)
            target_link_libraries(${target_name} PUBLIC CUDA::cudart)
        endif()
    endif()
endmacro()

# Compile sources once using Object Library to avoid duplication
# This is more efficient than compiling the same sources for static and shared libraries
add_library(${PROJECT_NAME}_objects OBJECT ${lib_sources})
target_include_directories(${PROJECT_NAME}_objects PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/inc)
link_core_dependencies(${PROJECT_NAME}_objects)

# Static library (uses pre-compiled objects)
add_library(${PROJECT_NAME}_static STATIC $<TARGET_OBJECTS:${PROJECT_NAME}_objects>)
link_core_dependencies(${PROJECT_NAME}_static)

# Shared library (uses pre-compiled objects)
add_library(${PROJECT_NAME}_shared SHARED $<TARGET_OBJECTS:${PROJECT_NAME}_objects>)
link_core_dependencies(${PROJECT_NAME}_shared)

# Set Link Time Optimization (LTO)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -flto=auto")

# ---  I N S T A L L A T I O N  --- #
# ---------------------------------- #

# Set default installation prefix to $HOME/.local
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "$ENV{HOME}/.local" CACHE PATH "Installation prefix" FORCE)
endif()

# Include CMakePackageConfigHelpers for configuration file generation
include(CMakePackageConfigHelpers)

# Define standard install directories
set(CMAKE_INSTALL_INCLUDEDIR "include" CACHE PATH "Include directory")
set(CMAKE_INSTALL_LIBDIR "lib" CACHE PATH "Library directory")

# Configure targets export file
export(TARGETS ${PROJECT_NAME}_static ${PROJECT_NAME}_shared
    FILE ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake
    NAMESPACE ${PROJECT_NAME}::
)

# Install libraries with export
install(TARGETS ${PROJECT_NAME}_static ${PROJECT_NAME}_shared
    EXPORT linoctreeTargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

# Install headers preserving directory structure
install(DIRECTORY inc/
    DESTINATION include/${PROJECT_NAME}
    FILES_MATCHING PATTERN "*.hpp" PATTERN "*.h"
)

# Generate and install CMake config file for find_package()
configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
    INSTALL_DESTINATION lib/cmake/${PROJECT_NAME}
    PATH_VARS CMAKE_INSTALL_INCLUDEDIR CMAKE_INSTALL_LIBDIR
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
    DESTINATION lib/cmake/${PROJECT_NAME}
)

# Export targets for use by find_package()
install(EXPORT linoctreeTargets
    FILE ${PROJECT_NAME}Targets.cmake
    NAMESPACE ${PROJECT_NAME}::
    DESTINATION lib/cmake/${PROJECT_NAME}
)

