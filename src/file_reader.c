#include "file_reader.h"
#include <errno.h>  // global error codes
#include <fcntl.h>  // open flags
#include <unistd.h>  // read, close

static fr_error map_open_errno(int err_no) {
    switch (err_no) {
    case ENOENT:
    case ENOTDIR:
    case ENAMETOOLONG:
    case ELOOP:
        return FR_ERR_INVALID_PATH;
    case EACCES:
    case EPERM:
    case EISDIR:
    case EMFILE:
    case ENFILE:
    case ENOMEM:
    default:
        return FR_ERR_OPEN;
}
}

static fr_error map_read_errno(int err_no) {
    (void)err_no;
    return FR_ERR_READ;
}

static fr_error map_close_errno(int err_no) {
    (void)err_no;
    return FR_ERR_CLOSE;
}

fr_error fr_open(fr_reader *reader, const char *path) {
    // check parameters
    if (reader == NULL || path == NULL || path[0] == '\0') {
        return FR_ERR_INVALID_ARG;
    }

    // open the file
    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        int saved_errno = errno;  // save errno before any other calls
        return map_open_errno(saved_errno);
    }

    // save the file descriptor and initialize state
    reader->fd = fd;
    reader->eof = 0;
    return FR_OK;
}

fr_error fr_read_chunk(fr_reader *reader, void *buffer, size_t buffer_size, size_t *bytes_read) {
    // check parameters
    if (reader == NULL || buffer == NULL || bytes_read == NULL || buffer_size == 0) {
        return FR_ERR_INVALID_ARG;
    }
    if (reader->fd < 0) {
        return FR_ERR_NOT_OPEN;
    }

    // read from the file
    ssize_t result = read(reader->fd, buffer, buffer_size);
    if (result < 0) {
        int saved_errno = errno;  // save errno before any other calls
        return map_read_errno(saved_errno);
    }

    // update bytes read and EOF status
    *bytes_read = (size_t)result;
    if (result == 0) {
        reader->eof = 1;  // reached end of file
    }
    return FR_OK;
}

fr_error fr_close(fr_reader *reader) {
    // check parameters
    if (reader == NULL) {
        return FR_ERR_INVALID_ARG;
    }
    if (reader->fd < 0) {
        return FR_ERR_NOT_OPEN;
    }

    // close the file
    if (close(reader->fd) < 0) {
        int saved_errno = errno;  // save errno before any other calls
        return map_close_errno(saved_errno);
    }

    // reset reader state
    reader->fd = -1;
    reader->eof = 0;
    return FR_OK;
}