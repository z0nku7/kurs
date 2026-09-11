find_package(Qt6 REQUIRED COMPONENTS Core Network Sql)

set(SERVER_SOURCES
    main.cpp
    Server.h
    Server.cpp
    ClientHandler.h
    ClientHandler.cpp
    DatabaseManager.h
    DatabaseManager.cpp
)

add_executable(OrderServer ${SERVER_SOURCES})

target_include_directories(OrderServer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(OrderServer PRIVATE
    common
    Qt6::Core
    Qt6::Network
    Qt6::Sql
)
