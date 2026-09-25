/*
 * SAGAR PARAJULI CS26BTKMU11003
 * object file usuage
 *   ./aesni enc <32-hex-char key> <32-hex-char plaintext>
 *   ./aesni dec <32-hex-char key> <32-hex-char ciphertext>
 *
 * Example:
 *   ./aesni enc 000102030405060708090a0b0c0d0e0f 00112233445566778899aabbccddeeff
 *
 * Prints the round keys and the resulting ciphertext/plaintext in hex,
 * so the output can be diffed directly against the standard
 * implementation's output for the same key/plaintext (or key/ciphertext).
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <wmmintrin.h>   /* AES-NI intrinsics */

#define NR 10               /* number of AES rounds */
#define NUM_KEYS (NR + 1)   /* k_{-1}, k_0, ..., k_9 => 11 keys */

// Round constants RC_1 .. RC_10, applied to byte 0 of the 128-bit state only 
static const uint8_t RCON[NR] = {
    0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1B, 0x36
};

//custom kkey schedule similar to keygen.c 

static void shift_rows(const uint8_t in[16], uint8_t out[16]) {
    for (int c = 0; c < 4; c++) {
        for (int r = 0; r < 4; r++) {
            out[r + 4 * c] = in[r + 4 * ((c + r) % 4)];
        }
    }
}

static void generate_round_keys(const uint8_t master_key[16], uint8_t keys[NUM_KEYS][16]) {
    memcpy(keys[0], master_key, 16);
    for (int i = 1; i <= NR; i++) {
        uint8_t shifted[16];
        shift_rows(keys[i - 1], shifted);
        memcpy(keys[i], shifted, 16);
        keys[i][0] ^= RCON[i - 1];
    }
}

static void load_round_keys(const uint8_t keys[NUM_KEYS][16], __m128i rk[NUM_KEYS]) {
    for (int i = 0; i < NUM_KEYS; i++) {
        rk[i] = _mm_loadu_si128((const __m128i *)keys[i]);
    }
}

//encryption and decysprtion blocks

/*
 * rk[0]      = k_{-1}  (whitening key)
 * rk[1..9]   = k_0..k_8 (9 main rounds, aesenc)
 * rk[10]     = k_9      (final round, aesenclast)
 */
static __m128i aesni_encrypt_block(__m128i plaintext, const __m128i rk[NUM_KEYS]) {
    __m128i state = _mm_xor_si128(plaintext, rk[0]);
    for (int i = 1; i <= NR - 1; i++) {          /* i = 1..9 -> rk[1..9] */
        state = _mm_aesenc_si128(state, rk[i]);
    }
    state = _mm_aesenclast_si128(state, rk[NR]); /* rk[10] */
    return state;
}

/*
 * Equivalent-inverse-cipher decryption:
 * Encryption key order: rk[0], rk[1], ..., rk[9], rk[10]
 * Decryption key order: rk[10], IMC(rk[9]), ..., IMC(rk[1]), rk[0]
 */
static __m128i aesni_decrypt_block(__m128i ciphertext, const __m128i rk[NUM_KEYS]) {
    __m128i state = _mm_xor_si128(ciphertext, rk[NR]); /* XOR with k_9 = rk[10] */
    for (int i = NR - 1; i >= 1; i--) {                /* i = 9..1 -> rk[9..1] */
        state = _mm_aesdec_si128(state, _mm_aesimc_si128(rk[i]));
    }
    state = _mm_aesdeclast_si128(state, rk[0]);        /* final with k_{-1} */
    return state;
}

//hex to bytes
static int hex_to_bytes(const char *hex, uint8_t out[16]) {
    if (strlen(hex) != 32) return 0;
    for (int i = 0; i < 16; i++) {
        unsigned int byte;
        if (sscanf(hex + 2 * i, "%2x", &byte) != 1) return 0;
        out[i] = (uint8_t)byte;
    }
    return 1;
}

static void print_hex_line(const char *label, const uint8_t *data, int len) {
    printf("%-11s: ", label);
    for (int i = 0; i < len; i++) printf("%02x", data[i]);
    printf("\n");
}

//main arugments to pass key and plaintext
int main(int argc, char *argv[]) {
    if (argc != 4 || (strcmp(argv[1], "enc") != 0 && strcmp(argv[1], "dec") != 0)) {
        fprintf(stderr,
            "Usage:\n"
            "  %s enc <32-hex key> <32-hex plaintext>\n"
            "  %s dec <32-hex key> <32-hex ciphertext>\n",
            argv[0], argv[0]);
        return 1;
    }

    int is_encrypt = (strcmp(argv[1], "enc") == 0);

    uint8_t master_key[16], input_block[16];
    if (!hex_to_bytes(argv[2], master_key)) {
        fprintf(stderr, "Error: key must be exactly 32 hex characters (16 bytes)\n");
        return 1;
    }
    if (!hex_to_bytes(argv[3], input_block)) {
        fprintf(stderr, "Error: input must be exactly 32 hex characters (16 bytes)\n");
        return 1;
    }

    uint8_t keys_bytes[NUM_KEYS][16];
    generate_round_keys(master_key, keys_bytes);

    printf("=== AES-NI round keys ===\n");
    print_hex_line("k_-1", keys_bytes[0], 16);
    for (int i = 1; i <= NR; i++) {
        char label[8];
        snprintf(label, sizeof(label), "k_%d", i - 1);
        print_hex_line(label, keys_bytes[i], 16);
    }

    __m128i rk[NUM_KEYS];
    load_round_keys(keys_bytes, rk);

    __m128i in_block = _mm_loadu_si128((const __m128i *)input_block);
    __m128i out_block = is_encrypt ? aesni_encrypt_block(in_block, rk)
                                    : aesni_decrypt_block(in_block, rk);

    uint8_t out_bytes[16];
    _mm_storeu_si128((__m128i *)out_bytes, out_block);

    printf("\n=== AES-NI %s ===\n", is_encrypt ? "encryption" : "decryption");
    print_hex_line("key", master_key, 16);
    print_hex_line(is_encrypt ? "plaintext" : "ciphertext", input_block, 16);
    print_hex_line(is_encrypt ? "ciphertext" : "plaintext", out_bytes, 16);

    return 0;
}