#include "../include/Crypto.hpp"
#include <stdexcept>
#include <vector>
#include <string>
#include <algorithm>
#include <iostream>

extern "C" {
#include <aes.h>
}

namespace Crypto {

void pad_pkcs7(std::vector<char>& data) {
    size_t block_size = AES_BLOCKLEN;
    size_t padding_len = block_size - (data.size() % block_size);
    char padding_val = static_cast<char>(padding_len);
    for (size_t i = 0; i < padding_len; ++i) {
        data.push_back(padding_val);
    }
}

bool unpad_pkcs7(std::vector<char>& data) {
    if (data.empty()) {
        return false;
    }
    size_t padding_len = static_cast<size_t>(data.back());
    if (padding_len == 0 || padding_len > data.size() || padding_len > AES_BLOCKLEN) {
        return false;
    }

    for (size_t i = 0; i < padding_len; ++i) {
        if (data[data.size() - 1 - i] != static_cast<char>(padding_len)) {
            return false;
        }
    }

    data.resize(data.size() - padding_len);
    return true;
}

void generateKeyFromPassword(const std::string& password, uint8_t* key, size_t key_len) {
    std::fill_n(key, key_len, 0);
    size_t len_to_copy = std::min(password.length(), key_len);
    std::copy(password.begin(), password.begin() + len_to_copy, key);
}


std::vector<char> encryptAES(const std::vector<char>& plaintext, const std::string& password) {
    std::cout << "[DEBUG] Crypto::encryptAES function was called." << std::endl;
    std::cout << "[DEBUG] Input data size (before encryption): " << plaintext.size() << " bytes." << std::endl;
    if (password.empty()) {
        throw std::runtime_error("Password cannot be empty for encryption.");
    }

    uint8_t key[AES_KEYLEN];
    uint8_t iv[]  = "init_vector_1234";

    generateKeyFromPassword(password, key, sizeof(key));

    std::vector<char> padded_data = plaintext;
    pad_pkcs7(padded_data);

    struct AES_ctx ctx;
    AES_init_ctx_iv(&ctx, key, iv);

    AES_CBC_encrypt_buffer(&ctx, reinterpret_cast<uint8_t*>(padded_data.data()), padded_data.size());
    std::cout << "[DEBUG] Encryption finished. Output data size (after encryption and padding): " << padded_data.size() << " bytes." << std::endl;
    return padded_data;
}

std::vector<char> decryptAES(const std::vector<char>& ciphertext, const std::string& password) {
    if (password.empty()) {
        return {};
    }
    if (ciphertext.empty() || ciphertext.size() % AES_BLOCKLEN != 0) {
       return {};
    }

    uint8_t key[AES_KEYLEN];
    uint8_t iv[]  = "init_vector_1234";

    generateKeyFromPassword(password, key, sizeof(key));

    std::vector<char> decrypted_data = ciphertext;

    struct AES_ctx ctx;
    AES_init_ctx_iv(&ctx, key, iv);

    AES_CBC_decrypt_buffer(&ctx, reinterpret_cast<uint8_t*>(decrypted_data.data()), decrypted_data.size());

    if (!unpad_pkcs7(decrypted_data)) {
        return {};
    }

    return decrypted_data;
}

} // namespace Crypto
