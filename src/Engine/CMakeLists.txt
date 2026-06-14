
set(Engine_SRCS
	Camera.cpp
	Camera.h	
	main.cpp
	Model.cpp
	Model.h
	RenderSystem.h
	RenderSystemRT.cpp
	RenderSystemRT.h
	RenderWorld.cpp
	RenderWorld.h
)

add_executable(Engine  ${Engine_SRCS})

set_property(TARGET Engine PROPERTY WIN32_EXECUTABLE TRUE)

target_link_libraries(
	Engine
	PRIVATE Common
	PRIVATE SDL3::SDL3
)

target_include_directories(
	Engine
	
	PRIVATE ${RBMK_SOURCE_DIR}/contrib/spdlog
	PUBLIC ${RBMK_SOURCE_DIR}/src/Common
)


