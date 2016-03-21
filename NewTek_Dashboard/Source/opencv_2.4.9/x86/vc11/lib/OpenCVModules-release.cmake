#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
SET(CMAKE_IMPORT_FILE_VERSION 1)

# Compute the installation prefix relative to this file.
GET_FILENAME_COMPONENT(_IMPORT_PREFIX "${CMAKE_CURRENT_LIST_FILE}" PATH)
GET_FILENAME_COMPONENT(_IMPORT_PREFIX "${_IMPORT_PREFIX}" PATH)
GET_FILENAME_COMPONENT(_IMPORT_PREFIX "${_IMPORT_PREFIX}" PATH)
GET_FILENAME_COMPONENT(_IMPORT_PREFIX "${_IMPORT_PREFIX}" PATH)

# Make sure the targets which have been exported in some other 
# export set exist.

# Import target "opencv_core" for configuration "Release"
SET_PROPERTY(TARGET opencv_core APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
SET_TARGET_PROPERTIES(opencv_core PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/x86/vc11/lib/opencv_core249.lib"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/x86/vc11/bin/opencv_core249.dll"
  )

LIST(APPEND _IMPORT_CHECK_TARGETS opencv_core )
LIST(APPEND _IMPORT_CHECK_FILES_FOR_opencv_core "${_IMPORT_PREFIX}/x86/vc11/lib/opencv_core249.lib" "${_IMPORT_PREFIX}/x86/vc11/bin/opencv_core249.dll" )

# Make sure the targets which have been exported in some other 
# export set exist.

# Import target "opencv_flann" for configuration "Release"
SET_PROPERTY(TARGET opencv_flann APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
SET_TARGET_PROPERTIES(opencv_flann PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/x86/vc11/lib/opencv_flann249.lib"
  IMPORTED_LINK_INTERFACE_LIBRARIES_RELEASE "opencv_core"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/x86/vc11/bin/opencv_flann249.dll"
  )

LIST(APPEND _IMPORT_CHECK_TARGETS opencv_flann )
LIST(APPEND _IMPORT_CHECK_FILES_FOR_opencv_flann "${_IMPORT_PREFIX}/x86/vc11/lib/opencv_flann249.lib" "${_IMPORT_PREFIX}/x86/vc11/bin/opencv_flann249.dll" )

# Make sure the targets which have been exported in some other 
# export set exist.

# Import target "opencv_imgproc" for configuration "Release"
SET_PROPERTY(TARGET opencv_imgproc APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
SET_TARGET_PROPERTIES(opencv_imgproc PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/x86/vc11/lib/opencv_imgproc249.lib"
  IMPORTED_LINK_INTERFACE_LIBRARIES_RELEASE "opencv_core"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/x86/vc11/bin/opencv_imgproc249.dll"
  )

LIST(APPEND _IMPORT_CHECK_TARGETS opencv_imgproc )
LIST(APPEND _IMPORT_CHECK_FILES_FOR_opencv_imgproc "${_IMPORT_PREFIX}/x86/vc11/lib/opencv_imgproc249.lib" "${_IMPORT_PREFIX}/x86/vc11/bin/opencv_imgproc249.dll" )

# Make sure the targets which have been exported in some other 
# export set exist.

# Import target "opencv_highgui" for configuration "Release"
SET_PROPERTY(TARGET opencv_highgui APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
SET_TARGET_PROPERTIES(opencv_highgui PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/x86/vc11/lib/opencv_highgui249.lib"
  IMPORTED_LINK_DEPENDENT_LIBRARIES_RELEASE "opencv_core;opencv_imgproc"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/x86/vc11/bin/opencv_highgui249.dll"
  )

LIST(APPEND _IMPORT_CHECK_TARGETS opencv_highgui )
LIST(APPEND _IMPORT_CHECK_FILES_FOR_opencv_highgui "${_IMPORT_PREFIX}/x86/vc11/lib/opencv_highgui249.lib" "${_IMPORT_PREFIX}/x86/vc11/bin/opencv_highgui249.dll" )

# Make sure the targets which have been exported in some other 
# export set exist.

# Import target "opencv_features2d" for configuration "Release"
SET_PROPERTY(TARGET opencv_features2d APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
SET_TARGET_PROPERTIES(opencv_features2d PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/x86/vc11/lib/opencv_features2d249.lib"
  IMPORTED_LINK_INTERFACE_LIBRARIES_RELEASE "opencv_core;opencv_flann;opencv_imgproc;opencv_highgui"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/x86/vc11/bin/opencv_features2d249.dll"
  )

LIST(APPEND _IMPORT_CHECK_TARGETS opencv_features2d )
LIST(APPEND _IMPORT_CHECK_FILES_FOR_opencv_features2d "${_IMPORT_PREFIX}/x86/vc11/lib/opencv_features2d249.lib" "${_IMPORT_PREFIX}/x86/vc11/bin/opencv_features2d249.dll" )

# Make sure the targets which have been exported in some other 
# export set exist.

# Import target "opencv_calib3d" for configuration "Release"
SET_PROPERTY(TARGET opencv_calib3d APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
SET_TARGET_PROPERTIES(opencv_calib3d PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/x86/vc11/lib/opencv_calib3d249.lib"
  IMPORTED_LINK_INTERFACE_LIBRARIES_RELEASE "opencv_core;opencv_flann;opencv_imgproc;opencv_highgui;opencv_features2d"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/x86/vc11/bin/opencv_calib3d249.dll"
  )

LIST(APPEND _IMPORT_CHECK_TARGETS opencv_calib3d )
LIST(APPEND _IMPORT_CHECK_FILES_FOR_opencv_calib3d "${_IMPORT_PREFIX}/x86/vc11/lib/opencv_calib3d249.lib" "${_IMPORT_PREFIX}/x86/vc11/bin/opencv_calib3d249.dll" )


# Loop over all imported files and verify that they actually exist
FOREACH(target ${_IMPORT_CHECK_TARGETS} )
  FOREACH(file ${_IMPORT_CHECK_FILES_FOR_${target}} )
    IF(NOT EXISTS "${file}" )
      MESSAGE(FATAL_ERROR "The imported target \"${target}\" references the file
   \"${file}\"
but this file does not exist.  Possible reasons include:
* The file was deleted, renamed, or moved to another location.
* An install or uninstall procedure did not complete successfully.
* The installation package was faulty and contained
   \"${CMAKE_CURRENT_LIST_FILE}\"
but not all the files it references.
")
    ENDIF()
  ENDFOREACH()
  UNSET(_IMPORT_CHECK_FILES_FOR_${target})
ENDFOREACH()
UNSET(_IMPORT_CHECK_TARGETS)

# Cleanup temporary variables.
SET(_IMPORT_PREFIX)

# Commands beyond this point should not need to know the version.
SET(CMAKE_IMPORT_FILE_VERSION)
