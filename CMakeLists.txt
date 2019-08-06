project(kvsvr)
cmake_minimum_required(VERSION 3.5)
add_compile_options(-std=c++11)

set(LIBS_SRC base logger thread_pool parameters net)
include_directories(${LIBS_SRC})

find_package(Threads REQUIRED)

add_subdirectory(base)
add_subdirectory(logger)
add_subdirectory(thread_pool)
add_subdirectory(parameters)
add_subdirectory(net)

add_library(BTree BTree_index.cpp)
add_library(LRU LRU.cpp)
add_library(SimpleKVIndex SimpleKVIndex.cpp)
#add_library(SimpleKVStore SimpleKVStore.cpp)
add_library(kvsvr_pool kvsvr_pool.cpp)


set(BASE_LIBS
  logger
  parameters
  TcpServer
  thread_pool
  work_thread
  my_condition
  my_thread
)

set(KVSVR_LIBS
  BTree
  LRU
  SimpleKVIndex
  #SimpleKVStore
  kvsvr_pool 
)



add_executable(BTree_test test/BTree_test.cpp)
target_link_libraries(BTree_test BTree)

add_executable(BTree_index_test test/BTree_index_test.cpp)
target_link_libraries(BTree_index_test BTree)

add_executable(lru_test test/lru_test.cpp)
target_link_libraries(lru_test LRU)

add_executable(SimpleKVIndex_test test/SimpleKVIndex_test)
target_link_libraries(SimpleKVIndex_test SimpleKVIndex BTree LRU my_thread logger) 

add_executable(kvsvr_pool_test test/kvsvr_pool_test.cpp)
target_link_libraries(kvsvr_pool_test ${CMAKE_THREAD_LIBS_INIT} ${KVSVR_LIBS} ${BASE_LIBS} /usr/local/lib/libboost_system.a /usr/local/lib/libboost_thread.a)



