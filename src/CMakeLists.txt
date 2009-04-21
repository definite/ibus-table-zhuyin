# Location of library include files

# Library location for the linker

#==================================================================
# Sources
#

# executable built from some source
ADD_EXECUTABLE(cin2ibus cin2ibus.c
    )

# Link the executable to the library.

IF(NOT DEFINED LIB_INSTALL_DIR)
    IF( $ENV{MACHTYPE} MATCHES "64")
	SET(LIB_INSTALL_DIR lib64)
	ADD_DEFINITIONS(-DLIB_INSTALL_DIR='"${LIB_INSTALL_DIR}"')
    ELSE()
	SET(LIB_INSTALL_DIR lib)
	ADD_DEFINITIONS(-DLIB_INSTALL_DIR='"${LIB_INSTALL_DIR}"')
    ENDIF()
ENDIF()

#INSTALL(TARGETS cin2ibus
#     RUNTIME DESTINATION bin
#     LIBRARY DESTINATION ${LIB_INSTALL_DIR}
#)

