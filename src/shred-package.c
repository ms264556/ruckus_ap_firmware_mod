#include <stdlib.h>
#include <netinet/in.h>
#include <getopt.h>
#include <stdbool.h>
#include "ruckus-fw.h"

void showUsage(char *cmd)
{
    fprintf(stderr, "Usage: %s [--input] package.bl7 [--header ruckus_header.img] [--kernel ruckus_kernel.img] [--rootfs ruckus_rootfs.img] [--trailer ruckus_trailer.img]\n", cmd);
}

// Copy bytes_to_copy bytes from input_fd to output_fd.
// Set bytes_to_copy to -1 to read to the end of the input.
//
size_t copy_file_bytes(FILE *input_fd, FILE *output_fd, size_t bytes_to_copy)
{
    unsigned char buffer[0x1000];
    size_t bytes_read;
    size_t bytes_to_read = sizeof(buffer);

    size_t bytes_written = 0;
    while ((bytes_to_copy != 0) && (bytes_read = fread(buffer, 1, (bytes_to_copy >= 0 && bytes_to_read > bytes_to_copy) ? bytes_to_copy : bytes_to_read, input_fd)) > 0)
    {
        bytes_written += fwrite(buffer, 1, bytes_read, output_fd);
        bytes_to_copy -= bytes_read;
    }
    return bytes_written;
}

// Shred a .bl7 package into constituent header, kernel, rootfs (and trailer if applicable).
//
int shred_package_file(char *filePath, char *headerPath, char *kernelPath, char *rootfsPath, char *trailerPath)
{
    FILE *fd = fopen(filePath, "rb");
    if (fd == NULL)
    {
        printf("Error: Could not open input file %s.\n", filePath);
        return false;
    }

    // read and validate header
    //
    struct bin_hdr hdr;
    size_t readSize = fread(&hdr, 1, sizeof(struct bin_hdr), fd);
    //
    if (readSize != sizeof(struct bin_hdr))
    {
        printf("Error: File truncated: %s.\n", filePath);
        fclose(fd);
        return false;
    }
    if (readSize != hdr.hdr_len || memcmp(hdr.magic, HDR_MAGIC, 4) != 0)
    {
        printf("Error: Corrupt header: %s.\n", filePath);
        fclose(fd);
        return false;
    }

    // save header
    //
    FILE *header_fd = fopen(headerPath, "wb");
    if (header_fd == NULL)
    {
        printf("Error: Could not open header output file %s.\n", headerPath);
        fclose(fd);
        fclose(header_fd);
        return false;
    }
    fwrite(&hdr, 1, sizeof(struct bin_hdr), header_fd);
    fclose(header_fd);

    // munge header to local byte order
    ntoh_hdr(&hdr);

    // save kernel
    //
    FILE *kernel_fd = fopen(kernelPath, "wb");
    if (kernel_fd == NULL)
    {
        printf("Error: Could not open kernel output file %s.\n", kernelPath);
        fclose(fd);
        fclose(kernel_fd);
        return false;
    }
    copy_file_bytes(fd, kernel_fd, (int)(hdr.next_image - hdr.hdr_len));
    fclose(kernel_fd);

    // save rootfs
    //
    FILE *rootfs_fd = fopen(rootfsPath, "wb");
    if (rootfs_fd == NULL)
    {
        printf("Error: Could not open rootfs output file %s.\n", rootfsPath);
        fclose(fd);
        fclose(rootfs_fd);
        return false;
    }
    int rootfs_bytes;
    if (hdr.image_type == 1 /* ISI */ && hdr.tail_offset != 0)
    {
        rootfs_bytes = (hdr.tail_offset - hdr.next_image) + hdr.hdr_len;
    }
    else
    {
        rootfs_bytes = (hdr.binl7_len - hdr.next_image) + hdr.hdr_len;
    }
    copy_file_bytes(fd, rootfs_fd, rootfs_bytes);
    fclose(rootfs_fd);

    // save trailer, if any
    //
    if (hdr.image_type == 1 || hdr.image_type == 2)
    {
        FILE *trailer_fd = fopen(trailerPath, "wb");
        if (trailer_fd == NULL)
        {
            printf("Error: Could not open footer output file %s.\n", trailerPath);
            fclose(fd);
            fclose(trailer_fd);
            return false;
        }
        copy_file_bytes(fd, trailer_fd, -1);
        fclose(kernel_fd);
    }
    else
    {
        remove(trailerPath);
    }

    fclose(fd);
    return true;
}

int main(int argc, char **argv)
{
    char *inputFile;
    char *headerFile = "ruckus_header.img";
    char *kernelFile = "ruckus_kernel.img";
    char *rootfsFile = "ruckus_rootfs.img";
    char *trailerFile = "ruckus_trailer.img";

    if (argc == 1)
    {
        showUsage(argv[0]);
        exit(EXIT_FAILURE);
    }
    else if (argc > 1)
        inputFile = argv[1];

    static struct option long_options[] = {
        {"input", required_argument, 0, 'i'},
        {"header", required_argument, 0, 'h'},
        {"kernel", required_argument, 0, 'k'},
        {"rootfs", required_argument, 0, 'r'},
        {"trailer", required_argument, 0, 't'},
        {"help", no_argument, 0, '?'},
        {0, 0, 0, 0}};

    int opt;
    int option_index = 0;
    while ((opt = getopt_long(argc, argv, "i:h:k:r:t:?", long_options, &option_index)) != -1)
    {
        switch (opt)
        {
        case 'i':
            inputFile = optarg;
            break;
        case 'h':
            headerFile = optarg;
            break;
        case 'k':
            kernelFile = optarg;
            break;
        case 'r':
            rootfsFile = optarg;
        case 't':
            trailerFile = optarg;
            break;
        case '?':
            showUsage(argv[0]);
            exit(EXIT_SUCCESS);
        }
    }

    if (!shred_package_file(inputFile, headerFile, kernelFile, rootfsFile, trailerFile))
        exit(EXIT_FAILURE);

    return 0;
}