
file(GLOB_RECURSE SRC_MATH
    "Math/*.cpp"
    "Math/*.h"
)

set(CommonLib_HDRS
	rbTypes.h
	Math/rbAngle.h
    Math/rbUtils.h
	Math/rbVec3.h
)

set(CommonLib_SRCS
	rbTypes.cpp
)

source_group("Math"          FILES ${SRC_MATH})
source_group(/ ...) 

add_library(Common STATIC ${CommonLib_HDRS} ${CommonLib_SRCS})

target_include_directories(Common	
	PUBLIC ${RTool_SOURCE_DIR}/contrib/fmt/include
	PUBLIC ${RTool_SOURCE_DIR}/contrib/rapidjson/include
    PUBLIC ${RTool_SOURCE_DIR}/contrib/spdlog/include	
	PUBLIC ${RTool_SOURCE_DIR}/contrib/magic_enum/include
)

