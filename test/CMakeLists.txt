# Tests are written using the C++ Template Unit Test Framework (TUT)
# http://tut-framework.sourceforge.net/

set(TUT_URL http://sourceforge.net/projects/tut-framework/files/TUT/TUT-2009-08-30/TUT-2009-08-30.tar.gz)
set(TUT_DOWNLOAD_PATH "${PROJECT_BINARY_DIR}/tut-2009-08-30.tar.gz")
set(TUT_INCLUDE "${PROJECT_BINARY_DIR}/tut-2009-08-30")

if (NOT EXISTS "${TUT_DOWNLOAD_PATH}")
    message(STATUS "Downloading C++ Template Unit Test Framework (TUT)")
    file(DOWNLOAD "${TUT_URL}" "${TUT_DOWNLOAD_PATH}")
    execute_process(
        COMMAND cmake -E tar xzf "${TUT_DOWNLOAD_PATH}"
        WORKING_DIRECTORY "${PROJECT_BINARY_DIR}"
    )
endif()

add_subdirectory(src)
