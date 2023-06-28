cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

PROJECT(eic_endcap_insert
  LANGUAGES CXX
  )

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")

find_package(DD4hep REQUIRED COMPONENTS DDCore DDG4)

#-----------------------------------------------------------------------------------
dd4hep_configure_output()

dd4hep_add_plugin(${PROJECT_NAME} SOURCES src/*.cpp
  )
target_link_libraries(${PROJECT_NAME}
  PUBLIC DD4hep::DDCore  DD4hep::DDRec
  )

#-----------------------------------------------------------------------------------
# Moving compact, scripts, and hepmc_generation to install share directory
install(DIRECTORY compact scripts hepmc_generation ip6
    DESTINATION share/${PROJECT_NAME}
    FILE_PERMISSIONS OWNER_READ OWNER_EXECUTE OWNER_WRITE
    )
# Moving main XML file to install share directory
INSTALL(FILES insert_setup.sh
    DESTINATION .
    )
# Moving main XML file to install share directory
INSTALL(FILES endcapP_insert.xml
    DESTINATION share/${PROJECT_NAME}
    )