/*
SAGAR PARAJULI CS26BTKMU11003
Custom key schedule used in aseni and sw implementaion but this specific file not used anywhere, just the modules
*/
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <wmmintrin.h>   /* AES-NI intrinsics (for __m128i / _mm_loadu_si128) */

#define NR 10               /* number of AES rounds */
#define NUM_KEYS (NR + 1)   /* k_{-1}, k_0, ..., k_9 => 11 keys */

/* Round constants RC_1 .. RC_10, applied to byte 0 of the 128-bit state only */
static const uint8_t RCON[NR] = {
    0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1B, 0x36
};

//standard shift rows impelmentation of aes
static void shift_rows(const uint8_t in[16], uint8_t out[16]) {
    for (int c = 0; c < 4; c++) {
        for (int r = 0; r < 4; r++) {
            out[r + 4 * c] = in[r + 4 * ((c + r) % 4)];
        }
    }
}

//same keygen procedure
void generate_round_keys(const uint8_t master_key[16], uint8_t keys[NUM_KEYS][16]) {
    memcpy(keys[0], master_key, 16);
    for (int i = 1; i <= NR; i++) {
        uint8_t shifted[16];
        shift_rows(keys[i - 1], shifted);
        memcpy(keys[i], shifted, 16);
        keys[i][0] ^= RCON[i - 1];
    }
}

void load_round_keys(const uint8_t keys[NUM_KEYS][16], __m128i rk[NUM_KEYS]) {
    for (int i = 0; i < NUM_KEYS; i++) {
        rk[i] = _mm_loadu_si128((const __m128i *)keys[i]);
    }
}


//sanity check
static void print_hex(const char *label, const uint8_t *data, int len) {
    printf("%-6s: ", label);
    for (int i = 0; i < len; i++) printf("%02x", data[i]);
    printf("\n");
}

int main(void) {
    uint8_t master_key[16] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f
    };

    uint8_t keys_bytes[NUM_KEYS][16];
    generate_round_keys(master_key, keys_bytes);

    printf("=== AES-NI custom key schedule ===\n");
    print_hex("k_-1", keys_bytes[0], 16);
    for (int i = 1; i <= NR; i++) {
        char label[8];
        snprintf(label, sizeof(label), "k_%d", i - 1);
        print_hex(label, keys_bytes[i], 16);
    }

    /* Also load into __m128i to show it's ready for AES-NI encrypt/decrypt */
    __m128i rk[NUM_KEYS];
    load_round_keys(keys_bytes, rk);
    (void)rk; /* silence unused-variable warning; rk is what encrypt/decrypt will consume */

    return 0;
}