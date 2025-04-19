#include "miniz.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "zlib_impl.h"


// Create necessary parent directories (recursively)
void make_parent_dirs(const char *path) {
    char tmp[1024];
    strncpy(tmp, path, sizeof(tmp));
    for (char *p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = '\0';
            mkdir(tmp, 0755);  // may fail if already exists, that's fine
            *p = '/';
        }
    }
}

// Extract a ZIP archive to a target directory
int unzip_archive(const char *zip_path, const char *output_dir) {
    mz_zip_archive zip;
    memset(&zip, 0, sizeof(zip));

    if (!mz_zip_reader_init_file(&zip, zip_path, 0)) {
        fprintf(stderr, "Error: Failed to open zip file: %s\n", zip_path);
        return 0;
    }

    int num_files = (int)mz_zip_reader_get_num_files(&zip);
    printf("Found %d files in archive.\n", num_files);

    for (int i = 0; i < num_files; ++i) {
        mz_zip_archive_file_stat file_stat;
        if (!mz_zip_reader_file_stat(&zip, i, &file_stat)) {
            fprintf(stderr, "Warning: Could not read file stat for entry %d\n", i);
            continue;
        }

        // Construct the full output path
        char output_path[1024];
        snprintf(output_path, sizeof(output_path), "%s/%s", output_dir, file_stat.m_filename);

        if (mz_zip_reader_is_file_a_directory(&zip, i)) {
            // Make directory
            make_parent_dirs(output_path);
            mkdir(output_path, 0755);
            continue;
        }

        // Create parent dirs before writing the file
        make_parent_dirs(output_path);

        printf("Extracting: %s\n", output_path);

        if (!mz_zip_reader_extract_to_file(&zip, i, output_path, 0)) {
            fprintf(stderr, "Error: Failed to extract: %s\n", output_path);
        }
    }

    mz_zip_reader_end(&zip);
    printf("Extraction completed successfully.\n");
    return 1;
}


int zlib_unpack( const char *archive, const char *out_dir) 
{
    if (!unzip_archive(archive, out_dir)) {
        fprintf(stderr, "Extraction failed.\n");
        return 1;
    }
    return 0;
}
