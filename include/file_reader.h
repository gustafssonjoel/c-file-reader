#ifndef FILE_READER_H
#define FILE_READER_H

#include <stddef.h>

// Enable C++ compatibility
#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
FR_OK = 0,
FR_ERR_INVALID_ARG,
FR_ERR_OPEN,
FR_ERR_READ,
FR_ERR_CLOSE,
FR_ERR_NOT_OPEN,
FR_ERR_INVALID_PATH,
} fr_error;

typedef struct {
int fd;
int eof;
} fr_reader;

/**
@brief Open a file for sequential reading.
Initializes the reader state and opens the file at path in read-only mode.
@param reader[out] Pointer to reader state to initialize.
@param path[in] Null-terminated file path to open.
@return FR_OK on success.
@return FR_ERR_INVALID_ARG if reader or path is NULL, or path is empty.
@return FR_ERR_OPEN if the file cannot be opened.
*/
fr_error fr_open(fr_reader *reader, const char *path);

/**
 * @brief Read a chunk of data from the file.
 * Reads up to buffer_size bytes from the file into buffer, starting from the current position.
 * Updates the reader's position and EOF status accordingly.
 * @param reader[in,out] Pointer to the reader state.
 * @param buffer[out] Pointer to the buffer to receive the data.
 * @param buffer_size[in] Size of the buffer in bytes.
 * @param bytes_read[out] Pointer to size_t to receive the number of bytes actually read
 * @return FR_OK on success.
 * @return FR_ERR_INVALID_ARG if reader, buffer, or bytes_read is NULL, or buffer_size is zero.
 * @return FR_ERR_NOT_OPEN if the reader is not open.
 * @return FR_ERR_READ if an error occurs during reading.
 */
fr_error fr_read_chunk(fr_reader *reader, void *buffer, size_t buffer_size, size_t *bytes_read);

/**
 * @brief Close the file and release any resources associated with the reader.
 * @param reader[in,out] Pointer to the reader state.
 * @return FR_OK on success.
 * @return FR_ERR_INVALID_ARG if reader is NULL.
 * @return FR_ERR_NOT_OPEN if the reader is not open.
 * @return FR_ERR_CLOSE if an error occurs during closing.
 */
fr_error fr_close(fr_reader *reader);

/**
 * @brief Get a human-readable string for a file reader error code.
 * @param err[in] The error code.
 * @return A null-terminated string describing the error.
 */
const char *fr_strerror(fr_error err);

/**
 * @brief Check if the end of the file has been reached.
 * @param reader[in] Pointer to the reader state.
 * @return Non-zero if EOF has been reached, zero otherwise.
 */
int fr_is_eof(const fr_reader *reader);

#ifdef __cplusplus
}
#endif

#endif
