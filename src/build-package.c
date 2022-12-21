#include <stdlib.h>
#include <netinet/in.h>
#include <getopt.h>
#include <stdbool.h>
#include <time.h>
#include "md5.h"
#include "ruckus-fw.h"

void showUsage(char *cmd)
{
    fprintf(stderr, "Usage: %s package.bl7 [--header ruckus_header.img] [--kernel ruckus_kernel.img] [--rootfs ruckus_rootfs.img] [--footer ruckus_footer.img]\n", cmd);
}

// Copy bytes_to_copy bytes from input_fd to output_fd, keeping an MD5 state updated.
// Set bytes_to_copy to -1 to read to the end of the input.
//
size_t copy_file_bytes_with_md5(FILE *input_fd, FILE *output_fd, size_t bytes_to_copy, md5_state_t *state)
{
    unsigned char buffer[0x1000];
    size_t bytes_read;
    size_t bytes_to_read = sizeof(buffer);

    size_t bytes_written = 0;
    while ((bytes_to_copy != 0) && (bytes_read = fread(buffer, 1, (bytes_to_copy >= 0 && bytes_to_read > bytes_to_copy) ? bytes_to_copy : bytes_to_read, input_fd)) > 0)
    {
        bytes_written += fwrite(buffer, 1, bytes_read, output_fd);
        if (state != NULL)
            md5_append(state, (const md5_byte_t *)buffer, bytes_read);
        bytes_to_copy -= bytes_read;
    }
    return bytes_written;
}

// Concatenate a header, kernel and rootfs into a .bl7 package.
// Header hashes, offsets, etc are replace with calculated values.
//
int build_package_file(char *filePath, char *headerPath, char *kernelPath, char *rootfsPath)
{
    FILE *fd = fopen(filePath, "wb");
    if (fd == NULL)
    {
        printf("Error: Could not open output file %s.\n", filePath);
        return false;
    }

    // write scratch header
    FILE *header_fd = fopen(headerPath, "rb");
    if (header_fd == NULL)
    {
        printf("Error: Could not open header input file %s.\n", headerPath);
        return false;
    }
    struct bin_hdr hdr;
    size_t readSize = fread(&hdr, 1, sizeof(struct bin_hdr), header_fd);
    fclose(header_fd);

    if (readSize != sizeof(struct bin_hdr))
    {
        printf("Error: File truncated: %s.\n", headerPath);
        return false;
    }
    size_t header_bytes = fwrite(&hdr, 1, sizeof(struct bin_hdr), fd);
    size_t bytes_written = header_bytes;
    ntoh_hdr(&hdr); // munge header byte order so it's easier to work with

    // start building md5 hash for kernel+rootfs
    md5_state_t state;
    md5_init(&state);

    // write kernel
    FILE *kernel_fd = fopen(kernelPath, "rb");
    if (kernel_fd == NULL)
    {
        printf("Error: Could not open kernel input file %s.\n", kernelPath);
        return false;
    }
    bytes_written += copy_file_bytes_with_md5(kernel_fd, fd, -1, &state);
    fclose(kernel_fd);
    // pad header+kernel so size is a multiple of 64k
    int pad_bytes = ((bytes_written + 0xffff) & 0xffff0000) - bytes_written;
    if (pad_bytes)
    {
        unsigned char buffer[pad_bytes];
        memset(buffer, 0xff, pad_bytes);
        bytes_written += fwrite(buffer, sizeof(unsigned char), pad_bytes, fd);
        md5_append(&state, (const md5_byte_t *)buffer, pad_bytes);
    }
    size_t next_image = bytes_written;

    // write rootfs
    FILE *rootfs_fd = fopen(rootfsPath, "rb");
    if (rootfs_fd == NULL)
    {
        printf("Error: Could not open rootfs input file %s.\n", rootfsPath);
        return false;
    }
    bytes_written += copy_file_bytes_with_md5(rootfs_fd, fd, -1, &state);
    fclose(rootfs_fd);
    size_t binl7_len = bytes_written - header_bytes;

    // retrieve the kernel+rootfs md5 hash
    md5_byte_t signature[16];
    md5_finish(&state, signature);

    // finalize header calculations
    // calculated offsets
    hdr.next_image = next_image;
    hdr.binl7_len = binl7_len;
    // trailers are currently unsupported - remove from header
    hdr.image_type = 0;
    hdr.tail_offset = 0;
    // timestamp (remove this if your header has the timestamp you want)
    hdr.timestamp = time(NULL);
    // image md5 & header checksum
    memcpy(hdr.signature, signature, sizeof(signature));
    hdr.hdr_cksum = calc_hdr_cksum(&hdr);

    // overwrite scratch header with final version
    hton_hdr(&hdr); // un-munge header byte order
    fseek(fd, 0, SEEK_SET);
    fwrite(&hdr, 1, header_bytes, fd);

    fclose(fd);
    return true;
}

int main(int argc, char **argv)
{
    char *outputFile;
    char *headerFile = "ruckus_header.img";
    char *kernelFile = "ruckus_kernel.img";
    char *rootfsFile = "ruckus_rootfs.img";

    if (argc == 1)
    {
        showUsage(argv[0]);
        exit(EXIT_FAILURE);
    }
    else if (argc > 1)
        outputFile = argv[1];

    static struct option long_options[] = {
        {"output", required_argument, 0, 'o'},
        {"header", required_argument, 0, 'h'},
        {"kernel", required_argument, 0, 'k'},
        {"rootfs", required_argument, 0, 'r'},
        {"help", no_argument, 0, '?'},
        {0, 0, 0, 0}};

    int opt;
    int option_index = 0;
    while ((opt = getopt_long(argc, argv, "o:h:k:r:?", long_options, &option_index)) != -1)
    {
        switch (opt)
        {
        case 'o':
            outputFile = optarg;
            break;
        case 'h':
            headerFile = optarg;
            break;
        case 'k':
            kernelFile = optarg;
            break;
        case 'r':
            rootfsFile = optarg;
            break;
        case '?':
            showUsage(argv[0]);
            exit(EXIT_SUCCESS);
        }
    }

    if (!build_package_file(outputFile, headerFile, kernelFile, rootfsFile))
        exit(EXIT_FAILURE);

    return 0;
}
