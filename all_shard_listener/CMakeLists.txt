cmake_minimum_required (VERSION 3.5)

project (SeastarExample)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

# ---- Paths ----
set(SRC_DIR     "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(INCLUDE_DIR     "${CMAKE_CURRENT_SOURCE_DIR}/src/include")


# ---- Deps ----
find_package (Seastar CONFIG REQUIRED)
message(STATUS "Seastar found: ${Seastar_DIR}")
message(STATUS "Seastar version: ${Seastar_VERSION}")

find_package(fmt 9 REQUIRED)

set(SOURCES ${SRC_DIR}/main.cc ${SRC_DIR}/service.cc)

add_executable(example ${SOURCES})

target_compile_definitions(example PRIVATE
  SEASTAR_API_LEVEL=8
  SEASTAR_SCHEDULING_GROUPS_COUNT=16
  SEASTAR_SSTRING
  SEASTAR_DEPRECATED_OSTREAM_FORMATTERS
  SEASTAR_LOGGER_COMPILE_TIME_FMT
  SEASTAR_LOGGER_TYPE_STDOUT
)

target_include_directories(example PRIVATE
  ${INCLUDE_DIR}
  ${SRC_DIR}
)

target_link_libraries (example PRIVATE Seastar::seastar fmt::fmt)
