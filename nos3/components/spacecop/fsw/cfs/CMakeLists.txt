cmake_minimum_required(VERSION 2.6.4)
project(CFS_SPACECOP_APP C)

include_directories(public_inc)
include_directories(mission_inc)
include_directories(platform_inc)
include_directories(src)

include_directories(${APPLICATION_PLATFORM_INC_LIST})

foreach(EXT_APP time)
  list(FIND TGTSYS_${SYSVAR}_APPS ${EXT_APP} HAVE_APP)
  if(HAVE_APP GREATER_EQUAL 0)
    target_link_libraries(spacecop PRIVATE ${EXT_APP})
    string(TOUPPER "HAVE_${EXT_APP}" APP_MACRO)
    target_compile_definitions(spacecop PRIVATE ${APP_MACRO})
  endif()
endforeach()

aux_source_directory(src APP_SRC_FILES)

aux_source_directory(table APP_TABLE_FILES)

add_cfe_tables(spacecop ${APP_TABLE_FILES})

# Create the app module
add_cfe_app(spacecop ${APP_SRC_FILES})
