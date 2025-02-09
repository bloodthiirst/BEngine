#pragma once
#include <Windows.h>
#include <String/StringView.h>
#include <Allocators/Allocator.h>
#include <Containers/Queue.h>

struct FileWatcher
{
    enum class ChangeFlags
    {
        /// @brief Changes include renaming, creating, or deleting a file.
        FileNameChanged         = FILE_NOTIFY_CHANGE_FILE_NAME,
        /// @brief Changes include creating or deleting a directory.
        DirectoryNameChanged    = FILE_NOTIFY_CHANGE_DIR_NAME,
        FileAttributeChanged    = FILE_NOTIFY_CHANGE_ATTRIBUTES,
        FileSizeChanged         = FILE_NOTIFY_CHANGE_SIZE,
        FileLastWriteChanged    = FILE_NOTIFY_CHANGE_LAST_WRITE,
        FileLastAccessChanged   = FILE_NOTIFY_CHANGE_LAST_ACCESS,
        /// @brief Any change to the creation time of files
        FileCreationTimeChanged = FILE_NOTIFY_CHANGE_CREATION,
        FileSecurityChanged     = FILE_NOTIFY_CHANGE_SECURITY
    };

    struct Params
    {
        StringView directoy_path;
        bool include_subtree;
        ChangeFlags change_flag_filter;
    };

    struct ChangeInfo
    {
        ChangeFlags change_type;
        union
        {
            char data[100];
            char file_modified[100];
            struct 
            {
                char old_name[50];
                char new_name[50];
            } file_renamed;
        } info;
        
    };

    HANDLE handle;
    OVERLAPPED overlapped;
    Params params;
    Queue<ChangeInfo> changes_queue;

    static void Create(FileWatcher *out_fw, Params params);
    void Start();
    static void Destroy(FileWatcher *in_fw);
};