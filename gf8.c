#include <stdio.h>
#include <stdint.h>

#define FIELD_BITS 3
#define FIELD_SIZE (1 << FIELD_BITS)   /* 8 elements: 0..7 */
#define REDUCTION_POLY 0x0B            /* x^3 + x + 1 = 1011 */
#define OVERFLOW_BIT   0x08            /* bit representing the x^3 term */

/*
 * Multiply two elements a, b of GF(2^3), represented as 3-bit integers,
 * using shift-and-XOR (the F2-polynomial equivalent of shift-and-add),
 * reducing modulo P(x) after every left shift that produces an x^3 term.
 */
static uint8_t gf8_mult(uint8_t a, uint8_t b) {
    uint8_t result = 0;
    for (int i = 0; i < FIELD_BITS; i++) {
        if (b & 1) {
            result ^= a;              /* add a * x^i into the result (mod 2) */
        }
        b >>= 1;
        a <<= 1;                       /* multiply a by x */
        if (a & OVERFLOW_BIT) {        /* degree reached 3: reduce */
            a ^= REDUCTION_POLY;       /* x^3 -> x + 1 (mod P(x)) */
        }
    }
    return result & 0x07;              /* keep to 3 bits */
}

// Print an element as its 3-bit binary string, e.g. 5 -> "101"
static void print_binary3(uint8_t v) {
    for (int b = FIELD_BITS - 1; b >= 0; b--) {
        putchar((v & (1 << b)) ? '1' : '0');
    }
}

// Print an element in a2 x^2 + a1 x + a0 polynomial notation 
static void print_poly(uint8_t v) {
    int a2 = (v >> 2) & 1, a1 = (v >> 1) & 1, a0 = v & 1;
    int printed = 0;
    if (a2) { printf("x^2"); printed = 1; }
    if (a1) { printf("%s x", printed ? " +" : ""); printed = 1; }
    if (a0) { printf("%s 1", printed ? " +" : ""); printed = 1; }
    if (!printed) printf("0");
}

int main(void) {
    printf("GF(2^3) = F2[x] / (x^3 + x + 1)\n\n");

    printf("Element representation:\n");
    printf("Binary  Decimal  Polynomial\n");
    for (int v = 0; v < FIELD_SIZE; v++) {
        print_binary3((uint8_t)v);
        printf("     %-7d  ", v);
        print_poly((uint8_t)v);
        printf("\n");
    }

    printf("\nMultiplication table (decimal labels):\n\n");

    //header
    printf("x |");
    for (int col = 0; col < FIELD_SIZE; col++) printf(" %d", col);
    printf("\n");
    printf("--+");
    for (int col = 0; col < FIELD_SIZE; col++) printf("--");
    printf("\n");

    //table body 
    for (int row = 0; row < FIELD_SIZE; row++) {
        printf("%d |", row);
        for (int col = 0; col < FIELD_SIZE; col++) {
            uint8_t product = gf8_mult((uint8_t)row, (uint8_t)col);
            printf(" %d", product);
        }
        printf("\n");
    }

    return 0;
}