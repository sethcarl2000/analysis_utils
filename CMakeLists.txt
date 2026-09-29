cmake_minimum_required(VERSION 3.24)

project(analysis_utils VERSION 0.1.0 LANGUAGES CXX)

# I will be totally honest with you. I have no idea what most of this does. 
# I just asked claude to write an interface-library CMake script, and this is more or less 
# what it gave me. - SEth 

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

add_library(analysis_utils INTERFACE)
add_library(analysis_utils::analysis_utils ALIAS analysis_utils)

target_include_directories(analysis_utils INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_compile_features(analysis_utils INTERFACE cxx_std_20)

install(TARGETS analysis_utils EXPORT analysis_utilsTargets)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT analysis_utilsTargets 
    FILE analysis_utilsConfig.cmake 
    NAMESPACE analysis_utils::
    DESTINATION ${CMAKE_INSTALL_DATADIR}/cmake/analysis_utils    
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/analysis_utilsConfigVersion.cmake
    COMPATIBILITY SameMajorVersion
    ARCH_INDEPENDENT
)

install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/analysis_utilsConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_DATADIR}/cmake/analysis_utils    
)