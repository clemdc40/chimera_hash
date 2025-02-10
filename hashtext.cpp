#include <iostream>
#include <string>
#include <array>
#include <set>
#include <cassert>
#include <iomanip>
#include <sstream>
#include <cstring>
#include <random>
#include <chrono>

#include "chimera_hash.cpp"

void testBasicHashing(bool& success) {
    std::cout << "Test de base...\n";
    std::string input1 = "test";
    std::string input2 = "hello";
    std::string input3 = "world";

    auto hash1 = toHexString(chimeraHash(reinterpret_cast<const uint8_t*>(input1.data()), input1.size()));
    auto hash2 = toHexString(chimeraHash(reinterpret_cast<const uint8_t*>(input2.data()), input2.size()));
    auto hash3 = toHexString(chimeraHash(reinterpret_cast<const uint8_t*>(input3.data()), input3.size()));

    std::cout << "test: " << hash1 << "\n";
    std::cout << "hello: " << hash2 << "\n";
    std::cout << "world: " << hash3 << "\n";

    assert(hash1 != hash2 && hash1 != hash3 && hash2 != hash3);
    std::cout << "Test de base reussi.\n\n";
    success = true;
}

void testCollisions(bool& success) {
    std::cout << "Test des collisions...\n";
    std::string input1 = "abc";
    std::string input2 = "abcd";

    auto hash1 = toHexString(chimeraHash(reinterpret_cast<const uint8_t*>(input1.data()), input1.size()));
    auto hash2 = toHexString(chimeraHash(reinterpret_cast<const uint8_t*>(input2.data()), input2.size()));

    std::cout << "abc:  " << hash1 << "\n";
    std::cout << "abcd: " << hash2 << "\n";

    assert(hash1 != hash2);
    std::cout << "Test des collisions reussi.\n\n";
    success = true;
}

void testUniqueness(size_t numTests, bool& success) {
    std::cout << "Test d'unicite sur " << numTests << " entrees aleatoires...\n";
    std::set<std::string> uniqueHashes;
    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<char> charDist(32, 126);

    for (size_t i = 0; i < numTests; ++i) {
        std::string randomInput;
        size_t length = 10 + rng() % 20;
        for (size_t j = 0; j < length; ++j) {
            randomInput += charDist(rng);
        }

        auto hash = toHexString(chimeraHash(reinterpret_cast<const uint8_t*>(randomInput.data()), randomInput.size()));
        assert(uniqueHashes.find(hash) == uniqueHashes.end());
        uniqueHashes.insert(hash);
    }
    std::cout << "Aucun hash duplique detecte parmi " << numTests << " entrees aleatoires.\n\n";
    success = true;
}

void testAvalancheEffect(bool& success) {
    std::cout << "Test de diffusion (effet avalanche)...\n";
    std::string input = "The quick brown fox jumps over the lazy dog.";
    auto hash1 = chimeraHash(reinterpret_cast<const uint8_t*>(input.data()), input.size());

    input[0] = 't';
    auto hash2 = chimeraHash(reinterpret_cast<const uint8_t*>(input.data()), input.size());

    std::string hex1 = toHexString(hash1);
    std::string hex2 = toHexString(hash2);

    std::cout << "Hash original : " << hex1 << "\n";
    std::cout << "Hash modifie  : " << hex2 << "\n";

    int bitDifference = 0;
    for (size_t i = 0; i < CHIMERA_HASH_SIZE; ++i) {
        bitDifference += __builtin_popcount(hash1[i] ^ hash2[i]);
    }

    std::cout << "Difference de bits : " << bitDifference << " bits\n";
    assert(bitDifference > CHIMERA_HASH_SIZE * 4);
    std::cout << "Test de diffusion reussi.\n\n";
    success = true;
}

void testHashingTime(const std::string& input) {
    std::cout << "Test du temps de hachage...\n";
    auto start = std::chrono::high_resolution_clock::now();
    auto hash = chimeraHash(reinterpret_cast<const uint8_t*>(input.data()), input.size());
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;

    std::cout << "Temps de hachage pour \"" << input << "\": " << duration.count() << " secondes\n";
}

int main() {
    std::cout << "=== Tests de fiabilite de ChimeraHash512 ===\n\n";

    bool basicHashingSuccess = false;
    bool collisionsSuccess = false;
    bool uniquenessSuccess = false;
    bool avalancheEffectSuccess = false;

    testBasicHashing(basicHashingSuccess);
    testCollisions(collisionsSuccess);
    testUniqueness(100000, uniquenessSuccess);
    testAvalancheEffect(avalancheEffectSuccess);

    std::cout << "=== Resume des resultats ===\n";
    std::cout << "Test de base : " << (basicHashingSuccess ? "Reussi" : "Echoue") << "\n";
    std::cout << "Test des collisions : " << (collisionsSuccess ? "Reussi" : "Echoue") << "\n";
    std::cout << "Test d'unicite : " << (uniquenessSuccess ? "Reussi" : "Echoue") << "\n";
    std::cout << "Test de diffusion : " << (avalancheEffectSuccess ? "Reussi" : "Echoue") << "\n";

    std::cout << "Tous les tests ont ete passes avec succes !\n";

    testHashingTime("example");

    return 0;
}
