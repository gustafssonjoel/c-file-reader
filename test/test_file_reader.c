#include "file_reader.h"

#include <stdio.h>
#include <string.h>

static int expect_error(const char *label, fr_error actual, fr_error expected) {
    if (actual != expected) {
        fprintf(stderr,
                "FAIL %s: expected error %d, got %d\n",
                label,
                (int)expected,
                (int)actual);
        return 1;
    }
    return 0;
}

static int test_open_invalid_args(void) {
    int failed = 0;
    fr_reader reader = {.fd = -1, .eof = 0};

    failed += expect_error("fr_open(NULL, path)",
                           fr_open(NULL, "test/fixtures/small.txt"),
                           FR_ERR_INVALID_ARG);
    failed += expect_error("fr_open(reader, NULL)",
                           fr_open(&reader, NULL),
                           FR_ERR_INVALID_ARG);
    failed += expect_error("fr_open(reader, empty)",
                           fr_open(&reader, ""),
                           FR_ERR_INVALID_ARG);

    if (!failed) {
        printf("PASS test_open_invalid_args\n");
    }
    return failed;
}

static int test_open_invalid_path(void) {
    fr_reader reader = {.fd = -1, .eof = 0};
    fr_error err = fr_open(&reader, "test/fixtures/does_not_exist.txt");
    if (err != FR_ERR_INVALID_PATH) {
        fprintf(stderr,
                "FAIL test_open_invalid_path: expected %d, got %d\n",
                (int)FR_ERR_INVALID_PATH,
                (int)err);
        return 1;
    }
    printf("PASS test_open_invalid_path\n");
    return 0;
}

static int test_read_invalid_args_and_not_open(void) {
    int failed = 0;
    fr_reader reader = {.fd = -1, .eof = 0};
    unsigned char buf[8];
    size_t bytes_read = 0;

    failed += expect_error("fr_read_chunk(NULL, ...)",
                           fr_read_chunk(NULL, buf, sizeof(buf), &bytes_read),
                           FR_ERR_INVALID_ARG);
    failed += expect_error("fr_read_chunk(reader, NULL, ...)",
                           fr_read_chunk(&reader, NULL, sizeof(buf), &bytes_read),
                           FR_ERR_INVALID_ARG);
    failed += expect_error("fr_read_chunk(reader, ..., size=0, ...)",
                           fr_read_chunk(&reader, buf, 0, &bytes_read),
                           FR_ERR_INVALID_ARG);
    failed += expect_error("fr_read_chunk(reader, ..., NULL)",
                           fr_read_chunk(&reader, buf, sizeof(buf), NULL),
                           FR_ERR_INVALID_ARG);
    failed += expect_error("fr_read_chunk(not_open)",
                           fr_read_chunk(&reader, buf, sizeof(buf), &bytes_read),
                           FR_ERR_NOT_OPEN);

    if (!failed) {
        printf("PASS test_read_invalid_args_and_not_open\n");
    }
    return failed;
}

static int test_close_invalid_and_double_close(void) {
    int failed = 0;
    fr_reader reader = {.fd = -1, .eof = 0};

    failed += expect_error("fr_close(NULL)", fr_close(NULL), FR_ERR_INVALID_ARG);
    failed += expect_error("fr_close(not_open)", fr_close(&reader), FR_ERR_NOT_OPEN);

    if (fr_open(&reader, "test/fixtures/small.txt") != FR_OK) {
        fprintf(stderr, "FAIL test_close_invalid_and_double_close: fr_open failed\n");
        return 1;
    }

    failed += expect_error("fr_close(open_reader)", fr_close(&reader), FR_OK);
    failed += expect_error("fr_close(double_close)", fr_close(&reader), FR_ERR_NOT_OPEN);

    if (!failed) {
        printf("PASS test_close_invalid_and_double_close\n");
    }
    return failed;
}

static int test_open_resets_eof(void) {
    fr_reader reader = {.fd = -1, .eof = 1};
    fr_error err = fr_open(&reader, "test/fixtures/small.txt");
    if (err != FR_OK) {
        fprintf(stderr, "FAIL test_open_resets_eof: fr_open error %d\n", (int)err);
        return 1;
    }

    if (reader.eof != 0) {
        fprintf(stderr, "FAIL test_open_resets_eof: eof was not reset\n");
        (void)fr_close(&reader);
        return 1;
    }

    err = fr_close(&reader);
    if (err != FR_OK) {
        fprintf(stderr, "FAIL test_open_resets_eof: fr_close error %d\n", (int)err);
        return 1;
    }

    printf("PASS test_open_resets_eof\n");
    return 0;
}

static int test_eof_and_read_after_eof(void) {
    fr_reader reader = {.fd = -1, .eof = 0};
    fr_error err = fr_open(&reader, "test/fixtures/empty.txt");
    if (err != FR_OK) {
        fprintf(stderr, "FAIL test_eof_and_read_after_eof: fr_open error %d\n", (int)err);
        return 1;
    }

    unsigned char buf[8];
    size_t bytes_read = 123;

    err = fr_read_chunk(&reader, buf, sizeof(buf), &bytes_read);
    if (err != FR_OK || bytes_read != 0 || reader.eof != 1) {
        fprintf(stderr,
                "FAIL test_eof_and_read_after_eof: first EOF read mismatch (err=%d bytes=%zu eof=%d)\n",
                (int)err,
                bytes_read,
                reader.eof);
        (void)fr_close(&reader);
        return 1;
    }

    bytes_read = 123;
    err = fr_read_chunk(&reader, buf, sizeof(buf), &bytes_read);
    if (err != FR_OK || bytes_read != 0 || reader.eof != 1) {
        fprintf(stderr,
                "FAIL test_eof_and_read_after_eof: second EOF read mismatch (err=%d bytes=%zu eof=%d)\n",
                (int)err,
                bytes_read,
                reader.eof);
        (void)fr_close(&reader);
        return 1;
    }

    err = fr_close(&reader);
    if (err != FR_OK) {
        fprintf(stderr, "FAIL test_eof_and_read_after_eof: fr_close error %d\n", (int)err);
        return 1;
    }

    printf("PASS test_eof_and_read_after_eof\n");
    return 0;
}

static int test_fixture_matches_stdio(const char *path) {
    fr_reader reader = {.fd = -1, .eof = 0};
    fr_error err = fr_open(&reader, path);
    if (err != FR_OK) {
        fprintf(stderr, "FAIL %s: fr_open error %d\n", path, (int)err);
        return 1;
    }

    FILE *expected = fopen(path, "rb");
    if (expected == NULL) {
        fprintf(stderr, "FAIL %s: fopen failed\n", path);
        (void)fr_close(&reader);
        return 1;
    }

    unsigned char fr_buf[17];
    unsigned char std_buf[17];
    int failed = 0;

    for (;;) {
        size_t fr_bytes = 0;
        err = fr_read_chunk(&reader, fr_buf, sizeof(fr_buf), &fr_bytes);
        if (err != FR_OK) {
            fprintf(stderr, "FAIL %s: fr_read_chunk error %d\n", path, (int)err);
            failed = 1;
            break;
        }

        size_t std_bytes = fread(std_buf, 1, sizeof(std_buf), expected);
        if (std_bytes != fr_bytes) {
            fprintf(stderr,
                    "FAIL %s: byte count mismatch (fr=%zu stdio=%zu)\n",
                    path,
                    fr_bytes,
                    std_bytes);
            failed = 1;
            break;
        }

        if (fr_bytes > 0 && memcmp(fr_buf, std_buf, fr_bytes) != 0) {
            fprintf(stderr, "FAIL %s: content mismatch\n", path);
            failed = 1;
            break;
        }

        if (fr_bytes == 0) {
            if (ferror(expected)) {
                fprintf(stderr, "FAIL %s: stdio read error\n", path);
                failed = 1;
            }
            break;
        }
    }

    if (fclose(expected) != 0) {
        fprintf(stderr, "FAIL %s: fclose failed\n", path);
        failed = 1;
    }

    err = fr_close(&reader);
    if (err != FR_OK) {
        fprintf(stderr, "FAIL %s: fr_close error %d\n", path, (int)err);
        failed = 1;
    }

    if (!failed) {
        printf("PASS %s\n", path);
    }

    return failed;
}

int main(void) {
    static const char *fixtures[] = {
        "test/fixtures/empty.txt",
        "test/fixtures/large_binary",
        "test/fixtures/multi_chunk.txt",
        "test/fixtures/small.txt",
        "test/fixtures/somejsonfile.json",
        "test/fixtures/somejsonfile_formatted.json",
        "test/fixtures/utf16be_bom.txt",
        "test/fixtures/utf16le_bom.txt",
        "test/fixtures/utf32le_bom.txt",
        "test/fixtures/utf8_bom.txt",
        "test/fixtures/utf8_no_bom.txt",
    };

    int failures = 0;

    failures += test_open_invalid_args();
    failures += test_open_invalid_path();
    failures += test_read_invalid_args_and_not_open();
    failures += test_close_invalid_and_double_close();
    failures += test_open_resets_eof();
    failures += test_eof_and_read_after_eof();

    size_t i;
    for (i = 0; i < sizeof(fixtures) / sizeof(fixtures[0]); ++i) {
        failures += test_fixture_matches_stdio(fixtures[i]);
    }

    if (failures != 0) {
        fprintf(stderr, "\n%d fixture test(s) failed\n", failures);
        return 1;
    }

    printf("\nAll fixture tests passed\n");
    return 0;
}