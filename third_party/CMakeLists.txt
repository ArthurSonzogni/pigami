include(FetchContent)

#set(FETCHCONTENT_UPDATES_DISCONNECTED TRUE)
FetchContent_Declare(smk
  GIT_REPOSITORY https://github.com/ArthurSonzogni/smk
  GIT_TAG b7388f980c9237794a885bbb397e151d000f3108
)

FetchContent_GetProperties(smk)
if(NOT smk_POPULATED)
  FetchContent_Populate(smk)
  add_subdirectory(${smk_SOURCE_DIR} ${smk_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()
