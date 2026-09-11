find_package(Qt6 REQUIRED COMPONENTS Core)

add_library(common STATIC
    Protocol.h
    Role.h
    User.h
    Customer.h
    Product.h
    DeliveryMethod.h
    DeliveryTariff.h
    Order.h
    OrderItem.h
)

set_target_properties(common PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(common PUBLIC Qt6::Core)
