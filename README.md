<!-- This readme might be incomplete, please see the github repo for updated readme -->
# Assignment 2: Cryptology 6160
Sagar Parajuli  
CS26BTKMU11003

This repo contains a Software implementation of AES and AES-NI implemented through compiler instrincis, comparison benchmar between the two and a table implementation of GF(2).

## AES-NI Implementation
The aesni.c has the AES-NI implementaion: 

To compile:
First:
>> gcc -O2 -maes -msse4.1 -o aesni aesni.c

Encyrption and Decyrption:
>> ./aesni enc <32-hex-char key> <32-hex-char plaintext>

>> ./aesni dec <32-hex-char key> <32-hex-char ciphertext>

Example encyrption/decryption run:

Output:
>>./aesni enc 000102030405060708090a0b0c0d0e0f 00112233445566778899aabbccddeeff

=== AES-NI round keys ===
k_-1       : 000102030405060708090a0b0c0d0e0f
k_0        : 01050a0f04090e03080d02070c01060b
k_1        : 0309020b040d060f08010a030c050e07
k_2        : 070d0a0704010e0b0805020f0c090603
k_3        : 0f0102030405060708090a0b0c0d0e0f
k_4        : 1f050a0f04090e03080d02070c01060b
k_5        : 3f09020b040d060f08010a030c050e07
k_6        : 7f0d0a0704010e0b0805020f0c090603
k_7        : ff0102030405060708090a0b0c0d0e0f
k_8        : e4050a0f04090e03080d02070c01060b
k_9        : d209020b040d060f08010a030c050e07

=== AES-NI encryption ===
key        : 000102030405060708090a0b0c0d0e0f
plaintext  : 00112233445566778899aabbccddeeff
ciphertext : 1745e78859246d5739c934153a79aac8

>>./aesni dec 000102030405060708090a0
b0c0d0e0f 1745e78859246d5739c934153a79aac8 

=== AES-NI round keys ===
k_-1       : 000102030405060708090a0b0c0d0e0f
k_0        : 01050a0f04090e03080d02070c01060b
k_1        : 0309020b040d060f08010a030c050e07
k_2        : 070d0a0704010e0b0805020f0c090603
k_3        : 0f0102030405060708090a0b0c0d0e0f
k_4        : 1f050a0f04090e03080d02070c01060b
k_5        : 3f09020b040d060f08010a030c050e07
k_6        : 7f0d0a0704010e0b0805020f0c090603
k_7        : ff0102030405060708090a0b0c0d0e0f
k_8        : e4050a0f04090e03080d02070c01060b
k_9        : d209020b040d060f08010a030c050e07

=== AES-NI decryption ===
key        : 000102030405060708090a0b0c0d0e0f
ciphertext : 1745e78859246d5739c934153a79aac8
plaintext  : 00112233445566778899aabbccddeeff

## AES software implementation
The commands remain the same for AES software implementaion just the object file name and C file name can be changed. Identical outputs can be seen for both implemntation verifying validity of both AES implementation.

Compile:
>> gcc -O2 -maes -msse4.1 -o aes AESStandard.c

Output:
>>$ ./aes enc 000102030405060708090a0b0
c0d0e0f 00112233445566778899aabbccddeeff

=== Software implementation: round keys ===
k_-1       : 000102030405060708090a0b0c0d0e0f
k_0        : 01050a0f04090e03080d02070c01060b
k_1        : 0309020b040d060f08010a030c050e07
k_2        : 070d0a0704010e0b0805020f0c090603
k_3        : 0f0102030405060708090a0b0c0d0e0f
k_4        : 1f050a0f04090e03080d02070c01060b
k_5        : 3f09020b040d060f08010a030c050e07
k_6        : 7f0d0a0704010e0b0805020f0c090603
k_7        : ff0102030405060708090a0b0c0d0e0f
k_8        : e4050a0f04090e03080d02070c01060b
k_9        : d209020b040d060f08010a030c050e07

=== Software encryption ===
key        : 000102030405060708090a0b0c0d0e0f
plaintext  : 00112233445566778899aabbccddeeff
ciphertext : 1745e78859246d5739c934153a79aac8

>> ./aes dec 000102030405060708090a0b0
c0d0e0f 1745e78859246d5739c934153a79aac8 

=== Software implementation: round keys ===
k_-1       : 000102030405060708090a0b0c0d0e0f
k_0        : 01050a0f04090e03080d02070c01060b
k_1        : 0309020b040d060f08010a030c050e07
k_2        : 070d0a0704010e0b0805020f0c090603
k_3        : 0f0102030405060708090a0b0c0d0e0f
k_4        : 1f050a0f04090e03080d02070c01060b
k_5        : 3f09020b040d060f08010a030c050e07
k_6        : 7f0d0a0704010e0b0805020f0c090603
k_7        : ff0102030405060708090a0b0c0d0e0f
k_8        : e4050a0f04090e03080d02070c01060b
k_9        : d209020b040d060f08010a030c050e07

=== Software decryption ===
key        : 000102030405060708090a0b0c0d0e0f
ciphertext : 1745e78859246d5739c934153a79aac8
plaintext  : 00112233445566778899aabbccddeeff

## AES benchmark

Please find the report on AES comparison on the repo and the classroom.

## Galios field table of 8
Copy pasted output from terminal may not appear properly formatted.

>> gcc -o gf gf8.c  
>> ./gf

GF(2^3) = F2[x] / (x^3 + x + 1)

Element representation:
Binary  Decimal  Polynomial
000     0        0
001     1         1
010     2         x
011     3         x + 1
100     4        x^2
101     5        x^2 + 1
110     6        x^2 + x
111     7        x^2 + x + 1

Multiplication table (decimal labels):

x | 0 1 2 3 4 5 6 7
--+----------------
0 | 0 0 0 0 0 0 0 0
1 | 0 1 2 3 4 5 6 7
2 | 0 2 4 6 3 1 7 5
3 | 0 3 6 5 7 4 1 2
4 | 0 4 3 7 6 2 5 1
5 | 0 5 1 4 2 7 3 6
6 | 0 6 7 1 5 3 2 4
7 | 0 7 5 2 1 6 4 3
