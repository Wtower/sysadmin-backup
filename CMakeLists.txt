cmake_minimum_required(VERSION 3.5)
project(sysadmin-backup)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories(gklib4/)

set(SOURCE_FILES
        gklib4/application.h
        gklib4/application.cpp
        gklib4/output.h
        gklib4/output.cpp
        gklib4/parameterrecord.h
        src/backupapp.h
        src/backupapp.cpp
        src/sysadmin-backup.cpp)

add_executable(sysadmin-backup ${SOURCE_FILES})
