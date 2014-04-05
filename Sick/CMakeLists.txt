project(Sick)

################################################################################
add_definitions( -DSICKLDMRS_EXPORTS )



# ========================================
# Configure qt4
# ========================================
if(QT4_FOUND)
  set(QT_USE_QTXML true)
  set(QT_USE_QTNETWORK true)
  include(${QT_USE_FILE})
else()
  message(ERROR "Qt4 needed")
endif()

# ========================================
# Compiler definitions
# ========================================
add_definitions(
  ${QT_DEFINITIONS}
)

# ========================================
# Include directories
# ========================================
include_directories(
  ${PROJECT_BINARY_DIR}
  ${QT_INCLUDE_DIR}
)

# ========================================
# Link directories
# ========================================
link_directories( ${PACPUS_LIB_DIR}
)


pacpus_plugin(PLUGIN_CPP PLUGIN_H ${PROJECT_NAME} )

# ========================================
# List of sources
# ========================================
set(
    PROJECT_SRCS
  SickComponent.h
  SickComponent.cpp
  AbstractSickSensor.h
  AbstractSickSensor.cpp
  SickLDMRSSensor.h
  SickLDMRSSensor.cpp
  SickLMSSensor.h
  SickLMSSensor.cpp
  SickSocket.h
  SickSocket.cpp
    ${PLUGIN_CPP}
)

# ========================================
# Files to MOC
# ========================================
set(
    FILES_TO_MOC
     SickComponent.h
     SickLDMRSSensor.h
     SickLMSSensor.h
     AbstractSickSensor.h
     SickSocket.h
    ${PLUGIN_H}
)

set(
    UI_FILES

)

# ========================================
# Call MOC
# ========================================
qt4_wrap_cpp(
    PROJECT_MOC_SRCS
    ${FILES_TO_MOC}
)

qt4_wrap_ui(
    PROJECT_UI_SRCS
    ${UI_FILES}
)

# ========================================
# Build a library
# ========================================
pacpus_add_library(
    ${PROJECT_NAME} SHARED
    ${PROJECT_SRCS}
    ${PROJECT_MOC_SRCS}
    ${PROJECT_UI_SRCS}
)

message(STATUS  ${PACPUS_DEPENDENCIES_LIB} ) 

set(LIBS
    optimized FileLib debug FileLib_d
    optimized PacpusLib debug PacpusLib_d
    optimized PacpusTools debug PacpusTools_d
)
if (WIN32)
    list(APPEND LIBS
        optimized ROAD_TIME debug ROAD_TIME_d
    )
endif()

# ========================================
# Libraries
# ========================================
# All the platform
target_link_libraries(
    ${PROJECT_NAME}
    ${PACPUS_LIBRARIES}
    ${QT_LIBRARIES}
	${PACPUS_DEPENDENCIES_LIB}
	${LIBS}
)

pacpus_folder(${PROJECT_NAME} "components")

# ========================================
# Install
# ========================================
pacpus_install(${PROJECT_NAME})
