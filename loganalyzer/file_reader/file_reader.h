#ifndef FILE_READER_H
#define FILE_READER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>


typedef struct file_hanler file_handler_t;

typedef enum {
    FILE_READER_OK = 0,
    FILE_READER_BAD_ARG,
    FILE_READER_CANT_FIND,
    FILE_READER_EOF

} file_reader_status_t;

/**
 * @brief Open log file and return file handler.
 * 
 * @details 
 * 
 * @param pointer to file handler structure
 * @param full path to file
 * @return opearation status. see file_reader_status_t
 */
file_reader_status_t file_reader_open_file(file_handler_t *fhandler, const char *file_path);

/**
 * @brief 
 * 
 * @details 
 * 
 * @param 
 * @param 
 * @return opearation status. see file_reader_status_t
 */
file_reader_status_t file_reader_close_file(file_handler_t *fhandler);

/**
 * @brief 
 * 
 * @details 
 * 
 * @param pointer to file handler structure
 * @param 
 * @return opearation status. see file_reader_status_t
 */
file_reader_status_t file_reader_get_next_line(file_handler_t *fhandler, char **line_buffer);

/**
 * @brief 
 * 
 * @details 
 * 
 * @param pointer to file handler structure
 * @param 
 * @return opearation status. see file_reader_status_t
 */
file_reader_status_t file_reader_get_previous_line(file_handler_t *fhandler, char **line_buffer);

#endif // FILE_READER_H
