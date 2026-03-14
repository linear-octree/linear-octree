# ---- CUDA ---- #
# -------------- #

# Detect CUDA from conda environment or system
set(CUDA_TOOLKIT_ROOT_DIR)
set(CUDA_SEARCH_PATHS)

# Check if running in conda environment
if(DEFINED ENV{CONDA_PREFIX})
    set(CONDA_ENV_PATH "$ENV{CONDA_PREFIX}")
    message(STATUS "Conda environment detected: ${CONDA_ENV_PATH}")
    
    # CUDA is typically installed in conda env under targets/x86_64-linux or directly
    list(APPEND CUDA_SEARCH_PATHS
        "${CONDA_ENV_PATH}/targets/x86_64-linux"
        "${CONDA_ENV_PATH}"
    )
endif()

# Also check system paths as fallback
list(APPEND CUDA_SEARCH_PATHS
    "/opt/cuda"
    "/usr/local/cuda"
    "${CUDA_TOOLKIT_ROOT_DIR}"
)

# Try to find CUDA in the search paths
foreach(CUDA_PATH ${CUDA_SEARCH_PATHS})
    if(EXISTS "${CUDA_PATH}/include/cuda_runtime.h" OR 
       EXISTS "${CUDA_PATH}/targets/x86_64-linux/include/cuda_runtime.h")
        set(CUDA_TOOLKIT_ROOT_DIR "${CUDA_PATH}")
        message(STATUS "CUDA toolkit root found at: ${CUDA_TOOLKIT_ROOT_DIR}")
        break()
    endif()
endforeach()

# Set CMAKE_PREFIX_PATH to include CUDA toolkit for find_package
if(CUDA_TOOLKIT_ROOT_DIR)
    list(PREPEND CMAKE_PREFIX_PATH "${CUDA_TOOLKIT_ROOT_DIR}")
    if(EXISTS "${CUDA_TOOLKIT_ROOT_DIR}/targets/x86_64-linux")
        list(PREPEND CMAKE_PREFIX_PATH "${CUDA_TOOLKIT_ROOT_DIR}/targets/x86_64-linux")
    endif()
endif()

# Find CUDAToolkit package (modern approach, replaces deprecated FindCUDA)
find_package(CUDAToolkit 12.0 QUIET)

if(CUDAToolkit_FOUND)
    # CUDA toolkit found successfully
    message("CUDA ${CUDAToolkit_VERSION} found at ${CUDA_TOOLKIT_ROOT_DIR}")
    set(HAVE_CUDA ON)
    
    # Include CUDA headers globally
    include_directories(${CUDAToolkit_INCLUDE_DIRS})
    
    # Extract CUDA library directory for explicit linking
    get_target_property(CUDART_LIBRARY CUDA::cudart IMPORTED_LOCATION)
    if(CUDART_LIBRARY)
        get_filename_component(CUDA_LIB_DIR "${CUDART_LIBRARY}" DIRECTORY)
        message(STATUS "CUDA runtime library directory: ${CUDA_LIB_DIR}")
        # Add to link search path
        link_directories(${CUDA_LIB_DIR})
    endif()
    
    # Detect GPU model and driver version from nvidia-smi
    set(GPU_NAME "Unknown GPU")
    set(GPU_DRIVER_VERSION "Unknown Driver")
    set(GPU_COMPUTE_CAPABILITY "Unknown")
    
    # Try to query GPU information from nvidia-smi
    if(UNIX AND NOT APPLE)
        # Query GPU name
        execute_process(
            COMMAND bash -c "which nvidia-smi > /dev/null 2>&1 && nvidia-smi --query-gpu=name --format=csv,noheader || echo 'nvidia-smi not found'"
            OUTPUT_VARIABLE GPU_NAME_OUTPUT
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
        )
        if(GPU_NAME_OUTPUT AND NOT GPU_NAME_OUTPUT MATCHES "nvidia-smi not found")
            set(GPU_NAME "${GPU_NAME_OUTPUT}")
        endif()
        
        # Query driver version
        execute_process(
            COMMAND bash -c "which nvidia-smi > /dev/null 2>&1 && nvidia-smi --query-gpu=driver_version --format=csv,noheader || echo 'N/A'"
            OUTPUT_VARIABLE GPU_DRIVER_OUTPUT
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
        )
        if(GPU_DRIVER_OUTPUT AND NOT GPU_DRIVER_OUTPUT MATCHES "N/A")
            set(GPU_DRIVER_VERSION "${GPU_DRIVER_OUTPUT}")
        endif()
        
        # Query compute capability
        execute_process(
            COMMAND bash -c "which nvidia-smi > /dev/null 2>&1 && nvidia-smi --query-gpu=compute_cap --format=csv,noheader || echo 'N/A'"
            OUTPUT_VARIABLE GPU_COMPUTE_OUTPUT
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
        )
        if(GPU_COMPUTE_OUTPUT AND NOT GPU_COMPUTE_OUTPUT MATCHES "N/A")
            set(GPU_COMPUTE_CAPABILITY "${GPU_COMPUTE_OUTPUT}")
        endif()
    endif()
    
    message(STATUS "  GPU: ${GPU_NAME} (Compute Capability ${GPU_COMPUTE_CAPABILITY})")
    message(STATUS "  Driver: ${GPU_DRIVER_VERSION}")
    
    # Note: We DON'T enable CUDA language to avoid GCC/CLANG incompatibility
    # Instead, we link against CUDA libraries directly via CUDA::cudart target
else()
    # CUDA not found - build CPU-only version
    message("CUDA not found - Building CPU-only version")
    set(HAVE_CUDA OFF)
endif()
