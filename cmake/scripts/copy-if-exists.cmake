# Copy files if they exist (silent skip if not found)
# Usage: cmake -DSRC_DIR=<dir> -DDST_DIR=<dir> -DFILES=<file1;file2;...> -P copy-if-exists.cmake

if(NOT DEFINED SRC_DIR OR NOT DEFINED DST_DIR OR NOT DEFINED FILES)
    message(FATAL_ERROR "Usage: cmake -DSRC_DIR=<dir> -DDST_DIR=<dir> -DFILES=<file1;file2;...> -P copy-if-exists.cmake")
endif()

foreach(FILE ${FILES})
    set(SRC_PATH "${SRC_DIR}/${FILE}")
    if(EXISTS "${SRC_PATH}")
        file(COPY "${SRC_PATH}" DESTINATION "${DST_DIR}")
        message(STATUS "Copied: ${FILE}")
    else()
        message(STATUS "Skipped (not found): ${FILE}")
    endif()
endforeach()
