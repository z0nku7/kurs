find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network)

set(CLIENT_SOURCES
    main.cpp
    NetworkClient.h
    NetworkClient.cpp
    view/LoginDialog.h
    view/LoginDialog.cpp
    view/ProductView.h
    view/ProductView.cpp
    view/OrderView.h
    view/OrderView.cpp
    view/CustomerView.h
    view/CustomerView.cpp
    view/DeliveryView.h
    view/DeliveryView.cpp
    view/MainWindow.h
    view/MainWindow.cpp
    view/dialogs/ProductEditDialog.h
    view/dialogs/ProductEditDialog.cpp
    view/dialogs/ProfileDialog.h
    view/dialogs/ProfileDialog.cpp
)

add_executable(OrderClient ${CLIENT_SOURCES})

target_include_directories(OrderClient PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/view
    ${CMAKE_CURRENT_SOURCE_DIR}/view/dialogs
)

target_link_libraries(OrderClient PRIVATE
    common
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
)
