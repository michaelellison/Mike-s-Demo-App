#ifndef _CATStringCore_H_
#define _CATStringCore_H_
#define     CAT_STATUS                             0x00000001 // Status
#define     CAT_STAT_FILE_AT_EOF                   0x00000002 // The file is at EOF
#define     CAT_STAT_MULTIPLE_PROCS                0x00000003 // Multiple matching processes found.
#define     CAT_STAT_PATH_NO_FILE                  0x00000004 // No filename present.
#define     CAT_STAT_PATH_NO_DIRECTORY             0x00000005 // No directory present.
#define     CAT_STAT_PATH_IS_DIRECTORY             0x00000006 // The path is a directory.
#define     CAT_STAT_PATH_IS_FILE                  0x00000007 // The path is a file.
#define     CAT_STAT_PROMPT_OK                     0x00000008 // OK
#define     CAT_STAT_PROMPT_YES                    0x00000009 // Yes
#define     CAT_STAT_PROMPT_NO                     0x0000000A // No
#define     CAT_STAT_PROMPT_CANCEL                 0x0000000B // Cancel
#define     CAT_STAT_IN_PROGRESS                   0x0000000C // Progress
#define     CAT_STAT_IMAGE_ALREADY_LOADED          0x0000000D // Image already loaded.
#define     CAT_STAT_CONTROL_IMAGE_SIZE_MISMATCH   0x0000000E // Image size mismatch in control.
#define     CAT_STAT_SKIN_WINDOW_ALREADY_OPEN      0x0000000F // Window already open.
#define     CAT_STAT_SQL_ROW                       0x00001001 // sqlite3_step() has another row ready
#define     CAT_STAT_SQL_DONE                      0x00001002 // sqlite3_step() has finished executing
#define     CAT_STR_EMPTY                          0x10000000 // 
#define     CAT_STR_USAGE                          0x10000001 // Usage:
#define     CAT_STR_HELPDESC                       0x10000002 // Display Help.
#define     CAT_STR_MIKE_RESUME_LINE6              0x11000000 // Senior Software Engineer (2001-3, 2007-present)
#define     CAT_STR_MIKE_RESUME_REFLECT            0x11000001 // Senior Software Developer (2005-2008)
#define     CAT_STR_MIKE_RESUME_IRSI               0x11000002 // Senior Software Engineer (1997-2000)
#define     CAT_STR_MIKE_RESUME_PST                0x11000003 // Software Engineer (1996-1997)
#define     CAT_ERROR                              0x80000000 // Error
#define     CAT_ERR_NOT_IMPLEMENTED                0x80000001 // Feature not yet implemented.
#define     CAT_ERR_FILE_NOT_FOUND                 0x80000002 // File not found.
#define     CAT_ERR_UNABLE_TO_OPEN_PROCESS         0x80000003 // Unable to open process.
#define     CAT_ERR_REMOTE_ALLOC_RAM               0x80000004 // Unable to allocate memory in remote process.
#define     CAT_ERR_REMOTE_WRITE                   0x80000005 // Unable to write to remote process.
#define     CAT_ERR_REMOTE_CREATE_THREAD           0x80000006 // Unable to create a thread in the remote process.
#define     CAT_ERR_REMOTE_THREAD_TIMEOUT          0x80000007 // Remote thread timed out.
#define     CAT_ERR_REMOTE_THREAD_INVALID_EXIT     0x80000008 // Remote thread exited with an error.
#define     CAT_ERR_PROCESS_CREATE                 0x80000009 // Error creating process
#define     CAT_ERR_INVALID_PARAM                  0x8000000A // Invalid parameter
#define     CAT_ERR_OUT_OF_MEMORY                  0x8000000B // Out of memory
#define     CAT_ERR_FILE_OPEN                      0x8000000C // Error opening file.
#define     CAT_ERR_FILE_READ                      0x8000000D // Error reading from file
#define     CAT_ERR_FILE_HAS_OPEN_SUBSTREAMS       0x8000000E // Attempting to close a file with open substreams.
#define     CAT_ERR_FILE_NOT_OPEN                  0x8000000F // The file has not been opened.
#define     CAT_ERR_FILE_UNSUPPORTED_MODE          0x80000010 // The requested open mode is not supported.
#define     CAT_ERR_FILE_WRITE                     0x80000011 // Error writing to file.
#define     CAT_ERR_FILE_GET_POSITION              0x80000012 // Error retrieving file position.
#define     CAT_ERR_FILE_SEEK                      0x80000013 // Error seeking within file.
#define     CAT_ERR_FILE_SET_POSITION              0x80000014 // Error setting the file position.
#define     CAT_ERR_OPENING_SUBSTREAM              0x80000015 // Opening a substream is not a valid operation.
#define     CAT_ERR_CLOSING_SUBSTREAM              0x80000016 // Closing a substream is not a valid operation.
#define     CAT_ERR_SUBSTREAM_NO_PARENT            0x80000017 // Substream has no parent.
#define     CAT_ERR_WRITE_PAST_SPECIFIED_END       0x80000018 // A write request was made past the end of a substream.
#define     CAT_ERR_SEEK_PAST_SPECIFIED_END        0x80000019 // A seek request was made past the end of a substream.
#define     CAT_ERR_STRINGTABLE_NOT_FOUND          0x8000001A // Stringtable was not found for removal.
#define     CAT_ERR_CMD_TABLE_INVALID              0x8000001B // Invalid command table for command line parsing.
#define     CAT_ERR_CMD_INVALID_SWITCH             0x8000001C // Invalid switch on command line.
#define     CAT_ERR_CMD_NO_STRINGTABLE             0x8000001D // No string table.
#define     CAT_ERR_PARSE_CALLBACK_ABORT           0x8000001E // Callback during cmd line parsing aborted.
#define     CAT_ERR_ENUM_PROCS                     0x8000001F // An error occurred enumerating processes.
#define     CAT_ERR_NO_MATCHING_PROC               0x80000020 // No matching process found.
#define     CAT_ERR_READING_TARGET_EXEC            0x80000021 // Error reading target executable.
#define     CAT_ERR_EXE_NOT_PE_FORMAT              0x80000022 // Target is not in Portable Executable format.
#define     CAT_ERR_CMDREQ_GROUP                   0x80000023 // Required command-line group not found.
#define     CAT_ERR_CMDREQ_EXCLUSIVE_GROUP         0x80000024 // Multiple options set in exclusive group on command line.
#define     CAT_ERR_CMDREQ_OP                      0x80000025 // Required operand not found on command line.
#define     CAT_ERR_CMDREQ_SWITCH                  0x80000026 // Required switch not found on command line.
#define     CAT_ERR_CMD_SWITCH_NO_ARG              0x80000027 // A switch on the command line is missing a required argument.
#define     CAT_ERR_UNPATCH_TIMEOUT                0x80000028 // Timed out waiting for target process patch.
#define     CAT_ERR_HOOK_PROTECT_FAILED            0x80000029 // Setting memory protections failed when hooking function.
#define     CAT_ERR_HOOK_NOT_FOUND                 0x8000002A // Hook target function not found.
#define     CAT_ERR_HOOK_NOT_ENOUGH_BYTES          0x8000002B // Not enough bytes for push/jmp in hook.
#define     CAT_ERR_INTERCEPT_NO_DSOUND            0x8000002C // Could not find DirectSound DLL.
#define     CAT_ERR_XML_CREATE_FAILED              0x8000002D // Could not create XML object.
#define     CAT_ERR_XML_INVALID_ATTRIBUTE          0x8000002E // Invalid attribute in XML.
#define     CAT_ERR_XML_PARSER_INVALID_PATH        0x8000002F // Invalid XML path.
#define     CAT_ERR_XML_PARSER_OUT_OF_MEMORY       0x80000030 // XML Parser out of memory.
#define     CAT_ERR_XML_INVALID_XML                0x80000031 // Invalid XML file.
#define     CAT_ERR_IMAGE_NULL                     0x80000032 // Null image.
#define     CAT_ERR_IMAGE_INVALID_SUB_POSITION     0x80000033 // Invalid sub-image position.
#define     CAT_ERR_IMAGE_INVALID_SIZE             0x80000034 // Invalid image size.
#define     CAT_ERR_IMAGE_MUST_INITIALIZE          0x80000035 // Must initialize image first.
#define     CAT_ERR_IMAGE_FILL_OUT_OF_BOUNDS       0x80000036 // Image fill request is out of bounds.
#define     CAT_ERR_IMAGE_OPERATION_INVALID_ON_ROOT 0x80000037 // The image operation is invalid on root image.
#define     CAT_ERR_IMAGE_EMPTY                    0x80000038 // The image is empty.
#define     CAT_ERR_IMAGE_OUT_OF_RANGE             0x80000039 // Out of range of image size.
#define     CAT_ERR_IMAGE_OVERLAY_OUT_OF_BOUNDS    0x8000003A // Image overlay out of range.
#define     CAT_ERR_PNG_ERROR_CREATING_READ        0x8000003B // Error creating read struct for .png image.
#define     CAT_ERR_PNG_UNSUPPORTED_FORMAT         0x8000003C // Unsupported PNG format - 3 or 4 channel only.
#define     CAT_ERR_IMAGE_UNKNOWN_FORMAT           0x8000003D // Unsupported image format.
#define     CAT_ERR_PNG_CORRUPT                    0x8000003E // .PNG image file is corrupt.
#define     CAT_ERR_PNG_ERROR_CREATING_WRITE       0x8000003F // Error creating write struct for .png image.
#define     CAT_ERR_PNG_WARNING                    0x80000040 // Warning received from .png library.
#define     CAT_ERR_MUTEX_INVALID_HANDLE           0x80000041 // Invalid mutex handle.
#define     CAT_ERR_MUTEX_TIMEOUT                  0x80000042 // Mutex timed out.
#define     CAT_ERR_MUTEX_WAIT_ERROR               0x80000043 // Error waiting on mutex.
#define     CAT_ERR_SIGNAL_INVALID_HANDLE          0x80000044 // Invalid handle for signal.
#define     CAT_ERR_SIGNAL_TIMEOUT                 0x80000045 // Signal timed out.
#define     CAT_ERR_SIGNAL_WAIT_ERROR              0x80000046 // Error waiting for signal.
#define     CAT_ERR_FILE_ALREADY_EXISTS            0x80000047 // File already exists.
#define     CAT_ERR_PATH_EMPTY                     0x80000048 // The path is empty.
#define     CAT_ERR_FILE_DOES_NOT_EXIST            0x80000049 // The file does not exist.
#define     CAT_ERR_FILE_IS_DIRECTORY              0x8000004A // The path is a directory.
#define     CAT_ERR_DIR_DOES_NOT_EXIST             0x8000004B // The directory does not exist.
#define     CAT_ERR_DIR_IS_FILE                    0x8000004C // The path is a file.
#define     CAT_ERR_NULL_PARAM                     0x8000004D // Null parameter.
#define     CAT_ERR_FILESYSTEM_CREATE_DIR          0x8000004E // Filesystem is unable to create the directory.
#define     CAT_ERR_PATH_DOES_NOT_EXIST            0x8000004F // The path does not exist.
#define     CAT_ERR_FIND_NO_MATCHES                0x80000050 // No matches found.
#define     CAT_ERR_FIND_CALL_FINDFIRST            0x80000051 // Must call FindFirst before FindNext.
#define     CAT_ERR_FIND_END                       0x80000052 // No more matches.
#define     CAT_ERR_SKIN_XPIN_MISUSE               0x80000053 // XPos must be positive, XPin must be negative, and Width may not be used for scaled skin controls.
#define     CAT_ERR_SKIN_YPIN_MISUSE               0x80000054 // YPos must be positive, YPin must be negative, and Width may not be used for scaled skin controls.
#define     CAT_ERR_QUEUE_EMPTY                    0x80000055 // Queue is empty.
#define     CAT_ERR_FILE_CORRUPTED                 0x80000056 // File is corrupted.
#define     CAT_ERR_FILE_ACCESS_DENIED             0x80000057 // Access denied to file.
#define     CAT_ERR_EXECUTE_FAILED                 0x80000058 // Failed to execute file.
#define     CAT_ERR_WINDOW_CLASSREG_FAILED         0x80000059 // Window class registration failed.
#define     CAT_ERR_WINDOW_CREATE_FAILED           0x8000005A // Window creation failed.
#define     CAT_ERR_STACK_EMPTY                    0x8000005B // Stack is empty.
#define     CAT_ERR_NO_HELP_AVAILABLE              0x8000005C // No help available.
#define     CAT_ERR_FILEOPEN_CANCELLED             0x8000005D // File Open dialog was cancelled.
#define     CAT_ERR_FILESAVE_CANCELLED             0x8000005E // File Save dialog was cancelled.
#define     CAT_ERR_COMBO_MENU_NOT_FOUND           0x8000005F // Combo control is missing a menu.
#define     CAT_ERR_COMBO_EDIT_NOT_FOUND           0x80000060 // Combo control is missing an edit box.
#define     CAT_ERR_CONTROL_NO_IMAGE               0x80000061 // No image found for control.
#define     CAT_ERR_GUIFACTORY_UNKNOWN_TYPE        0x80000062 // GUIFactory - control type not found!
#define     CAT_ERR_LIST_ITEM_NOT_FOUND            0x80000063 // Item not found in list.
#define     CAT_ERR_LIST_OUT_OF_RANGE              0x80000064 // Item out of range in list.
#define     CAT_ERR_SKIN_WINDOW_NOT_FOUND          0x80000065 // Window skin not found.
#define     CAT_ERR_SKIN_WINDOW_NOT_OPEN           0x80000066 // Window not open.
#define     CAT_ERR_INVALID_TAB                    0x80000067 // Invalid tab index.
#define     CAT_ERR_CONTROL_NOT_FOUND              0x80000068 // Control not found.
#define     CAT_ERR_CMD_NOT_FOUND                  0x80000069 // Command not found.
#define     CAT_ERR_TREE_ITEM_NOT_FOUND            0x8000006A // Tree item not found.
#define     CAT_ERR_TREE_INSERT_ERROR              0x8000006B // Error inserting item into tree.
#define     CAT_ERR_TREE_REMOVE_ERROR              0x8000006C // Error removing item from tree.
#define     CAT_ERR_TREE_SET_ITEM_ERROR            0x8000006D // Error setting tree item.
#define     CAT_ERR_IMAGELIST_FAILED               0x8000006E // Imagelist failed.
#define     CAT_ERR_PREFS_NO_FILESYSTEM            0x8000006F // No filesystem for prefs.
#define     CAT_ERR_STREAM_INVALID                 0x80000070 // Stream invalid.
#define     CAT_ERR_NOT_INITIALIZED                0x80000071 // Object is not initialized.
#define     CAT_ERR_INVALID_STRINGTABLE            0x80000072 // The stringtable is invalid or missing.
#define     CAT_ERR_SQL_ERROR                      0x80001000 // SQL error or missing database
#define     CAT_ERR_SQL_INTERNAL                   0x80001001 // Internal logic error in SQLite
#define     CAT_ERR_SQL_PERM                       0x80001002 // Access permission denied
#define     CAT_ERR_SQL_ABORT                      0x80001003 // Callback routine requested an abort
#define     CAT_ERR_SQL_BUSY                       0x80001004 // The database file is locked
#define     CAT_ERR_SQL_LOCKED                     0x80001005 // A table in the database is locked
#define     CAT_ERR_SQL_NOMEM                      0x80001006 // A malloc() failed
#define     CAT_ERR_SQL_READONLY                   0x80001007 // Attempt to write a readonly database
#define     CAT_ERR_SQL_INTERRUPT                  0x80001008 // Operation terminated by sqlite3_interrupt(
#define     CAT_ERR_SQL_IOERR                      0x80001009 // Some kind of disk I/O error occurred
#define     CAT_ERR_SQL_CORRUPT                    0x8000100A // The database disk image is malformed
#define     CAT_ERR_SQL_NOTFOUND                   0x8000100B // NOT USED. Table or record not found
#define     CAT_ERR_SQL_FULL                       0x8000100C // Insertion failed because database is full
#define     CAT_ERR_SQL_CANTOPEN                   0x8000100D // Unable to open the database file
#define     CAT_ERR_SQL_PROTOCOL                   0x8000100E // NOT USED. Database lock protocol error
#define     CAT_ERR_SQL_EMPTY                      0x8000100F // Database is empty
#define     CAT_ERR_SQL_SCHEMA                     0x80001010 // The database schema changed
#define     CAT_ERR_SQL_TOOBIG                     0x80001011 // String or BLOB exceeds size limit
#define     CAT_ERR_SQL_CONSTRAINT                 0x80001012 // Abort due to constraint violation
#define     CAT_ERR_SQL_MISMATCH                   0x80001013 // Data type mismatch
#define     CAT_ERR_SQL_MISUSE                     0x80001014 // Library used incorrectly
#define     CAT_ERR_SQL_NOLFS                      0x80001015 // Uses OS features not supported on host
#define     CAT_ERR_SQL_AUTH                       0x80001016 // Authorization denied
#define     CAT_ERR_SQL_FORMAT                     0x80001017 // Auxiliary database format error
#define     CAT_ERR_SQL_RANGE                      0x80001018 // 2nd parameter to sqlite3_bind out of range
#define     CAT_ERR_SQL_NOTADB                     0x80001019 // File opened that is not a database file
#define     CAT_ERR_QUERY_NOT_FOUND                0x8000101A // Query not found in database tables.
#define     CAT_ERR_ALREADY_IN_TRANSACTION         0x8000101B // Already in a transaction.
#define     CAT_ERR_NOT_IN_TRANSACTION             0x8000101C // Not in a transaction.
#define     CAT_ERR_CAPTURE_CREATE                 0x80002000 // Error creating capture system.
#endif  //_CATStringCore_H_

