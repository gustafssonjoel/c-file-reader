# c-file-reader

Small C file-reader library for Linux and embedded Linux style workflows that I implemented to refresh my memory.

## What This Project Provides

- A minimal file reader API in include/file_reader.h
- A static library build artifact: build/libfile_reader.a
- A fixture-based test runner: build/test_runner

Core functions:

- fr_open
- fr_read_chunk
- fr_close

## Build

Build the library:

```bash
make
```

Output:

- build/libfile_reader.a

Clean artifacts:

```bash
make clean
```

## Run Tests

```bash
make test
```

Current tests cover:

- argument validation (NULL pointers, zero sizes)
- invalid path handling
- not-open and double-close behavior
- EOF state behavior
- byte-for-byte parity against stdio fread on all fixtures in test/fixtures

## API Behavior Notes

- fr_read_chunk reads up to buffer_size bytes per call.
- Check the bytes read and eof

## Batch Size / Chunk Size

- You must provide a valid buffer of that size.
- A call is not guaranteed to return all requested bytes.

## Example

```c
#include "file_reader.h"
#include <stdio.h>

int main(void) {
	fr_reader reader = {.fd = -1, .eof = 0};
	unsigned char buf[4096];
	size_t n = 0;

	if (fr_open(&reader, "test/fixtures/small.txt") != FR_OK) {
		return 1;
	}

	for (;;) {
		if (fr_read_chunk(&reader, buf, sizeof(buf), &n) != FR_OK) {
			(void)fr_close(&reader);
			return 1;
		}
		if (n == 0) {
			break;
		}
		fwrite(buf, 1, n, stdout);
	}

	return fr_close(&reader) == FR_OK ? 0 : 1;
}
```

## Project Layout

- src: implementation
- include: public header
- test: test runner and fixtures

