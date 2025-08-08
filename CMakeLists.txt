cmake_minimum_required(VERSION 3.29)

project(walletcore VERSION 1.0)
set(CMAKE_CXX_STANDARD 20)

set(SECP256K1_DISABLE_SHARED ON CACHE BOOL "" FORCE)
set(SECP256K1_ENABLE_MODULE_ECDH OFF CACHE BOOL "" FORCE)
set(SECP256K1_ENABLE_MODULE_RECOVERY ON CACHE BOOL "" FORCE)
set(SECP256K1_ENABLE_MODULE_MUSIG OFF CACHE BOOL "" FORCE)
set(SECP256K1_BUILD_BENCHMARK OFF CACHE BOOL "" FORCE)
set(SECP256K1_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(SECP256K1_BUILD_EXHAUSTIVE_TESTS OFF CACHE BOOL "" FORCE)
set(SECP256K1_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SECP256K1_BUILD_CTIME_TESTS OFF CACHE BOOL "" FORCE)

add_subdirectory(secp256k1)

option(BUILD_JNI_LIB "Target JNI" OFF)
add_library(${PROJECT_NAME}_headers INTERFACE)

target_include_directories(
  ${PROJECT_NAME}_headers
  INTERFACE
  "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
)


file(GLOB_RECURSE core_sources src/*.h src/*.cpp src/*.c)

if(BUILD_JNI_LIB)
  file(GLOB_RECURSE jni_sources jni/*.h jni/*.cpp)
  set(sources ${core_sources} ${jni_sources})
  add_library(${PROJECT_NAME} SHARED ${sources})
  find_package(JNI REQUIRED)
  target_include_directories(${PROJECT_NAME} PRIVATE ${JNI_INCLUDE_DIRS})
else()
  set(sources ${core_sources})
  add_library(${PROJECT_NAME} STATIC ${sources})
endif()

target_include_directories(
  ${PROJECT_NAME}
  PRIVATE
  "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>"
)
target_link_libraries(${PROJECT_NAME} PUBLIC ${PROJECT_NAME}_headers secp256k1)

if (MSVC)
  add_compile_options(/utf-8)
endif()

#option(BUILD_EXAMPLE "build example" OFF)

if( NOT BUILD_JNI_LIB)
  find_package(OpenSSL REQUIRED)
  add_executable(example examples/cpp/main.cpp)
  target_link_libraries(example PRIVATE ${PROJECT_NAME} OpenSSL::Crypto)
endif()
