cmake_minimum_required(VERSION 3.3 FATAL_ERROR)

find_package( DD4hep REQUIRED COMPONENTS DDCore DDG4 )
#find_package(Acts REQUIRED COMPONENTS Core PluginIdentification PluginTGeo PluginDD4hep )

#-----------------------------------------------------------------------------------
set(a_lib_name my_insert)
dd4hep_add_plugin(${a_lib_name} SOURCES src/*.cpp
  )
target_link_libraries(${a_lib_name}
  PUBLIC DD4hep::DDCore  DD4hep::DDRec
  #ActsCore ActsPluginDD4hep
  )

#-----------------------------------------------------------------------------------
install(DIRECTORY compact/
    DESTINATION share/${PROJECT_NAME}/${a_lib_name}
    FILES_MATCHING PATTERN "*.xml"
    )
