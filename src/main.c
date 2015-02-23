#include "cichlid_hash_crc32.h"
#include "cichlid_hash_md5.h"
#include "cichlid_hash_sha224.h"
#include "cichlid_hash_sha256.h"

#include <stdlib.h>
#include <stdio.h>

static int compute_checksum(const char *filename);

int main(int argc, char* argv[])
{
    int rv;
    if (argc == 1) {
        printf("Usage: %s <filename>\n", argv[0]);
        rv = 1;
    } else {
        rv = compute_checksum(argv[1]);
    }
    return 0;
}


static int compute_checksum(const char *filename)
{
    int rv = 0;
    char buf[1024];
    size_t fread ( void * ptr, size_t size, size_t count, FILE * stream );
    FILE *fid = fopen(filename, "r");
    if (fid == NULL) {
        rv = 2;
    } else {
        CichlidHashCrc32 crc32;
        cichlid_hash_crc32_init(&crc32);
        CichlidHashMd5 md5;
        cichlid_hash_md5_init(&md5);
        CichlidHashSha224 sha224;
        cichlid_hash_sha224_init(&sha224);
        CichlidHashSha256 sha256;
        cichlid_hash_sha256_init(&sha256);

        while (!feof(fid)) {
            size_t read_elems = fread(buf, sizeof(*buf), sizeof(buf), fid);
            cichlid_hash_crc32_update(&crc32, buf, read_elems);
            cichlid_hash_md5_update(&md5, buf, read_elems);
            cichlid_hash_sha224_update(&sha224, buf, read_elems);
            cichlid_hash_sha256_update(&sha256, buf, read_elems);
        }

        printf("Hashes of \"%s\"\n", filename);
        char *hash_string = cichlid_hash_crc32_get_hash_string(&crc32);
        printf(" CRC32: %s\n", hash_string);
        free(hash_string);
        hash_string = cichlid_hash_md5_get_hash_string(&md5);
        printf("   MD5: %s\n", hash_string);
        free(hash_string);
        hash_string = cichlid_hash_sha224_get_hash_string(&sha224);
        printf("SHA224: %s\n", hash_string);
        free(hash_string);
        hash_string = cichlid_hash_sha256_get_hash_string(&sha256);
        printf("SHA256: %s\n", hash_string);
        free(hash_string);
    }
    return rv;
}
