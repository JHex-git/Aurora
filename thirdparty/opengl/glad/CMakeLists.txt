set(SRCS glad.c)

if(MINGW)
    add_library(glad SHARED ${SRCS})
elseif(MSVC)
    add_library(glad STATIC ${SRCS})
endif()