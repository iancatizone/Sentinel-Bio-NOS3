project(sentinelbio_flatsat_eps_sim)

find_package(Boost REQUIRED QUIET COMPONENTS system program_options filesystem)
find_package(ITC_Common REQUIRED QUIET COMPONENTS itc_logger)
find_package(NOSENGINE REQUIRED QUIET COMPONENTS common transport client i2c)

include_directories(inc
                    ${sim_common_SOURCE_DIR}/inc
                    ${ITC_Common_INCLUDE_DIRS}
                    ${NOSENGINE_INCLUDE_DIRS})

set(sentinelbio_flatsat_eps_sim_src
    src/sentinelbio_flatsat_eps_hardware_model.cpp
)

# For Code::Blocks and other IDEs
file(GLOB sentinelbio_flatsat_eps_sim_inc inc/*.hpp)

set(sentinelbio_flatsat_eps_sim_libs
    sim_common
    ${Boost_LIBRARIES}
    ${ITC_Common_LIBRARIES}
    ${NOSENGINE_LIBRARIES}
)

set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_RPATH}:$ORIGIN/../lib") # Pick up .so in install directory

add_library(sentinelbio_flatsat_eps_sim SHARED ${sentinelbio_flatsat_eps_sim_src} ${sentinelbio_flatsat_eps_sim_inc})
set_target_properties(sentinelbio_flatsat_eps_sim PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED YES)
target_link_libraries(sentinelbio_flatsat_eps_sim ${sentinelbio_flatsat_eps_sim_libs})
install(TARGETS sentinelbio_flatsat_eps_sim LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
