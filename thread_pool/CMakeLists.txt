
add_library(thread_pool thread_pool.cpp)
target_link_libraries(thread_pool logger)
add_library(work_thread work_thread.cpp)

