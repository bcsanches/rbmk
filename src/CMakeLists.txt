add_compile_definitions(SPDLOG_FMT_EXTERNAL)

set(MY_DEFINITION 
	$<$<CONFIG:Debug>:RBMK_DEBUG>
	$<$<CONFIG:RelWithDebInfo>:RBMK_DEBUG>
	$<$<CONFIG:Release>:RBMK_RELEASE> 
	$<$<CONFIG:MinSizeRel>:RBMK_RELEASE>
)

add_compile_definitions(${MY_DEFINITION})

add_subdirectory(Common)
add_subdirectory(Engine)