
# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Find Boost libraries
find_package(Boost REQUIRED COMPONENTS program_options json system)

# Include Boost headers
include_directories(${Boost_INCLUDE_DIRS})


add_executable(Bridge 
    CVirtualNic.h 
    CVirtualNic.cpp 
    Relay.h 
    Relay.cpp 
    CSocket.h 
    CSocket.cpp 
    main.cpp 
    ServerSide.h 
    ServerSide.cpp
    ClientSide.h
    ClientSide.cpp
    )
# Link Boost libraries
target_link_libraries(Bridge ${Boost_LIBRARIES} pthread Boost::program_options Boost::json Boost::system)
target_precompile_headers(Bridge PRIVATE stdafx.h)
