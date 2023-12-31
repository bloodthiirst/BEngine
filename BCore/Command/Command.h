#pragma once
#include "../String/StringView.h"
#include "../String/StringBuffer.h"
#include "../Context/CoreContext.h"
#include <cstdio>

struct Command
{
    StringView cmd_line;

    static void Create( StringView cmd_line, Command* out )
    {
        Command cmd;
        cmd.cmd_line = cmd_line;

        *out = cmd;
    }

    static void Run( Command cmd, StringBuffer output )
    {
        const char* c_str = (char*) _alloca( output.length + 1 );

        CoreContext::mem_copy((void*) cmd.cmd_line.buffer, (void*) c_str ,cmd.cmd_line.length );

        FILE* shader_builder_cmd = _popen( c_str, "rt" );

        char psBuffer[128];
        while ( fgets( psBuffer, 128, shader_builder_cmd ) )
        {
            puts( psBuffer );
        }

        int closeReturnVal = _pclose( shader_builder_cmd );
    }
};
