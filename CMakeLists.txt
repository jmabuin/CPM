cmake_minimum_required(VERSION 3.0)
project(CPM)

set(CMAKE_CXX_STANDARD 14)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed
set(CMAKE_AUTOMOC ON)
# Create code from a list of Qt designer ui files
set(CMAKE_AUTOUIC ON)

set(CMAKE_AUTOUIC_SEARCH_PATHS src/view/)

set(THREADS_PREFER_PTHREAD_FLAG ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/modules")


SET(ONLY_GUI false CACHE BOOLEAN "Configure and build only GUI")
SET(ONLY_MASTER false CACHE BOOLEAN "Configure and build only MonitoringMaster")
SET(ONLY_AGENT false CACHE BOOLEAN "Configure and build only MonitoringAgent")

if(${ONLY_GUI})
    message("Building only GUI")
    # Find the QtWidgets library
    find_package(Qt5Widgets CONFIG REQUIRED)
    find_package(Qt5PrintSupport CONFIG REQUIRED)
    find_package(Threads REQUIRED)
    find_package(LibSSH REQUIRED)

    include_directories(src/controller)

    add_executable(MonitoringProcesses src/MonitoringProcesses.cpp src/controller/Configuration.cpp src/controller/ConfigurationWindow.cpp
            src/controller/DataWidget.cpp src/controller/mainwindow.cpp src/controller/ManageClusterWindow.cpp
            src/controller/Network.cpp src/controller/qcustomplot.cpp src/controller/ssh_handler.cpp)

    target_link_libraries(MonitoringProcesses Qt5::Widgets)
    target_link_libraries(MonitoringProcesses Qt5::PrintSupport)
    target_link_libraries(MonitoringProcesses Threads::Threads)
    target_link_libraries(MonitoringProcesses ssh)

elseif(${ONLY_MASTER})
    # else section.
    message("Building only MonitoringMaster and MonitoringAgent")
    find_package(PAPI REQUIRED)
    find_package(Procps REQUIRED)

    add_executable(MonitoringMaster src/MonitoringMaster.cpp src/Globals.cpp src/MasterChild.cpp
            src/Network.cpp)
    add_executable(MonitoringAgent src/MonitoringAgent.cpp src/Globals.cpp src/AgentChild.cpp src/Network.cpp src/Energy.cpp
            src/PapiCounts.cpp)

    target_link_libraries(MonitoringAgent papi)
    target_link_libraries(MonitoringAgent procps)

elseif(${ONLY_AGENT})
    message("Building only MonitoringAgent")
    find_package(PAPI REQUIRED)
    find_package(Procps REQUIRED)

    add_executable(MonitoringAgent src/MonitoringAgent.cpp src/Globals.cpp src/AgentChild.cpp src/Network.cpp src/Energy.cpp
            src/PapiCounts.cpp)

    target_link_libraries(MonitoringAgent papi)
    target_link_libraries(MonitoringAgent procps)
else(${ONLY_GUI})
    # Find the QtWidgets library
    find_package(Qt5Widgets CONFIG REQUIRED)
    find_package(Qt5PrintSupport CONFIG REQUIRED)
    find_package(Threads REQUIRED)
    find_package(LibSSH REQUIRED)
    find_package(PAPI REQUIRED)
    find_package(Procps REQUIRED)

    include_directories(src/controller)


    add_executable(MonitoringProcesses src/MonitoringProcesses.cpp src/controller/Configuration.cpp src/controller/ConfigurationWindow.cpp
            src/controller/DataWidget.cpp src/controller/mainwindow.cpp src/controller/ManageClusterWindow.cpp
            src/controller/Network.cpp src/controller/qcustomplot.cpp src/controller/ssh_handler.cpp)

    target_link_libraries(MonitoringProcesses Qt5::Widgets)
    target_link_libraries(MonitoringProcesses Qt5::PrintSupport)
    target_link_libraries(MonitoringProcesses Threads::Threads)
    target_link_libraries(MonitoringProcesses ssh)


    add_executable(MonitoringMaster src/MonitoringMaster.cpp src/Globals.cpp src/MasterChild.cpp
            src/Network.cpp)

    add_executable(MonitoringAgent src/MonitoringAgent.cpp src/Globals.cpp src/AgentChild.cpp src/Network.cpp src/Energy.cpp
            src/PapiCounts.cpp)

    target_link_libraries(MonitoringAgent papi)
    target_link_libraries(MonitoringAgent procps)
endif(${ONLY_GUI})

