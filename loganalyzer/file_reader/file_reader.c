#include "file_reader.h"

struct file_hanler {
    int fd;
    int current_line;
    int current_offset;
};

file_reader_status_t file_reader_open_file(file_handler_t *fhandler, const char *file_path)
{


    return FILE_READER_OK;
}

file_reader_status_t file_reader_close_file(file_handler_t *fhandler)
{

    return FILE_READER_OK;
}


file_reader_status_t file_reader_get_next_line(file_handler_t *fhandler, char **line_buffer)
{

    return FILE_READER_OK;
}

file_reader_status_t file_reader_get_previous_line(file_handler_t *fhandler, char **line_buffer)
{

    return FILE_READER_OK;
}