
set(Engine_SRCS
	main.cpp
)

add_executable(Engine  ${Engine_SRCS})

set_property(TARGET Engine PROPERTY WIN32_EXECUTABLE TRUE)

target_link_libraries(
	Engine
	PRIVATE SDL3::SDL3
)

target_include_directories(
	Engine
	
	PRIVATE ${RBMK_SOURCE_DIR}/contrib/spdlog
)


