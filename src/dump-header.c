#include <stdlib.h>
#include <netinet/in.h>
#include <getopt.h>
#include <stdbool.h>
#include "ruckus-fw.h"

void showUsage(char *cmd)
{
    fprintf(stderr, "Usage: %s [--input] package.bl7\n", cmd);
}

int bin_hdr_dump(struct bin_hdr *hdrp, md5_byte_t *calculated_digest)
{
	int is_header_valid = true;

	struct bin_hdr hdr_localorder = *hdrp;
	ntoh_hdr(&hdr_localorder);

	printf("Magic:           %.4s", hdr_localorder.magic);
	if (memcmp(hdr_localorder.magic, HDR_MAGIC, 4) != 0)
	{
		printf(" != %.4s - bad magic\n", HDR_MAGIC);
		is_header_valid = false;
	}
	else
	{
		printf("\n");
	}
	printf("next_image:      0x%X\n", hdr_localorder.next_image);
	printf("invalid:         %d\n", hdr_localorder.invalid);
	printf("hdr_len:         0x%X\n", hdr_localorder.hdr_len);
	printf("compression:     %.2s\n", hdr_localorder.compression);
	printf("load_address:    0x%X\n", hdr_localorder.load_address);
	printf("entry_point:     0x%X\n", hdr_localorder.entry_point);
	printf("timestamp:       0x%X\n", hdr_localorder.timestamp);
	printf("binl7_len:       0x%X\n", hdr_localorder.binl7_len);
	printf("hdr_version:     %d\n", hdr_localorder.hdr_version);
	printf("hdr_cksum:       0x%04X", hdr_localorder.hdr_cksum);
	uint16_t calc_cksum = calc_hdr_cksum(hdrp);
	if (hdr_localorder.hdr_cksum != calc_cksum)
	{
		printf(" != 0x%04X - bad cksum\n", calc_cksum);
		is_header_valid = false;
	}
	else
	{
		printf("\n");
	}
	printf("version:         %.*s\t(%.*s)\n", (int)sizeof(hdr_localorder.version_v2), hdr_localorder.version_v2, (int)sizeof(hdr_localorder.version), hdr_localorder.version);
	printf("MD5:             0x");
	for (int i = 0; i < (int)sizeof(hdr_localorder.signature); i++)
	{
		printf("%02X", hdr_localorder.signature[i]);
	}
	if (calculated_digest != 0 && memcmp(calculated_digest, hdr_localorder.signature, (int)sizeof(hdr_localorder.signature)) != 0)
	{
		printf(" != 0x");
		for (int i = 0; i < (int)sizeof(hdr_localorder.signature); i++)
		{
			printf("%02X", calculated_digest[i]);
		}
		is_header_valid = false;
	}
	printf("\n");
	printf("product:         %.*s\t(%d)\n", (int)sizeof(hdr_localorder.product_v2), hdr_localorder.product_v2, hdr_localorder.product);
	printf("architecture:    %d\n", (uint32_t)hdr_localorder.architecture);
	printf("chipset:         %d\n", (uint32_t)hdr_localorder.chipset);
	printf("board_type:      %d\n", (uint32_t)hdr_localorder.board_type);
	printf("board_class:     %d\n", (uint32_t)hdr_localorder.board_class);
	printf("customer:        %.*s\n", (int)sizeof(hdr_localorder.customer), hdr_localorder.customer);
	printf("Image Sign Type: ");
	switch (hdr_localorder.image_type)
	{
	case 1:
		printf("Intermediate Signed Image (ISI)\n");
		printf("ISI Tail starts: 0x%X\n", hdr_localorder.tail_offset);
		break;

	case 2:
		printf("Fully Signed Image (FSI)\n");
		break;

	default:
		printf("Unsigned Image (UI)\n");
	}

	return is_header_valid;
}

int show_file_header(char *filePath)
{
	FILE *fd = fopen(filePath, "rb");
	if (fd == NULL)
	{
		printf("Error: Could not open file %s.\n", filePath);
		return false;
	}

	struct bin_hdr hdr;
	size_t readSize = fread(&hdr, 1, sizeof(struct bin_hdr), fd);

	if (readSize != sizeof(struct bin_hdr))
	{
		printf("Error: File truncated: %s.\n", filePath);
		return false;
	}

	struct bin_hdr hdr_localorder = hdr;
	ntoh_hdr(&hdr_localorder);

	int result;
	md5_byte_t digest[16];
	if (calc_binl7_digest(fd, hdr_localorder.binl7_len, digest))
	{
		result = bin_hdr_dump(&hdr, digest);
	}
	else
	{
		result = bin_hdr_dump(&hdr, NULL);
	}

	fclose(fd);
	return result;
}

int main(int argc, char **argv)
{
    char *inputFile;

    if (argc == 1)
    {
        showUsage(argv[0]);
        exit(EXIT_FAILURE);
    }
    else if (argc > 1)
        inputFile = argv[1];

    static struct option long_options[] = {
        {"input", required_argument, 0, 'i'},
        {"help", no_argument, 0, '?'},
        {0, 0, 0, 0}};

    int opt;
    int option_index = 0;
    while ((opt = getopt_long(argc, argv, "i:?", long_options, &option_index)) != -1)
    {
        switch (opt)
        {
        case 'i':
            inputFile = optarg;
            break;
        case '?':
            showUsage(argv[0]);
            exit(EXIT_SUCCESS);
        }
    }

    if (!show_file_header(inputFile))
        exit(EXIT_FAILURE);

    return 0;
}