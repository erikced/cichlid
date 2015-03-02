add_library( libcichlid
    cichlid_hash_common.h
    cichlid_hash_crc32.h
    cichlid_hash_crc32.c
    cichlid_hash_md5.h
    cichlid_hash_md5.c
    cichlid_hash_sha2_32.h
    cichlid_hash_sha2_32.c
    cichlid_hash_sha2_64.h
    cichlid_hash_sha2_64.c
    cichlid_hash_sha224.h
    cichlid_hash_sha224.c
    cichlid_hash_sha256.h
    cichlid_hash_sha256.c
    cichlid_hash_sha384.h
    cichlid_hash_sha384.c
    cichlid_hash_sha512.h
    cichlid_hash_sha512.c
)

add_executable( cichlid
    main.c
)

target_link_libraries( cichlid
    libcichlid
)
