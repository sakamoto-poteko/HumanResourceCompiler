# FindFlexBison.cmake - A custom module to download and install Flex and Bison for Windows

# Include guard: Ensure this file is only included once
if(NOT FIND_FLEX_BISON_INCLUDED)
    set(FIND_FLEX_BISON_INCLUDED TRUE)

    include(ExternalProject)

    set(WINFLEXBISON_INSTALL_DIR "${CMAKE_BINARY_DIR}/tools/winflexbison")

    ExternalProject_Add(
            winflexbison
            URL https://github.com/lexxmark/winflexbison/releases/download/v2.5.25/win_flex_bison-2.5.25.zip
            PREFIX "${WINFLEXBISON_INSTALL_DIR}"
            CONFIGURE_COMMAND ""
            BUILD_COMMAND ""
            INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory ${WINFLEXBISON_INSTALL_DIR}/src/winflexbison ${WINFLEXBISON_INSTALL_DIR}
    )

    set(FLEX_EXECUTABLE "${WINFLEXBISON_INSTALL_DIR}/win_flex.exe" CACHE FILEPATH "Path to the Flex executable")
    set(BISON_EXECUTABLE "${WINFLEXBISON_INSTALL_DIR}/win_bison.exe" CACHE FILEPATH "Path to the Bison executable")

    add_custom_target(FlexBison ALL DEPENDS winflexbison)

    macro(FLEX_TARGET target_name input_file output_file)
        get_filename_component(ABS_INPUT_FILE "${input_file}" ABSOLUTE)

        add_custom_command(
                OUTPUT ${output_file}
                COMMAND ${FLEX_EXECUTABLE} --wincompat -o${output_file} ${ABS_INPUT_FILE}
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
                    COMMAND ${BISON_EXECUTABLE} -o ${output_file} --defines=${ABS_DEFINES_FILE} ${ABS_INPUT_FILE}
                    DEPENDS ${ABS_INPUT_FILE}
                    COMMENT "Generating parser with Bison: ${target_name}"
                    VERBATIM
            )
        else()
            add_custom_command(
                    OUTPUT ${output_file}
                    COMMAND ${BISON_EXECUTABLE} -o ${output_file} ${ABS_INPUT_FILE}
                    DEPENDS ${ABS_INPUT_FILE}
                    COMMENT "Generating parser with Bison: ${target_name}"
                    VERBATIM
            )
        endif()
    endmacro()


endif()
