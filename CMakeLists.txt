cmake_minimum_required(VERSION 3.12)


if (POLICY CMP0025)
  cmake_policy(SET CMP0025 NEW)
endif ()


project("Metrics"
	VERSION 0.0.1
	DESCRIPTION "A header only c++ library that manage metrics type"
	HOMEPAGE_URL "https://github.com/xtofpic/metrics")



include(GNUInstallDirs)
# include(CMakePackageConfigHelpers)
include(CTest)


option(CATCH_ENABLE_COVERAGE "Generate coverage for codecov.io" ON)



# set_property(GLOBAL PROPERTY USE_FOLDERS ON)


add_library(${PROJECT_NAME} INTERFACE)
add_subdirectory(tests)
enable_testing ()
add_test (NAME TestAll COMMAND TestAll)

target_include_directories(
	${PROJECT_NAME}
	INTERFACE	$<BUILD_INTERFACE:${${PROJECT_NAME}_SOURCE_DIR}/include>
			$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)


# set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)
# target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_17)


install(TARGETS ${PROJECT_NAME}
        EXPORT ${PROJECT_NAME}_Targets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})


include(CMakePackageConfigHelpers)
write_basic_package_version_file(
	"${PROJECT_NAME}ConfigVersion.cmake"
	VERSION ${PROJECT_VERSION}
	COMPATIBILITY SameMajorVersion)


configure_package_config_file(
	"${PROJECT_SOURCE_DIR}/CMake/${PROJECT_NAME}Config.cmake.in"
	"${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
	INSTALL_DESTINATION
	${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake)


install(
	EXPORT ${PROJECT_NAME}_Targets
	FILE ${PROJECT_NAME}Targets.cmake
	NAMESPACE ${PROJECT_NAME}::
	DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake)


install(
	FILES	"${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
		"${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
	DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake)


install(DIRECTORY ${PROJECT_SOURCE_DIR}/include DESTINATION include)
