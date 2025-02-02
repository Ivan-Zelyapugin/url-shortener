cmake_minimum_required(VERSION 3.10)

# Название проекта
project(test_task)

# Устанавливаем стандарт C++17 для всего проекта
set(CMAKE_CXX_STANDARD 17)
# Обязательное использование стандарта C++17
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Пути к исходным и заголовочным файлам
set(SOURCE_DIR "${CMAKE_SOURCE_DIR}/src")
set(INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include")

# Поиск всех файлов .cpp в папке src
file(GLOB SOURCES "${SOURCE_DIR}/*.cpp")

# Указание пути к заголовочным файлам (добавляем директорию include)
include_directories(${INCLUDE_DIR})

# Поиск библиотеки nlohmann_json
find_package(nlohmann_json 3.2.0 REQUIRED)
# Поиск библиотеки Boost
find_package(Boost 1.71 REQUIRED COMPONENTS system filesystem)

if (Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
else()
    message(FATAL_ERROR "Boost not found!")
endif()

# Добавляем флаги для линковки с библиотеками libpqxx и libpq
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lpqxx -lpq")

# Указываем путь к заголовочным файлам библиотеки pqxx
set(PQXX /usr/local/include/pqxx)

# Ищем библиотеки pqxx и pq для линковки
find_library(PQXX_LIB pqxx)
find_library(PQ_LIB pq)

# Добавление исполняемого файла
add_executable(test_task ${SOURCES})

# Линковка с библиотекой nlohmann_json
target_link_libraries(test_task PRIVATE nlohmann_json::nlohmann_json)

# Линковка с библиотеками Boost
target_link_libraries(test_task PRIVATE Boost::system Boost::filesystem)

# Линковка с библиотеками libpqxx и libpq
target_link_libraries(test_task PRIVATE ${PQXX_LIB} ${PQ_LIB})

# Линковка с pthread
target_link_libraries(test_task PRIVATE pthread)

# Линковка со стандартной библиотекой C++
target_link_libraries(test_task PRIVATE stdc++)

