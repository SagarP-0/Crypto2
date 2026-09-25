/*
 * gcc -O3 -maes -msse2 -o benchmark benchmark.c
 * Same methods used in aesni.c and AESStandard.c used here 
 * just kept the bencharmark separate for clairty 
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <x86intrin.h>   /* __rdtsc, __rdtscp */
#include <wmmintrin.h>   /* AES-NI intrinsics */

#define NR 10
#define NUM_KEYS (NR + 1)   /* k_{-1}, k_0 .. k_9 */

//identical methods of two AES implementation 
static const uint8_t RCON[NR] = {
    0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1B, 0x36
};

static void ks_shift_rows(const uint8_t in[16], uint8_t out[16]) {
    for (int c = 0; c < 4; c++)
        for (int r = 0; r < 4; r++)
            out[r + 4 * c] = in[r + 4 * ((c + r) % 4)];
}

static void generate_round_keys(const uint8_t master_key[16], uint8_t keys[NUM_KEYS][16]) {
    memcpy(keys[0], master_key, 16);
    for (int i = 1; i <= NR; i++) {
        uint8_t shifted[16];
        ks_shift_rows(keys[i - 1], shifted);
        memcpy(keys[i], shifted, 16);
        keys[i][0] ^= RCON[i - 1];
    }
}

//identical AES sw implementation
static const uint8_t SBOX[256] = {
    0x63,0x7c,0x77,0x7b,0xf2,0x6b,0x6f,0xc5,0x30,0x01,0x67,0x2b,0xfe,0xd7,0xab,0x76,
    0xca,0x82,0xc9,0x7d,0xfa,0x59,0x47,0xf0,0xad,0xd4,0xa2,0xaf,0x9c,0xa4,0x72,0xc0,
    0xb7,0xfd,0x93,0x26,0x36,0x3f,0xf7,0xcc,0x34,0xa5,0xe5,0xf1,0x71,0xd8,0x31,0x15,
    0x04,0xc7,0x23,0xc3,0x18,0x96,0x05,0x9a,0x07,0x12,0x80,0xe2,0xeb,0x27,0xb2,0x75,
    0x09,0x83,0x2c,0x1a,0x1b,0x6e,0x5a,0xa0,0x52,0x3b,0xd6,0xb3,0x29,0xe3,0x2f,0x84,
    0x53,0xd1,0x00,0xed,0x20,0xfc,0xb1,0x5b,0x6a,0xcb,0xbe,0x39,0x4a,0x4c,0x58,0xcf,
    0xd0,0xef,0xaa,0xfb,0x43,0x4d,0x33,0x85,0x45,0xf9,0x02,0x7f,0x50,0x3c,0x9f,0xa8,
    0x51,0xa3,0x40,0x8f,0x92,0x9d,0x38,0xf5,0xbc,0xb6,0xda,0x21,0x10,0xff,0xf3,0xd2,
    0xcd,0x0c,0x13,0xec,0x5f,0x97,0x44,0x17,0xc4,0xa7,0x7e,0x3d,0x64,0x5d,0x19,0x73,
    0x60,0x81,0x4f,0xdc,0x22,0x2a,0x90,0x88,0x46,0xee,0xb8,0x14,0xde,0x5e,0x0b,0xdb,
    0xe0,0x32,0x3a,0x0a,0x49,0x06,0x24,0x5c,0xc2,0xd3,0xac,0x62,0x91,0x95,0xe4,0x79,
    0xe7,0xc8,0x37,0x6d,0x8d,0xd5,0x4e,0xa9,0x6c,0x56,0xf4,0xea,0x65,0x7a,0xae,0x08,
    0xba,0x78,0x25,0x2e,0x1c,0xa6,0xb4,0xc6,0xe8,0xdd,0x74,0x1f,0x4b,0xbd,0x8b,0x8a,
    0x70,0x3e,0xb5,0x66,0x48,0x03,0xf6,0x0e,0x61,0x35,0x57,0xb9,0x86,0xc1,0x1d,0x9e,
    0xe1,0xf8,0x98,0x11,0x69,0xd9,0x8e,0x94,0x9b,0x1e,0x87,0xe9,0xce,0x55,0x28,0xdf,
    0x8c,0xa1,0x89,0x0d,0xbf,0xe6,0x42,0x68,0x41,0x99,0x2d,0x0f,0xb0,0x54,0xbb,0x16
};

static const uint8_t INV_SBOX[256] = {
    0x52,0x09,0x6a,0xd5,0x30,0x36,0xa5,0x38,0xbf,0x40,0xa3,0x9e,0x81,0xf3,0xd7,0xfb,
    0x7c,0xe3,0x39,0x82,0x9b,0x2f,0xff,0x87,0x34,0x8e,0x43,0x44,0xc4,0xde,0xe9,0xcb,
    0x54,0x7b,0x94,0x32,0xa6,0xc2,0x23,0x3d,0xee,0x4c,0x95,0x0b,0x42,0xfa,0xc3,0x4e,
    0x08,0x2e,0xa1,0x66,0x28,0xd9,0x24,0xb2,0x76,0x5b,0xa2,0x49,0x6d,0x8b,0xd1,0x25,
    0x72,0xf8,0xf6,0x64,0x86,0x68,0x98,0x16,0xd4,0xa4,0x5c,0xcc,0x5d,0x65,0xb6,0x92,
    0x6c,0x70,0x48,0x50,0xfd,0xed,0xb9,0xda,0x5e,0x15,0x46,0x57,0xa7,0x8d,0x9d,0x84,
    0x90,0xd8,0xab,0x00,0x8c,0xbc,0xd3,0x0a,0xf7,0xe4,0x58,0x05,0xb8,0xb3,0x45,0x06,
    0xd0,0x2c,0x1e,0x8f,0xca,0x3f,0x0f,0x02,0xc1,0xaf,0xbd,0x03,0x01,0x13,0x8a,0x6b,
    0x3a,0x91,0x11,0x41,0x4f,0x67,0xdc,0xea,0x97,0xf2,0xcf,0xce,0xf0,0xb4,0xe6,0x73,
    0x96,0xac,0x74,0x22,0xe7,0xad,0x35,0x85,0xe2,0xf9,0x37,0xe8,0x1c,0x75,0xdf,0x6e,
    0x47,0xf1,0x1a,0x71,0x1d,0x29,0xc5,0x89,0x6f,0xb7,0x62,0x0e,0xaa,0x18,0xbe,0x1b,
    0xfc,0x56,0x3e,0x4b,0xc6,0xd2,0x79,0x20,0x9a,0xdb,0xc0,0xfe,0x78,0xcd,0x5a,0xf4,
    0x1f,0xdd,0xa8,0x33,0x88,0x07,0xc7,0x31,0xb1,0x12,0x10,0x59,0x27,0x80,0xec,0x5f,
    0x60,0x51,0x7f,0xa9,0x19,0xb5,0x4a,0x0d,0x2d,0xe5,0x7a,0x9f,0x93,0xc9,0x9c,0xef,
    0xa0,0xe0,0x3b,0x4d,0xae,0x2a,0xf5,0xb0,0xc8,0xeb,0xbb,0x3c,0x83,0x53,0x99,0x61,
    0x17,0x2b,0x04,0x7e,0xba,0x77,0xd6,0x26,0xe1,0x69,0x14,0x63,0x55,0x21,0x0c,0x7d
};

static void sw_shift_rows(uint8_t state[16]) {
    uint8_t in[16]; memcpy(in, state, 16);
    for (int c = 0; c < 4; c++)
        for (int r = 0; r < 4; r++)
            state[r + 4 * c] = in[r + 4 * ((c + r) % 4)];
}

static void sw_inv_shift_rows(uint8_t state[16]) {
    uint8_t in[16]; memcpy(in, state, 16);
    for (int c = 0; c < 4; c++)
        for (int r = 0; r < 4; r++)
            state[r + 4 * ((c + r) % 4)] = in[r + 4 * c];
}

static void sw_sub_bytes(uint8_t s[16])     { for (int i = 0; i < 16; i++) s[i] = SBOX[s[i]]; }
static void sw_inv_sub_bytes(uint8_t s[16]) { for (int i = 0; i < 16; i++) s[i] = INV_SBOX[s[i]]; }
static void sw_add_round_key(uint8_t s[16], const uint8_t k[16]) { for (int i = 0; i < 16; i++) s[i] ^= k[i]; }

static uint8_t gmul(uint8_t a, uint8_t b) {
    uint8_t p = 0;
    for (int i = 0; i < 8; i++) {
        if (b & 1) p ^= a;
        uint8_t hi = a & 0x80;
        a <<= 1;
        if (hi) a ^= 0x1B;
        b >>= 1;
    }
    return p;
}

static void sw_mix_columns(uint8_t s[16]) {
    for (int c = 0; c < 4; c++) {
        uint8_t s0=s[4*c],s1=s[4*c+1],s2=s[4*c+2],s3=s[4*c+3];
        s[4*c+0]=(uint8_t)(gmul(s0,2)^gmul(s1,3)^s2^s3);
        s[4*c+1]=(uint8_t)(s0^gmul(s1,2)^gmul(s2,3)^s3);
        s[4*c+2]=(uint8_t)(s0^s1^gmul(s2,2)^gmul(s3,3));
        s[4*c+3]=(uint8_t)(gmul(s0,3)^s1^s2^gmul(s3,2));
    }
}

static void sw_inv_mix_columns(uint8_t s[16]) {
    for (int c = 0; c < 4; c++) {
        uint8_t s0=s[4*c],s1=s[4*c+1],s2=s[4*c+2],s3=s[4*c+3];
        s[4*c+0]=(uint8_t)(gmul(s0,14)^gmul(s1,11)^gmul(s2,13)^gmul(s3,9));
        s[4*c+1]=(uint8_t)(gmul(s0,9)^gmul(s1,14)^gmul(s2,11)^gmul(s3,13));
        s[4*c+2]=(uint8_t)(gmul(s0,13)^gmul(s1,9)^gmul(s2,14)^gmul(s3,11));
        s[4*c+3]=(uint8_t)(gmul(s0,11)^gmul(s1,13)^gmul(s2,9)^gmul(s3,14));
    }
}

static void sw_encrypt_block(const uint8_t pt[16], const uint8_t keys[NUM_KEYS][16], uint8_t ct[16]) {
    uint8_t state[16]; memcpy(state, pt, 16);
    sw_add_round_key(state, keys[0]);
    for (int r = 1; r <= NR - 1; r++) {
        sw_sub_bytes(state);
        sw_shift_rows(state);
        sw_mix_columns(state);
        sw_add_round_key(state, keys[r]);
    }
    sw_sub_bytes(state);
    sw_shift_rows(state);
    sw_add_round_key(state, keys[NR]);
    memcpy(ct, state, 16);
}

static void sw_decrypt_block(const uint8_t ct[16], const uint8_t keys[NUM_KEYS][16], uint8_t pt[16]) {
    uint8_t state[16]; memcpy(state, ct, 16);
    sw_add_round_key(state, keys[NR]);
    for (int r = NR - 1; r >= 1; r--) {
        sw_inv_shift_rows(state);
        sw_inv_sub_bytes(state);
        sw_add_round_key(state, keys[r]);
        sw_inv_mix_columns(state);
    }
    sw_inv_shift_rows(state);
    sw_inv_sub_bytes(state);
    sw_add_round_key(state, keys[0]);
    memcpy(pt, state, 16);
}


//Identical AES NI core
static void load_round_keys(const uint8_t keys[NUM_KEYS][16], __m128i rk[NUM_KEYS]) {
    for (int i = 0; i < NUM_KEYS; i++)
        rk[i] = _mm_loadu_si128((const __m128i *)keys[i]);
}

static inline __m128i aesni_encrypt_block(__m128i pt, const __m128i rk[NUM_KEYS]) {
    __m128i state = _mm_xor_si128(pt, rk[0]);
    for (int i = 1; i <= NR - 1; i++) state = _mm_aesenc_si128(state, rk[i]);
    state = _mm_aesenclast_si128(state, rk[NR]);
    return state;
}

static inline __m128i aesni_decrypt_block(__m128i ct, const __m128i rk[NUM_KEYS]) {
    __m128i state = _mm_xor_si128(ct, rk[NR]);
    for (int i = NR - 1; i >= 1; i--) state = _mm_aesdec_si128(state, _mm_aesimc_si128(rk[i]));
    state = _mm_aesdeclast_si128(state, rk[0]);
    return state;
}

//benchmark parameters
#define NUM_SIZES 5
static const size_t SIZES[NUM_SIZES] = { 1024, 2048, 4096, 32768, 65536 };
static const char  *SIZE_LABELS[NUM_SIZES] = { "1 KB", "2 KB", "4 KB", "32 KB", "64 KB" };

#define WARMUP 5   /* untimed runs to warm caches / branch predictors  */
#define RUNS   31  /* timed runs per (impl, op, size); we take the median   */

static volatile uint8_t g_sink = 0; /* prevents dead-code elimination */

static double now_seconds(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;
}

//serialised rdtsc parameters as per Intel's standard procedures
static inline uint64_t serialized_rdtsc_start(void) {
    unsigned int aux;
    __asm__ __volatile__("cpuid" ::: "eax","ebx","ecx","edx");
    return __rdtscp(&aux);
}
static inline uint64_t serialized_rdtsc_end(void) {
    unsigned int aux;
    uint64_t t = __rdtscp(&aux);
    __asm__ __volatile__("cpuid" ::: "eax","ebx","ecx","edx");
    return t;
}

static int cmp_double(const void *a, const void *b) {
    double da = *(const double *)a, db = *(const double *)b;
    return (da > db) - (da < db);
}
static int cmp_u64(const void *a, const void *b) {
    uint64_t ua = *(const uint64_t *)a, ub = *(const uint64_t *)b;
    return (ua > ub) - (ua < ub);
}

static double median_double(double *v, int n) {
    qsort(v, n, sizeof(double), cmp_double);
    return (n % 2) ? v[n/2] : 0.5 * (v[n/2 - 1] + v[n/2]);
}
static uint64_t median_u64(uint64_t *v, int n) {
    qsort(v, n, sizeof(uint64_t), cmp_u64);
    return (n % 2) ? v[n/2] : (v[n/2 - 1] + v[n/2]) / 2;
}

typedef struct {
    double   time_sec[RUNS];
    uint64_t cycles[RUNS];
} RunStats;

//one-shot processing over a whole buffer for each implementation 

static void sw_encrypt_buffer(const uint8_t *in, uint8_t *out, size_t nblocks,
                               const uint8_t keys[NUM_KEYS][16]) {
    for (size_t i = 0; i < nblocks; i++)
        sw_encrypt_block(in + 16*i, keys, out + 16*i);
}
static void sw_decrypt_buffer(const uint8_t *in, uint8_t *out, size_t nblocks,
                               const uint8_t keys[NUM_KEYS][16]) {
    for (size_t i = 0; i < nblocks; i++)
        sw_decrypt_block(in + 16*i, keys, out + 16*i);
}
static void ni_encrypt_buffer(const uint8_t *in, uint8_t *out, size_t nblocks,
                               const __m128i rk[NUM_KEYS]) {
    for (size_t i = 0; i < nblocks; i++) {
        __m128i pt = _mm_loadu_si128((const __m128i *)(in + 16*i));
        __m128i ctb = aesni_encrypt_block(pt, rk);
        _mm_storeu_si128((__m128i *)(out + 16*i), ctb);
    }
}
static void ni_decrypt_buffer(const uint8_t *in, uint8_t *out, size_t nblocks,
                               const __m128i rk[NUM_KEYS]) {
    for (size_t i = 0; i < nblocks; i++) {
        __m128i ctb = _mm_loadu_si128((const __m128i *)(in + 16*i));
        __m128i pt = aesni_decrypt_block(ctb, rk);
        _mm_storeu_si128((__m128i *)(out + 16*i), pt);
    }
}

//Generic timed runner: calls `op` WARMUP+RUNS times, fills RunStats. 
typedef void (*OpFn)(const uint8_t *, uint8_t *, size_t, const void *);

static void run_timed(void (*fn_sw)(const uint8_t*, uint8_t*, size_t, const uint8_t (*)[16]),
                       void (*fn_ni)(const uint8_t*, uint8_t*, size_t, const __m128i*),
                       int use_ni,
                       const uint8_t *in, uint8_t *out, size_t nblocks,
                       const uint8_t keys[NUM_KEYS][16], const __m128i *rk,
                       RunStats *stats) {
    for (int r = 0; r < WARMUP; r++) {
        if (use_ni) fn_ni(in, out, nblocks, rk);
        else        fn_sw(in, out, nblocks, keys);
    }
    for (int r = 0; r < RUNS; r++) {
        double t0 = now_seconds();
        uint64_t c0 = serialized_rdtsc_start();
        if (use_ni) fn_ni(in, out, nblocks, rk);
        else        fn_sw(in, out, nblocks, keys);
        uint64_t c1 = serialized_rdtsc_end();
        double t1 = now_seconds();
        stats->time_sec[r] = t1 - t0;
        stats->cycles[r]   = c1 - c0;
        /* touch output so the whole buffer is a real dependency, and the
         * compiler/CPU cannot hoist the loop out of the timing window */
        g_sink ^= out[(nblocks - 1) * 16];
    }
}

int main(void) {
    /* Fixed key so runs are reproducible; content of key/plaintext does
     * not affect AES timing (no data-dependent branches in either impl). */
    uint8_t master_key[16];
    for (int i = 0; i < 16; i++) master_key[i] = (uint8_t)(0x11 * (i + 1));

    uint8_t keys[NUM_KEYS][16];
    generate_round_keys(master_key, keys);      /* NOT timed, done once */

    __m128i rk[NUM_KEYS];
    load_round_keys(keys, rk);                  /* NOT timed, done once */

    double  thr_sw_enc[NUM_SIZES], thr_ni_enc[NUM_SIZES];
    double  thr_sw_dec[NUM_SIZES], thr_ni_dec[NUM_SIZES];
    double  cpb_sw_enc[NUM_SIZES], cpb_ni_enc[NUM_SIZES];
    double  cpb_sw_dec[NUM_SIZES], cpb_ni_dec[NUM_SIZES];

    printf("Method: median of %d timed runs (%d untimed warmup runs discarded)\n", RUNS, WARMUP);
    printf("Cycle counts via serialized RDTSCP; wall time via CLOCK_MONOTONIC.\n\n");

    for (int s = 0; s < NUM_SIZES; s++) {
        size_t nbytes   = SIZES[s];
        size_t nblocks  = nbytes / 16;

        uint8_t *plain  = malloc(nbytes);
        uint8_t *cipher_sw = malloc(nbytes);
        uint8_t *cipher_ni = malloc(nbytes);
        uint8_t *decoded   = malloc(nbytes);

        srand(12345u + (unsigned)s);
        for (size_t i = 0; i < nbytes; i++) plain[i] = (uint8_t)rand();

        // Produce valid ciphertext to decrypt (not timed).
        sw_encrypt_buffer(plain, cipher_sw, nblocks, keys);
        ni_encrypt_buffer(plain, cipher_ni, nblocks, rk);

        RunStats st;

        //Software encryption
        run_timed(sw_encrypt_buffer, NULL, 0, plain, cipher_sw, nblocks, keys, rk, &st);
        double t = median_double(st.time_sec, RUNS);
        uint64_t c = median_u64(st.cycles, RUNS);
        thr_sw_enc[s] = (nbytes / 1024.0) / t;      /* KB/s -> convert later */
        cpb_sw_enc[s] = (double)c / (double)nbytes;

        // AES-NI encryption 
        run_timed(NULL, ni_encrypt_buffer, 1, plain, cipher_ni, nblocks, keys, rk, &st);
        t = median_double(st.time_sec, RUNS);
        c = median_u64(st.cycles, RUNS);
        thr_ni_enc[s] = (nbytes / 1024.0) / t;
        cpb_ni_enc[s] = (double)c / (double)nbytes;

        //Aes Software decryption 
        run_timed(sw_decrypt_buffer, NULL, 0, cipher_sw, decoded, nblocks, keys, rk, &st);
        t = median_double(st.time_sec, RUNS);
        c = median_u64(st.cycles, RUNS);
        thr_sw_dec[s] = (nbytes / 1024.0) / t;
        cpb_sw_dec[s] = (double)c / (double)nbytes;
        if (memcmp(decoded, plain, nbytes) != 0) fprintf(stderr, "WARNING: software roundtrip mismatch at %s\n", SIZE_LABELS[s]);

        //AES-NI decryption 
        run_timed(NULL, ni_decrypt_buffer, 1, cipher_ni, decoded, nblocks, keys, rk, &st);
        t = median_double(st.time_sec, RUNS);
        c = median_u64(st.cycles, RUNS);
        thr_ni_dec[s] = (nbytes / 1024.0) / t;
        cpb_ni_dec[s] = (double)c / (double)nbytes;
        if (memcmp(decoded, plain, nbytes) != 0) fprintf(stderr, "WARNING: AES-NI roundtrip mismatch at %s\n", SIZE_LABELS[s]);

        free(plain); free(cipher_sw); free(cipher_ni); free(decoded);
    }

    //results
    printf("Throughput (MB/s)\n");
    printf("%-20s", "Implementation");
    for (int s = 0; s < NUM_SIZES; s++) printf("%12s", SIZE_LABELS[s]);
    printf("\n");

    printf("%-20s", "Software Encryption");
    for (int s = 0; s < NUM_SIZES; s++) printf("%12.2f", thr_sw_enc[s] / 1024.0);
    printf("\n");
    printf("%-20s", "AES-NI Encryption");
    for (int s = 0; s < NUM_SIZES; s++) printf("%12.2f", thr_ni_enc[s] / 1024.0);
    printf("\n");
    printf("%-20s", "Software Decryption");
    for (int s = 0; s < NUM_SIZES; s++) printf("%12.2f", thr_sw_dec[s] / 1024.0);
    printf("\n");
    printf("%-20s", "AES-NI Decryption");
    for (int s = 0; s < NUM_SIZES; s++) printf("%12.2f", thr_ni_dec[s] / 1024.0);
    printf("\n\n");

    printf("Cycles per Byte (CPB)\n");
    printf("%-20s", "Implementation");
    for (int s = 0; s < NUM_SIZES; s++) printf("%12s", SIZE_LABELS[s]);
    printf("\n");

    printf("%-20s", "Software Encryption");
    for (int s = 0; s < NUM_SIZES; s++) printf("%12.2f", cpb_sw_enc[s]);
    printf("\n");
    printf("%-20s", "AES-NI Encryption");
    for (int s = 0; s < NUM_SIZES; s++) printf("%12.2f", cpb_ni_enc[s]);
    printf("\n");
    printf("%-20s", "Software Decryption");
    for (int s = 0; s < NUM_SIZES; s++) printf("%12.2f", cpb_sw_dec[s]);
    printf("\n");
    printf("%-20s", "AES-NI Decryption");
    for (int s = 0; s < NUM_SIZES; s++) printf("%12.2f", cpb_ni_dec[s]);
    printf("\n");

    fprintf(stderr, "(sink=%d)\n", g_sink); /* silence -Wunused, keep g_sink live */
    return 0;
}