#include "../include/LSB.hpp"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>

namespace SteganoLSB {

    const uint32_t MAGIC_NUMBER = 0x43455242; // "CERB"

    const int MAGIC_NUMBER_BITS = 32;
    const int IS_ENCRYPTED_FLAG_BITS = 8;
    const int DATA_TYPE_BITS = 8;
    const int EXT_LEN_BITS = 8;
    const int PAYLOAD_SIZE_BITS = 64;

    size_t getCapacity(const cv::Mat& image) {
        if (image.empty()) return 0;

        size_t maxBits = (size_t)image.rows * image.cols * 3;
        int minHeaderBits = MAGIC_NUMBER_BITS + IS_ENCRYPTED_FLAG_BITS + DATA_TYPE_BITS + EXT_LEN_BITS + PAYLOAD_SIZE_BITS;

        if (maxBits < minHeaderBits) return 0;

        return (maxBits - minHeaderBits) / 8;
    }

    bool encode(
        const cv::Mat& inputImage,
        const std::vector<char>& payload,
        DataType type,
        const std::string& fileExtension,
        cv::Mat& outputImage,
        const std::function<void(float)>& progressCallback,
        bool isEncrypted)
    {
        uint8_t extensionLength = fileExtension.length();
        uint64_t payloadSize = payload.size();
        size_t headerBits = MAGIC_NUMBER_BITS + IS_ENCRYPTED_FLAG_BITS + DATA_TYPE_BITS + EXT_LEN_BITS + (extensionLength * 8) + PAYLOAD_SIZE_BITS;
        size_t totalBitsToEncode = headerBits + (payloadSize * 8);

        if (((size_t)inputImage.rows * inputImage.cols * 3) < totalBitsToEncode) {
             std::cerr << "Error: Image is too small!" << std::endl;
            return false;
        }

        outputImage = inputImage.clone();
        int bitIndex = 0;

        const int ENCRYPTED_FLAG_START = MAGIC_NUMBER_BITS;
        const int DATA_TYPE_START = ENCRYPTED_FLAG_START + IS_ENCRYPTED_FLAG_BITS;
        const int EXT_LEN_START = DATA_TYPE_START + DATA_TYPE_BITS;
        const int EXT_START = EXT_LEN_START + EXT_LEN_BITS;
        const int PAYLOAD_SIZE_START = EXT_START + (extensionLength * 8);
        const int PAYLOAD_START = PAYLOAD_SIZE_START + PAYLOAD_SIZE_BITS;

        for (int y = 0; y < outputImage.rows; ++y) {
            for (int x = 0; x < outputImage.cols; ++x) {
                cv::Vec3b& pixel = outputImage.at<cv::Vec3b>(y, x);
                for (int color = 0; color < 3; ++color) {
                    if (bitIndex >= totalBitsToEncode) goto finished_encoding;

                    int bitToHide = 0;
                    if (bitIndex < ENCRYPTED_FLAG_START) {
                        bitToHide = (MAGIC_NUMBER >> bitIndex) & 1;
                    }
                    else if (bitIndex < DATA_TYPE_START) {
                        char encryptedFlag = isEncrypted ? 0x01 : 0x00;
                        bitToHide = (encryptedFlag >> (bitIndex - ENCRYPTED_FLAG_START)) & 1;
                    }
                    else if (bitIndex < EXT_LEN_START) {
                        bitToHide = (static_cast<uint8_t>(type) >> (bitIndex - DATA_TYPE_START)) & 1;
                    } else if (bitIndex < EXT_START) {
                        bitToHide = (extensionLength >> (bitIndex - EXT_LEN_START)) & 1;
                    } else if (bitIndex < PAYLOAD_SIZE_START) {
                        int localBitIndex = bitIndex - EXT_START;
                        bitToHide = (fileExtension[localBitIndex / 8] >> (localBitIndex % 8)) & 1;
                    } else if (bitIndex < PAYLOAD_START) {
                        bitToHide = (payloadSize >> (bitIndex - PAYLOAD_SIZE_START)) & 1;
                    } else {
                        int dataBitIndex = bitIndex - PAYLOAD_START;
                        bitToHide = (payload[dataBitIndex / 8] >> (dataBitIndex % 8)) & 1;
                    }

                    pixel[color] = (pixel[color] & 0xFE) | bitToHide;
                    bitIndex++;
                }
            }
            if (progressCallback) {
                float progress = static_cast<float>(y + 1) / outputImage.rows;
                progressCallback(progress);
            }
        }
    finished_encoding:
        if (progressCallback) {
            progressCallback(1.0f);
        }
        return true;
    }

    DecodedPayload decode(const cv::Mat& image) {
        DecodedPayload result;
        if (image.empty()) {
            std::cerr << "Decoding error: Image is empty." << std::endl;
            return result;
        }

        uint32_t magicNumber = 0;
        uint8_t encryptedFlag = 0;
        uint8_t dataType = 0;
        uint8_t extensionLength = 0;
        uint64_t payloadSize = 0;
        int bitIndex = 0;

        auto read_bits = [&](int count) -> uint64_t {
            uint64_t value = 0;
            for (int i = 0; i < count; ++i) {
                int row = bitIndex / (image.cols * 3);
                int col = (bitIndex / 3) % image.cols;
                int channel = bitIndex % 3;

                if (row >= image.rows) {
                    result.isValid = false;
                    std::cerr << "Decoding error: Attempt to read beyond image boundaries." << std::endl;
                    return -1;
                }

                const cv::Vec3b& pixel = image.at<cv::Vec3b>(row, col);
                value |= ((uint64_t)(pixel[channel] & 1) << i);
                bitIndex++;
            }
            return value;
        };

        magicNumber = read_bits(MAGIC_NUMBER_BITS);
        if (magicNumber != MAGIC_NUMBER) {
            std::cerr << "Decoding error: Magic number not found. This is not our file." << std::endl;
            return result;
        }

        encryptedFlag = read_bits(IS_ENCRYPTED_FLAG_BITS);
        result.isEncrypted = (encryptedFlag == 0x01);

        dataType = read_bits(DATA_TYPE_BITS);
        result.type = static_cast<DataType>(dataType);

        extensionLength = read_bits(EXT_LEN_BITS);

        if (extensionLength > 20) {
            std::cerr << "Decoding error: Invalid file extension length." << std::endl;
            return result;
        }
        result.fileExtension.resize(extensionLength);

        for (int i = 0; i < extensionLength; ++i) {
            result.fileExtension[i] = static_cast<char>(read_bits(8));
        }

        payloadSize = read_bits(PAYLOAD_SIZE_BITS);

        size_t remainingBits = ((size_t)image.rows * image.cols * 3) - bitIndex;
        if ((payloadSize * 8) > remainingBits) {
            std::cerr << "Decoding error: Declared data size exceeds image capacity." << std::endl;
            return result;
        }
        result.data.resize(payloadSize);

        for (uint64_t i = 0; i < payloadSize; ++i) {
            result.data[i] = static_cast<char>(read_bits(8));
        }

        result.isValid = true;
        std::cout << "Decoding successful." << std::endl;
        return result;
    }

} // namespace SteganoLSB