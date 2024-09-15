function(read_source_files sources_path list_var)
    set(parent_name "${list_var}")
    
    # Check if file exists
    if(NOT EXISTS ${CMAKE_CURRENT_LIST_DIR}/${sources_path})
        message(FATAL_ERROR "File ${sources_path} does not exist")
    else()
        # read file content and put in the list
        file(READ ${CMAKE_CURRENT_LIST_DIR}/${sources_path} file_content)

        # replace newlines by item separators
        STRING(REGEX REPLACE "\n\r?" ";" list_var "${file_content}")
    endif()
    
    # remove empty lines
    while(1)
        list(FIND list_var "" _index)

        if( ${_index} GREATER -1)
            LIST(REMOVE_AT list_var ${_index})
            continue()
        endif()
        
        break()

    endwhile()

    foreach(entry IN LISTS list_var )
        # remove comments
        if(${entry} MATCHES "^#")
            list(REMOVE_ITEM list_var ${entry})
            continue()
        endif()

        # check file exists
        if(NOT EXISTS ${CMAKE_CURRENT_LIST_DIR}/${entry})
            message(FATAL_ERROR "File at path \"${entry}\" doesn't exist")
        endif()
    endforeach()

    set(${parent_name} ${list_var} PARENT_SCOPE)
endfunction()

function(log_source_files list_var)
    message("\r")
    message("--Source files found")
    
    foreach(entry IN LISTS ${list_var})
        message("\t-- ${entry}")
    endforeach()
    
    message("\r")
endfunction()

function(log_header_files list_var)
    message("\r")
    message("--Header files found")
    
    foreach(entry IN LISTS ${list_var})
        message("\t-- ${entry}")
    endforeach()
    
    message("\r")
endfunction()

