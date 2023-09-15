cmake_minimum_required(VERSION 3.10)

project(ManagerLigiPolskiej2002)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "../../")

add_executable(
    managerligipolskiej2002
    ArgumentsReader.cpp
    Language.cpp
    Footballers.cpp
    Table.cpp
    Rounds.cpp
    PlayerClub.cpp
    News.cpp
    MainMenu.cpp
    Random.cpp
    Colors.cpp
    Input.cpp
    Manager.cpp
    Tactic.cpp
    Squad.cpp
    TeamInstructions.cpp
    Match.cpp
    main.cpp
)

# For the `make install` command, it will install in /usr/local/bin
# install(TARGETS managerligipolskiej2002 DESTINATION bin)
