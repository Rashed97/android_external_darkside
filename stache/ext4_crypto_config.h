#ifndef _EXT4_CRYPTO_CONFIG_H
#define _EXT4_CRYPTO_CONFIG_H

/*
 * Definitions that are needed for EXT4 encryption but for some reason
 * don't live in the kernel headers
 */

#define EXT4_MAX_PASSPHRASE_SZ 128
#define EXT4_ENCRYPTION_KEY_TYPE "logon"
#define EXT4_FULL_KEY_DESCRIPTOR_SIZE (EXT4_KEY_DESCRIPTOR_SIZE * 2 + EXT4_KEY_DESC_PREFIX_SIZE)

struct ext4_crypt_options {
    bool verbose;
    char *contents_cipher;
    char *filename_cipher;
    unsigned filename_padding;
    char key_descriptor[EXT4_KEY_DESCRIPTOR_SIZE];
    bool requires_descriptor;
};

static inline
char padding_length_to_flags(unsigned padding)
{
    switch ( padding ) {
        case 4:
            return EXT4_POLICY_FLAGS_PAD_4;
        case 8:
            return EXT4_POLICY_FLAGS_PAD_8;
        case 16:
            return EXT4_POLICY_FLAGS_PAD_16;
        case 32:
            return EXT4_POLICY_FLAGS_PAD_32;
        default:
            fprintf(stderr, "Invalid padding value: %d\n", padding);
            abort();
    }
}

static inline
unsigned flags_to_padding_length(char flags)
{
    return (4 << (flags & EXT4_POLICY_FLAGS_PAD_MASK));
}

struct cipher {
    const char *cipher_name;
    size_t cipher_key_size;
};

static
struct cipher cipher_modes[] = {
    [EXT4_ENCRYPTION_MODE_INVALID] = { "invalid", 0 },
    [EXT4_ENCRYPTION_MODE_AES_256_XTS] = { "aes-256-xts", EXT4_AES_256_XTS_KEY_SIZE },
    [EXT4_ENCRYPTION_MODE_AES_256_GCM] = { "aes-256-gcm", EXT4_AES_256_GCM_KEY_SIZE },
    [EXT4_ENCRYPTION_MODE_AES_256_CBC] = { "aes-256-cbc", EXT4_AES_256_CBC_KEY_SIZE },
    [EXT4_ENCRYPTION_MODE_AES_256_CTS] = { "aes-256-cts", EXT4_AES_256_CTS_KEY_SIZE },
};

#define NR_EXT4_ENCRYPTION_MODES (sizeof(cipher_modes) / sizeof(cipher_modes[0]))

static inline
const char *cipher_mode_to_string(unsigned char mode)
{
    if ( mode >= NR_EXT4_ENCRYPTION_MODES )
        return "invalid";

    return cipher_modes[mode].cipher_name;
}

static inline
char cipher_string_to_mode(const char *cipher)
{
    for ( size_t i = 0; i < NR_EXT4_ENCRYPTION_MODES; i++ ) {
        if ( strcmp(cipher, cipher_modes[i].cipher_name) == 0 )
            return i;
    }

    fprintf(stderr, "Invalid cipher mode: %s\n", cipher);
    abort();
}

static inline
size_t cipher_key_size(const char *cipher)
{
    for ( size_t i = 0; i < NR_EXT4_ENCRYPTION_MODES; i++ ) {
        if ( strcmp(cipher, cipher_modes[i].cipher_name) == 0 )
            return cipher_modes[i].cipher_key_size;
    }

    fprintf(stderr, "Invalid cipher mode: %s\n", cipher);
    abort();
}

typedef char key_desc_t[EXT4_KEY_DESCRIPTOR_SIZE];
typedef char full_key_desc_t[EXT4_FULL_KEY_DESCRIPTOR_SIZE];

int crypto_init();
int container_status(const char *dir_path);
int container_create(const char *dir_path, struct ext4_crypt_options);
int container_attach(const char *dir_path, struct ext4_crypt_options);
int container_detach(const char *dir_path, struct ext4_crypt_options);
void generate_random_name(char *, size_t);
int find_key_by_descriptor(key_desc_t *, key_serial_t *);
int request_key_for_descriptor(key_desc_t *, struct ext4_crypt_options, bool);
int remove_key_for_descriptor(key_desc_t *);

#endif /* _EXT4_CRYPTO_CONFIG_H */
