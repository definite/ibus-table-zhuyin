cmake_minimum_required(VERSION 2.4)
####################################################################
# Init Definition
####################################################################
INCLUDE(init.CMakeLists.txt)

####################################################################
# Project basic information
####################################################################
PROJECT(ibus-table-zhuyin)
SET(CMAKE_C_FLAGS "-Wall")

SET(RELEASE_FILE ${CMAKE_SOURCE_DIR}/RELEASE-NOTES.txt)
SET(RPM_RELEASE_FILE ${CMAKE_SOURCE_DIR}/SPECS/RPM-RELEASE-NOTES.txt)
SETTING_FILE_GET_ATTRIBUTE(PRJ_VER_PATTERN "PRJ_VER" ${RELEASE_FILE})
#MESSAGE("PRJ_VER_PATTERN=${PRJ_VER_PATTERN}")
SETTING_FILE_GET_ATTRIBUTE(PRJ_VER_RELEASE "RPM_RELEASE" ${RPM_RELEASE_FILE})

SET(SO_VER_MAJOR "1")
SET(SO_VER_MINOR "0")
SET(AUTHORS "Ding-Yi Chen")
SET(MAINTAINER "Ding-Yi Chen <dchen at redhat.com>")

# For CVS/SVN/GIT change message
SETTING_FILE_GET_ATTRIBUTE(CHANGE_SUMMARY "SUMMARY" ${RELEASE_FILE})
#MESSAGE("### CHANGE_SUMMARY=${CHANGE_SUMMARY}")

# Changelog items
EXECUTE_PROCESS(
    COMMAND tail -n +5 ${RELEASE_FILE}
    OUTPUT_VARIABLE CHANGELOG_ITEMS
    )
#MESSAGE("### CHANGELOG_ITEMS=${CHANGELOG_ITEMS}")

STRING(COMPARE GREATER "${PRJ_VER_RELEASE}" "1" NEED_RPM_RELEASE_INFO)
IF(${NEED_RPM_RELEASE_INFO})
    SETTING_FILE_GET_ATTRIBUTE(RPM_RELEASE_SUMMARY "RPM_RELEASE_SUMMARY" ${RPM_RELEASE_FILE})
    EXECUTE_PROCESS(
	COMMAND tail -n +5 ${RPM_RELEASE_FILE}
	OUTPUT_VARIABLE RPM_CHANGELOG_ITEMS
	)
ELSE(${NEED_RPM_RELEASE_INFO})
    SET(RPM_RELEASE_SUMMARY ${CHANGE_SUMMARY})
    SET(RPM_CHANGELOG_ITEMS ${CHANGELOG_ITEMS})
ENDIF(${NEED_RPM_RELEASE_INFO})
#MESSAGE("### RPM_RELEASE_SUMMARY=${RPM_RELEASE_SUMMARY}")
#MESSAGE("### RPM_CHANGELOG_ITEMS=|${RPM_CHANGELOG_ITEMS}|")

#SET(CVS_DIST_TAGS F-10)

SET(PROJECT_DATADIR "${datadir}/ibus-table")

SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "ibus-table-zhuyin: Plain Zhuyin Chinese input method.")
SET(CPACK_PACKAGE_VENDOR "Red Hat, APAC, Inc.")
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README")
SET(CPACK_RESOURCE_FILE_LICENSE
    ${CMAKE_CURRENT_SOURCE_DIR}/COPYING)


####################################################################
# Includes CMake modules
####################################################################

#==================================================================
# Include common.CMakeLists.txt
#
INCLUDE(common.CMakeLists.txt)

#====================================================================
# RPM configuration
#
FILE(READ ${CMAKE_SOURCE_DIR}/SPECS/RPM-ChangeLog RPM_CHANGELOG)
GENERATE_SPEC(SPECS/${PROJECT_NAME}.spec.in)


####################################################################
# Definitions
####################################################################

####################################################################
# Required packages
####################################################################
FIND_PROGRAM(IBUS_TABLE_CREATEDB ibus-table-createdb)

IF(${IBUS_TABLE_CREATEDB} STREQUAL "IBUS_TABLE_CREATEDB-NOTFOUND")
    MESSAGE(FATAL_ERROR "ibus_table_createdb not found, please install ibus-table.")
ENDIF()



#==================================================================
# Source code subdirectories
#
ADD_SUBDIRECTORY(src bin)

#====================================================================
# CPack File ignore list
#

SET(PRJ_COMMON_IGNORE_FILES
    "/docs/"
    "\\\\.cache$"  "\\\\.spec$"
)

SET(CPACK_SOURCE_IGNORE_FILES ${CPACK_SOURCE_IGNORE_FILES} ${PRJ_COMMON_IGNORE_FILES} "/bin/"
    )

SET(CPACK_PACKAGE_IGNORE_FILES ${CPACK_BINARY_IGNORE_FILES} ${PRJ_COMMON_IGNORE_FILES} "/src/"
    "\\\\.spec$" "\\\\.in$")

INCLUDE(CPack)

#====================================================================
# Additional targets
#

SET(CIN_FILE ${CMAKE_SOURCE_DIR}/data/phone.cin)
SET(TEMPLATE_IN_FILE ${CMAKE_SOURCE_DIR}/data/zhuyin.txt.in)
SET(TEMPLATE_FILE ${CMAKE_BINARY_DIR}/data/zhuyin.txt)
SET(DB_FILE  ${CMAKE_BINARY_DIR}/data/zhuyin.db)

ADD_CUSTOM_COMMAND(OUTPUT ${TEMPLATE_FILE}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/data
    COMMAND ${CMAKE_BINARY_DIR}/bin/cin2ibus -o ${TEMPLATE_FILE}
        ${TEMPLATE_IN_FILE} ${CIN_FILE}
    DEPENDS ${TEMPLATE_IN_FILE}  ${CIN_FILE}
    )

ADD_CUSTOM_COMMAND(OUTPUT ${DB_FILE}
    COMMAND ${IBUS_TABLE_CREATEDB} -o -n ${DB_FILE} -s ${TEMPLATE_FILE}
    DEPENDS ${TEMPLATE_FILE}
    )

ADD_CUSTOM_TARGET(table_db ALL
    DEPENDS ${DB_FILE}
    )

ADD_CUSTOM_TARGET(install_post
    COMMAND ${IBUS_TABLE_CREATEDB} -i -n ${PROJECT_DATADIR}/tables/zhuyin.db
    DEPENDS ${PROJECT_DATADIR}/tables/zhuyin.db
    )

SET_SOURCE_FILES_PROPERTIES(${TEMPLATE_FILE}
    PROPERTIES GENERATED TRUE
    OBJECT_DEPENDS (cin2ibus)
    )

ADD_DEPENDENCIES(table_db cin2ibus)

#====================================================================
# Files to be install.
#
INSTALL(DIRECTORY icons
    DESTINATION  ${PROJECT_DATADIR})

INSTALL(FILES ${DB_FILE}
    DESTINATION ${PROJECT_DATADIR}/tables)


