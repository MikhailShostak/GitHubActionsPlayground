set(SOURCES
    Memory/InplaceStorage.hpp
    DateTime/Counter.hpp
    DateTime/Date.hpp
    DateTime/Duration.hpp
    DateTime/Stopwatch.hpp
    DateTime/Time.hpp
    DateTime/Timer.hpp
    Meta/Aliases.hpp
    Meta/Inheritance.hpp
    Meta/Substitution.hpp
    Reflection/Reflection.hpp
    Serialization/Serialization.hpp
    Serialization/Serializer.hpp
    Serialization/Storage.hpp
    Utils.hpp
)

add_library(Utils INTERFACE ${SOURCES})
target_include_directories(Utils INTERFACE .)
target_link_libraries(Utils INTERFACE ThirdParty)
