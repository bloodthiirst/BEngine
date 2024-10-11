#pragma once
#include <String/StringView.h>
#include <String/StringBuffer.h>
#include <Allocators/Allocator.h>
#include <windows.h>
#include <assert.h>
#include "../Global/Global.h"

struct Command
{
    StringView cmd_line;

    static void Create(Command* out_cmd , StringView cmd_line  )
    {
        *out_cmd = {};
        out_cmd->cmd_line = cmd_line;
    }

    static void Run(Command cmd, StringBuffer* out_stdout , Allocator alloc )
    {
        StringBuffer cmd_line_buff = StringBuffer::Create(cmd.cmd_line , alloc);
        char* cmd_line = StringView::ToCString(cmd.cmd_line , alloc);

        SECURITY_ATTRIBUTES sa = {0};
        sa.nLength = sizeof(sa);
        sa.lpSecurityDescriptor = NULL;
        sa.bInheritHandle = TRUE;

        HANDLE h_stdout_read, h_stdout_write;
        HANDLE h_stderr_read, h_stderr_wr;

        assert(CreatePipe(&h_stdout_read, &h_stdout_write, &sa, 0));

        assert(CreatePipe(&h_stderr_read, &h_stderr_wr, &sa, 0));

        SetHandleInformation(h_stdout_read, HANDLE_FLAG_INHERIT, 0);
        SetHandleInformation(h_stderr_read, HANDLE_FLAG_INHERIT, 0);

        STARTUPINFO start_info = {0};
        start_info.cb = sizeof(start_info);
        start_info.dwFlags = STARTF_USESTDHANDLES;
        start_info.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
        start_info.hStdOutput = h_stdout_write;
        start_info.hStdError = h_stderr_wr;

        PROCESS_INFORMATION process_info = {0};

        bool success = CreateProcess(nullptr, cmd_line, nullptr, nullptr, true, 0, nullptr, nullptr, &start_info, &process_info);
        
        if(!success)
        {
            Global::logger.Error("Command error while running");
        }
        else
        {
            // read from hStdOutRd and hStdErrRd as needed until the process is terminated...
            DWORD stdout_size = {};
            assert(PeekNamedPipe( h_stdout_read, nullptr, 0, nullptr, &stdout_size, nullptr ));
        
            *out_stdout = StringBuffer::Create(stdout_size , alloc);
        
            if(stdout_size != 0)
            {
                assert(ReadFile(h_stdout_read , (void*)out_stdout->buffer , stdout_size , nullptr , nullptr ));
            }

            WaitForSingleObject(process_info.hProcess , INFINITE);

            CloseHandle(process_info.hThread);
            CloseHandle(process_info.hProcess);
        }

        CloseHandle(h_stdout_read);
        CloseHandle(h_stdout_write);
        CloseHandle(h_stderr_read);
        CloseHandle(h_stderr_wr);
    }
};
