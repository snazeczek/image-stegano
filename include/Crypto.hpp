#ifndef CRYPTO_HPP
#define CRYPTO_HPP

#include <vector>
#include <string>

namespace Crypto {
    /**
     * @brief Szyfruje dane za pomocą AES-256 w trybie GCM.
     * @param plaintext Dane do zaszyfrowania.
     * @param password Hasło używane do wygenerowania klucza.
     * @return Zaszyfrowane dane (ciphertext).
     * @throws std::runtime_error w przypadku błędu.
     */
    std::vector<char> encryptAES(const std::vector<char>& plaintext, const std::string& password);

    /**
     * @brief Deszyfruje dane za pomocą AES-256 w trybie GCM.
     * @param ciphertext Dane do odszyfrowania.
     * @param password Hasło używane do wygenerowania klucza.
     * @return Odszyfrowane dane (plaintext). Zwraca pusty wektor, jeśli deszyfrowanie się nie powiedzie (np. złe hasło).
     */
    std::vector<char> decryptAES(const std::vector<char>& ciphertext, const std::string& password);
}

#endif //CRYPTO_HPP