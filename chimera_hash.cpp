#include <iostream>
#include <vector>
#include <array>
#include <iomanip>
#include <string>
#include <cstring>
#include <cstdint>
#include <sstream>

// ==================== CONSTANTES ====================
static const size_t CHIMERA_HASH_SIZE_BITS = 512;
static const size_t CHIMERA_HASH_SIZE      = CHIMERA_HASH_SIZE_BITS / 8;
static const size_t BLOCK_SIZE_BITS        = 1024;
static const size_t BLOCK_SIZE            = BLOCK_SIZE_BITS / 8;
static const unsigned int NUM_ROUNDS       = 10;

static const uint8_t SBOX[256] = {
    0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
    0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
    0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
    0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
    0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
    0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
    0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
    0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
    0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
    0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
    0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
    0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
    0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
    0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
    0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
    0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16
};

// ==================== FONCTIONS INTERNES ====================

// Substitution de tous les octets par la S-Box AES
static void applySBox(std::array<uint8_t, CHIMERA_HASH_SIZE> &state)
{
    for (auto &byte : state) {
        byte = SBOX[byte];
    }
}

// Matrice dite "chaotique", style MixColumns
static const std::array<std::array<uint8_t, 8>, 8> CHAOTIC_MATRIX = {{
    {0x02, 0x03, 0x01, 0x01, 0x04, 0x07, 0x05, 0x02},
    {0x03, 0x05, 0x07, 0x01, 0x02, 0x06, 0x04, 0x03},
    {0x01, 0x07, 0x05, 0x04, 0x03, 0x02, 0x06, 0x01},
    {0x01, 0x04, 0x03, 0x07, 0x06, 0x05, 0x02, 0x03},
    {0x04, 0x02, 0x06, 0x06, 0x05, 0x01, 0x07, 0x04},
    {0x07, 0x06, 0x02, 0x05, 0x01, 0x03, 0x04, 0x07},
    {0x05, 0x04, 0x01, 0x02, 0x07, 0x04, 0x03, 0x05},
    {0x02, 0x03, 0x01, 0x03, 0x04, 0x07, 0x05, 0x06}
}};

// Mélange type MixColumns, bloc de 8 octets
static void chaoticMix(std::array<uint8_t, CHIMERA_HASH_SIZE> &state)
{
    for (size_t blockIndex = 0; blockIndex < 8; ++blockIndex) {
        std::array<uint8_t, 8> row;
        for (size_t i = 0; i < 8; ++i) {
            row[i] = state[blockIndex * 8 + i];
        }

        std::array<uint8_t, 8> newRow = {0};
        for (size_t i = 0; i < 8; ++i) {
            uint8_t sum = 0;
            for (size_t j = 0; j < 8; ++j) {
                uint8_t a = row[j];
                uint8_t b = CHAOTIC_MATRIX[i][j];
                
                uint8_t temp = 0;
                while (b) {
                    if (b & 1) {
                        temp ^= a;
                    }
                    // multiplication dans GF(2^8) avec polynôme 0x1b
                    a = (a << 1) ^ ((a & 0x80) ? 0x1b : 0x00);
                    b >>= 1;
                }
                sum ^= temp;
            }
            newRow[i] = sum;
        }

        for (size_t i = 0; i < 8; ++i) {
            state[blockIndex * 8 + i] = newRow[i];
        }
    }
}

// Décalage circulaire de l'état de "shiftBytes"
static void rotateState(std::array<uint8_t, CHIMERA_HASH_SIZE> &state, size_t shiftBytes)
{
    std::array<uint8_t, CHIMERA_HASH_SIZE> temp;
    for (size_t i = 0; i < CHIMERA_HASH_SIZE; ++i) {
        temp[i] = state[(i + shiftBytes) % CHIMERA_HASH_SIZE];
    }
    state = temp;
}

// Round de type Feistel sur 512 bits (256 + 256)
static void feistelRound(std::array<uint8_t, CHIMERA_HASH_SIZE> &state, unsigned round)
{
    constexpr size_t halfSize = CHIMERA_HASH_SIZE / 2;
    std::array<uint8_t, halfSize> left;
    std::array<uint8_t, halfSize> right;

    // Sépare en deux moitiés
    memcpy(left.data(),  state.data(),            halfSize);
    memcpy(right.data(), state.data() + halfSize, halfSize);

    // F-function = SBOX(right[i]) ^ round
    for (size_t i = 0; i < halfSize; ++i) {
        uint8_t s = SBOX[right[i]];
        uint8_t r = (s ^ static_cast<uint8_t>(round)) & 0xFF;
        left[i] = static_cast<uint8_t>(left[i] ^ r);
    }

    // Échange
    memcpy(state.data(),            right.data(), halfSize);
    memcpy(state.data() + halfSize, left.data(),  halfSize);
}

// État initial
static std::array<uint8_t, CHIMERA_HASH_SIZE> initState()
{
    std::array<uint8_t, CHIMERA_HASH_SIZE> state = {
        0xc1, 0x93, 0x5f, 0x28, 0x9a, 0xd4, 0x76, 0x20,
        0x72, 0xf3, 0x5b, 0xee, 0x12, 0x93, 0xab, 0xff,
        0x1e, 0x67, 0x2a, 0x49, 0xf0, 0xc8, 0xd6, 0x5c,
        0x34, 0xbe, 0x0a, 0x93, 0x91, 0xaa, 0x3b, 0x6f,
        0x8d, 0x56, 0x09, 0x2a, 0xee, 0x13, 0x7c, 0x67,
        0xbc, 0x98, 0x0f, 0x11, 0x80, 0xcd, 0x35, 0x90,
        0xf1, 0xeb, 0x47, 0x2e, 0x73, 0xaf, 0x90, 0x5f,
        0x27, 0x3c, 0x19, 0x8c, 0x4a, 0x6f, 0xc3, 0x21
    };
    return state;
}

// Compression d'un bloc de 128 octets dans l'état 512 bits
static void chimeraCompress(std::array<uint8_t, CHIMERA_HASH_SIZE> &state, 
                            const uint8_t *block)
{
    // Combine la première et la seconde moitié du bloc via XOR
    // bloc = 128 octets => i + CHIMERA_HASH_SIZE = i + 64
    for (size_t i = 0; i < CHIMERA_HASH_SIZE; ++i) {
        state[i] ^= block[i] ^ block[i + CHIMERA_HASH_SIZE];
    }

    // Effectue 10 tours (NUM_ROUNDS)
    for (unsigned round = 0; round < NUM_ROUNDS; ++round) {
        applySBox(state);
        chaoticMix(state);
        rotateState(state, (round + 1) % 16);
        feistelRound(state, round);
    }
}

// ==================== FONCTION DE HACHAGE AVEC PADDING CORRIGÉ ====================
std::array<uint8_t, CHIMERA_HASH_SIZE> chimeraHash(const uint8_t *data, size_t len)
{
    std::array<uint8_t, CHIMERA_HASH_SIZE> state = initState();

    // 1) Traiter les blocs complets
    size_t fullBlocks = len / BLOCK_SIZE;
    for (size_t i = 0; i < fullBlocks; ++i) {
        chimeraCompress(state, data + i * BLOCK_SIZE);
    }

    // 2) Gérer le bloc partiel
    size_t remaining = len % BLOCK_SIZE;
    uint8_t lastBlock[BLOCK_SIZE];
    memset(lastBlock, 0, BLOCK_SIZE);
    memcpy(lastBlock, data + fullBlocks * BLOCK_SIZE, remaining);

    // On place l'octet 0x80
    lastBlock[remaining] = 0x80;

    // Longueur du message (en bits)
    uint64_t bitLen = static_cast<uint64_t>(len) * 8ULL;

    // 3) Vérifier si on a la place pour stocker le bitLen dans les 8 derniers octets
    if (remaining + 1 + 8 <= BLOCK_SIZE) {
        // On peut tout mettre dans ce bloc
        for (size_t i = 0; i < 8; ++i) {
            lastBlock[BLOCK_SIZE - 1 - i] = static_cast<uint8_t>(bitLen >> (8 * i));
        }
        // Compresser ce bloc final
        chimeraCompress(state, lastBlock);
    } else {
        // On n'a pas la place, on compresse ce bloc partiel "sans la taille"
        chimeraCompress(state, lastBlock);

        // Puis on crée un second bloc plein de zéros
        uint8_t secondBlock[BLOCK_SIZE];
        memset(secondBlock, 0, BLOCK_SIZE);

        // On y place la taille en bits dans les 8 derniers octets
        for (size_t i = 0; i < 8; ++i) {
            secondBlock[BLOCK_SIZE - 1 - i] = static_cast<uint8_t>(bitLen >> (8 * i));
        }
        // Compresser ce second bloc
        chimeraCompress(state, secondBlock);
    }

    return state;
}

// ==================== CONVERSION EN HEXA ====================
static std::string toHexString(const std::array<uint8_t, CHIMERA_HASH_SIZE> &hashVal)
{
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (auto c : hashVal) {
        oss << std::setw(2) << static_cast<int>(c);
    }
    return oss.str();
}

/*
// Exemple d'utilisation
int main()
{
    std::string input = "test";
    auto result = chimeraHash(reinterpret_cast<const uint8_t*>(input.data()), input.size());
    std::cout << "Chaine d'entree : " << input << std::endl;
    std::cout << "Hash (ChimeraHash512) : " << toHexString(result) << std::endl;
    return 0;
}
*/
