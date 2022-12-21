#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <netinet/in.h>
#include <time.h>
#include "md5.h"
#include "ruckus-fw.h"

void ntoh_hdr(struct bin_hdr *binhdr)
{
	if (binhdr->hdr_version < 0x100)
		return; // already local endian

	binhdr->next_image = ntohl(binhdr->next_image);
	binhdr->load_address = ntohl(binhdr->load_address);
	binhdr->entry_point = ntohl(binhdr->entry_point);
	binhdr->timestamp = ntohl(binhdr->timestamp);
	binhdr->binl7_len = ntohl(binhdr->binl7_len);
	binhdr->hdr_version = ntohs(binhdr->hdr_version);
	binhdr->hdr_cksum = ntohs(binhdr->hdr_cksum);
	binhdr->image_type = ntohl(binhdr->image_type);
	binhdr->tail_offset = ntohl(binhdr->tail_offset);
}

void hton_hdr(struct bin_hdr *binhdr)
{
	if (binhdr->hdr_version > 0xff)
		return; // already network endian
	if (binhdr->hdr_version == htons(binhdr->hdr_version))
		return; // local endian == network endian

	binhdr->next_image = htonl(binhdr->next_image);
	binhdr->load_address = htonl(binhdr->load_address);
	binhdr->entry_point = htonl(binhdr->entry_point);
	binhdr->timestamp = htonl(binhdr->timestamp);
	binhdr->binl7_len = htonl(binhdr->binl7_len);
	binhdr->hdr_version = htons(binhdr->hdr_version);
	binhdr->hdr_cksum = htons(binhdr->hdr_cksum);
	binhdr->image_type = htonl(binhdr->image_type);
	binhdr->tail_offset = htonl(binhdr->tail_offset);
}

static uint16_t in_cksum(const void *data, size_t bytes)
{
	uint32_t running_xsum = 0;
	const uint16_t *running_buffer = data;

	while (bytes > 1)
	{
		running_xsum += ntohs(*running_buffer);
		running_buffer++;
		bytes -= 2;
	}
	if (bytes == 1)
		running_xsum += ntohs(*((const uint8_t *)running_buffer)) << 8;

	while (running_xsum >> 16)
		running_xsum = (running_xsum & 0xffff) + (running_xsum >> 16);

	return ~htons((uint16_t)running_xsum);
}

uint16_t calc_hdr_cksum(struct bin_hdr *binhdr)
{
	struct bin_hdr temp_hdr = *binhdr;
	hton_hdr(&temp_hdr);
	temp_hdr.hdr_cksum = 0;
	return ntohs(in_cksum(&temp_hdr, sizeof(struct bin_hdr)));
}

int calc_binl7_digest(FILE *fd, size_t binl7_bytes, md5_byte_t *digest)
{
	md5_state_t state;
	md5_init(&state);

	md5_byte_t buffer[0x1000];
	size_t bytes_read;
	while (binl7_bytes > 0 && (bytes_read = fread(buffer, 1, sizeof(buffer), fd)) > 0)
	{
		if (bytes_read > binl7_bytes)
			bytes_read = binl7_bytes;
		md5_append(&state, (const md5_byte_t *)buffer, bytes_read);
		binl7_bytes -= bytes_read;
	}

	if (binl7_bytes > 0)
		return false;

	md5_finish(&state, digest);
	return true;
}
