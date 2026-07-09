# ============================================================
#  CMakeLists.txt  -  Build do backend SixSeven_Projects
#
#  Requisitos (ver README para instalacao):
#   * CMake >= 3.16, compilador C++17
#   * Drogon (framework HTTP) + JsonCpp + OpenSSL
#   * Cliente MySQL (para o ORM do Drogon falar com o banco)
#   * libbcrypt (vendorizada em composers/libbcrypt) para hash de senha
#
#  Como o projeto e majoritariamente header-only (app/ e .hpp),
#  compilamos apenas main.cpp + a libbcrypt. Os headers sao
#  incluidos por main.cpp via bootstrap/Bootstrap.hpp.
# ============================================================

cmake_minimum_required(VERSION 3.16)
project(SixSevenBackend C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# ---- Dependencias externas ----
find_package(Drogon CONFIG REQUIRED)   # traz JsonCpp, Trantor, OpenSSL, etc.

# ---- libbcrypt (vendorizada) ----
# Compilamos a libbcrypt junto. Os arquivos ficam em composers/libbcrypt.
# (bcrypt.c + crypt_blowfish/*.c/S). Ver composers/README.
set(BCRYPT_DIR ${CMAKE_SOURCE_DIR}/composers/libbcrypt)
add_library(bcrypt STATIC
    ${BCRYPT_DIR}/bcrypt.c
    ${BCRYPT_DIR}/crypt_blowfish/crypt_blowfish.c
    ${BCRYPT_DIR}/crypt_blowfish/crypt_gensalt.c
    ${BCRYPT_DIR}/crypt_blowfish/wrapper.c
)
target_include_directories(bcrypt PUBLIC ${BCRYPT_DIR})
# A libbcrypt espera encontrar <bcrypt/bcrypt.h>; expomos o pai
# de "composers/" para que #include <bcrypt/bcrypt.h> resolva.
target_include_directories(bcrypt PUBLIC ${CMAKE_SOURCE_DIR}/composers)

# ---- Executavel principal ----
add_executable(sixseven_backend main.cpp)

target_include_directories(sixseven_backend PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/composers   # para <bcrypt/bcrypt.h>
)

target_link_libraries(sixseven_backend PRIVATE
    Drogon::Drogon
    bcrypt
)

# ---- Copia a pasta de config e cria dirs de runtime ----
# O binario espera "config/app.config.json" no diretorio de execucao,
# alem de ./public e ./storage/logs.
add_custom_command(TARGET sixseven_backend POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/config $<TARGET_FILE_DIR:sixseven_backend>/config
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/public $<TARGET_FILE_DIR:sixseven_backend>/public
    COMMAND ${CMAKE_COMMAND} -E make_directory
            $<TARGET_FILE_DIR:sixseven_backend>/storage/logs
)
