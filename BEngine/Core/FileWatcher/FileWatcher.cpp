#include "FileWatcher.h"
#include <Windows.h>
#include <String/StringView.h>
#include <Context/CoreContext.h>
#include <Allocators/Allocator.h>
#include <Defer/Defer.h>
#include <Containers/Queue.h>
#include "../Global/Global.h"

void FileWatcher::Create(FileWatcher *out_fw, Params params)
{
    ArenaCheckpoint check = Global::alloc_toolbox.GetArenaCheckpoint();

    DEFER([&]()
          { Global::alloc_toolbox.ResetArenaOffset(&check); });
    const char *c_str_path = StringView::ToCString(params.directoy_path, Global::alloc_toolbox.frame_allocator);

    *out_fw = {};

    HANDLE handle = CreateFile(c_str_path,
                               FILE_LIST_DIRECTORY,
                               FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                               NULL,
                               OPEN_EXISTING,
                               FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
                               NULL);

    assert(handle != INVALID_HANDLE_VALUE);

    OVERLAPPED overlapped = {};
    overlapped.hEvent = CreateEvent(NULL, FALSE, 0, NULL);

    // TODO : change this , make it something the user passes
    Queue<ChangeInfo>::Create(&out_fw->changes_queue, 100, Global::alloc_toolbox.heap_allocator);
    out_fw->handle = handle;
    out_fw->params = params;
    out_fw->overlapped = overlapped;
}

void FileWatcher::Start()
{
    const size_t change_buff_size = 1024;
    uint8_t change_buf[change_buff_size];

    while (true)
    {
        CoreContext::mem_init(&change_buf, change_buff_size);

        BOOL success = ReadDirectoryChangesW(handle, change_buf, 1024, params.include_subtree, (DWORD)params.change_flag_filter,
                                             NULL, &overlapped, NULL);

        DWORD result = WaitForSingleObject(overlapped.hEvent, INFINITE);

        assert(result == WAIT_OBJECT_0);

        DWORD bytes_transferred;
        GetOverlappedResult(handle, &overlapped, &bytes_transferred, FALSE);

        FILE_NOTIFY_INFORMATION *event = (FILE_NOTIFY_INFORMATION *)change_buf;

        do
        {
            Queue<FileWatcher::ChangeInfo> *queue = &Global::filewatch_ctx.file_watcher.changes_queue;
            FileWatcher::ChangeInfo info = {};

            switch (event->Action)
            {
            case FILE_ACTION_ADDED:
            {
                info.change_type = FileWatcher::ChangeFlags::FileLastWriteChanged;
                wcstombs((char *)&info.info.file_modified, (const wchar_t *)&event->FileName, event->FileNameLength + 1);

                Global::filewatch_ctx.queue_access_lock.Lock();
                {
                    Queue<FileWatcher::ChangeInfo>::Enqueue(queue, info);
                }
                Global::filewatch_ctx.queue_access_lock.Unlock();
                break;
            }

            case FILE_ACTION_REMOVED:
            {
                info.change_type = FileWatcher::ChangeFlags::FileAttributeChanged;
                wcstombs((char *)&info.info.file_modified, (const wchar_t *)&event->FileName, event->FileNameLength + 1);
                
                Global::filewatch_ctx.queue_access_lock.Lock();
                {
                    Queue<FileWatcher::ChangeInfo>::Enqueue(queue, info);
                }
                Global::filewatch_ctx.queue_access_lock.Unlock();
                break;
            }

            case FILE_ACTION_MODIFIED:
            {
                info.change_type = FileWatcher::ChangeFlags::FileLastWriteChanged;
                wcstombs((char *)&info.info.file_modified, (const wchar_t *)&event->FileName, event->FileNameLength + 1);
                
                Global::filewatch_ctx.queue_access_lock.Lock();
                {
                    Queue<FileWatcher::ChangeInfo>::Enqueue(queue, info);
                }
                Global::filewatch_ctx.queue_access_lock.Unlock();
                break;
            }

            case FILE_ACTION_RENAMED_OLD_NAME:
            {
                info.change_type = FileWatcher::ChangeFlags::FileNameChanged;
                wcstombs((char *)&info.info.file_renamed.old_name, (const wchar_t *)&event->FileName, event->FileNameLength + 1);         
                break;
            }

            case FILE_ACTION_RENAMED_NEW_NAME:
            {
                info.change_type = FileWatcher::ChangeFlags::FileNameChanged;
                wcstombs((char *)&info.info.file_renamed.new_name, (const wchar_t *)&event->FileName, event->FileNameLength + 1);
                
                Global::filewatch_ctx.queue_access_lock.Lock();
                {
                    Queue<FileWatcher::ChangeInfo>::Enqueue(queue, info);
                }
                Global::filewatch_ctx.queue_access_lock.Unlock();
                break;
            }

            default:
            {
                printf("Unknown action!\n");
                break;
            }
            }

            *((uint8_t **)&event) += event->NextEntryOffset;
        }
        while(event->NextEntryOffset);
    }
}

void FileWatcher::Destroy(FileWatcher *in_fw)
{
    Queue<ChangeInfo>::Destroy(&in_fw->changes_queue);
}