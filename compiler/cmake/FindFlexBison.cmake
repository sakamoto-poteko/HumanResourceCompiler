# FindFlexBison.cmake - A custom module to download and install Flex and Bison for Windows

# Include guard: Ensure this file is only included once
if(NOT FIND_FLEX_BISON_INCLUDED)
    set(FIND_FLEX_BISON_INCLUDED TRUE)

    include(ExternalProject)

    # Define install paths for Flex and Bison
    set(FLEX_INSTALL_DIR "${CMAKE_BINARY_DIR}/tools/flex")
    set(BISON_INSTALL_DIR "${CMAKE_BINARY_DIR}/tools/bison")

    # Download and install Flex (GnuWin32 version)
    ExternalProject_Add(
            flex
            URL https://sourceforge.net/projects/gnuwin32/files/flex/2.5.4a-1/flex-2.5.4a-1-bin.zip/download
            PREFIX "${FLEX_INSTALL_DIR}"
            CONFIGURE_COMMAND ""
            BUILD_COMMAND ""
            INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory ${FLEX_INSTALL_DIR}/src/flex ${FLEX_INSTALL_DIR}
    )

    # Download and install Bison Deps (GnuWin32 version)
    ExternalProject_Add(
            bisondeps
            URL https://sourceforge.net/projects/gnuwin32/files/bison/2.4.1/bison-2.4.1-dep.zip/download
            PREFIX "${BISON_INSTALL_DIR}"
            CONFIGURE_COMMAND ""
            BUILD_COMMAND ""
            INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory ${BISON_INSTALL_DIR}/src/bisondeps ${BISON_INSTALL_DIR}
    )
    # Download and install Bison (GnuWin32 version)
    ExternalProject_Add(
            bison
            URL https://sourceforge.net/projects/gnuwin32/files/bison/2.4.1/bison-2.4.1-bin.zip/download
            PREFIX "${BISON_INSTALL_DIR}"
            CONFIGURE_COMMAND ""
            BUILD_COMMAND ""
            INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory ${BISON_INSTALL_DIR}/src/bison ${BISON_INSTALL_DIR}
    )

    # Download and install Bison (GnuWin32 version)
    ExternalProject_Add(
            bisonlib
            URL https://sourceforge.net/projects/gnuwin32/files/bison/2.4.1/bison-2.4.1-lib.zip/download
            PREFIX "${BISON_INSTALL_DIR}"
            CONFIGURE_COMMAND ""
            BUILD_COMMAND ""
            INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory ${BISON_INSTALL_DIR}/src/bisonlib ${BISON_INSTALL_DIR}
    )


    # Download and install Bison Deps (GnuWin32 version)
    ExternalProject_Add(
            bisonfix
            URL http://marin.jb.free.fr/bison/bison-2.4.1-modified.zip
            PREFIX "${BISON_INSTALL_DIR}"
            CONFIGURE_COMMAND ""
            BUILD_COMMAND ""
            INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_if_different ${BISON_INSTALL_DIR}/src/bisonfix/bison.exe ${BISON_INSTALL_DIR}/bin/bison.exe
    )

    # Define the paths to the Flex and Bison executables
    set(FLEX_EXECUTABLE "${FLEX_INSTALL_DIR}/bin/flex.exe" CACHE FILEPATH "Path to the Flex executable")
    set(BISON_EXECUTABLE "${BISON_INSTALL_DIR}/bin/bison.exe" CACHE FILEPATH "Path to the Bison executable")

    # Mark the download as dependencies for the main project
    add_custom_target(FlexBison ALL DEPENDS flex bisondeps bison bisonlib)
    add_dependencies(bisonfix bison)
    add_dependencies(FlexBison bisonfix)

    file(TO_NATIVE_PATH "${FLEX_INSTALL_DIR}/bin" flex_bin_dir)
    file(TO_NATIVE_PATH "${BISON_INSTALL_DIR}/bin" bison_bin_dir)

    # Define the FLEX_TARGET macro
    macro(FLEX_TARGET target_name input_file output_file)
        get_filename_component(ABS_INPUT_FILE "${input_file}" ABSOLUTE)

        include_directories(${BISON_INSTALL_DIR}/include)

        add_custom_command(
                OUTPUT ${output_file}
                COMMAND ${FLEX_EXECUTABLE} -o${output_file} ${ABS_INPUT_FILE}
                DEPENDS ${input_file}
                COMMENT "Generating scanner with Flex: ${target_name}"
                VERBATIM
        )
    endmacro()

    # Define the BISON_TARGET macro with DEFINES_FILE support
    macro(BISON_TARGET target_name input_file output_file)
        cmake_parse_arguments(BISON "" "DEFINES_FILE" "" ${ARGN})

        # Make the input file path absolute
        get_filename_component(ABS_INPUT_FILE "${input_file}" ABSOLUTE)

        # Create the command for Bison
        if(BISON_DEFINES_FILE)
            get_filename_component(ABS_DEFINES_FILE "${BISON_DEFINES_FILE}" ABSOLUTE)
            add_custom_command(
                    OUTPUT ${output_file} ${ABS_DEFINES_FILE}
                    COMMAND ${CMAKE_COMMAND} -E env "PATH=${bison_bin_dir}" ${BISON_EXECUTABLE} -o ${output_file} --defines=${ABS_DEFINES_FILE} ${ABS_INPUT_FILE}
                    DEPENDS ${ABS_INPUT_FILE}
                    COMMENT "Generating parser with Bison: ${target_name}"
                    VERBATIM
            )
        else()
            add_custom_command(
                    OUTPUT ${output_file}
                    COMMAND ${CMAKE_COMMAND} -E env "PATH=${bison_bin_dir}" ${BISON_EXECUTABLE} -o ${output_file} ${ABS_INPUT_FILE}
                    DEPENDS ${ABS_INPUT_FILE}
                    COMMENT "Generating parser with Bison: ${target_name}"
                    VERBATIM
            )
        endif()
    endmacro()


endif()
