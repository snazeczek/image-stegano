#pragma once
#include <functional>
#include <string>
#include <vector>


namespace cv {
    class Mat;
}

namespace SteganoLSB {
    enum class DataType : uint8_t {
        Text = 0x01,
        File = 0x02
    };

    struct DecodedPayload {
        bool        isValid = false;
        bool        isEncrypted = false;
        DataType    type = DataType::Text;
        std::string fileExtension;
        std::vector<char> data;
    };

    bool encode(const cv::Mat& inputImage, const std::vector<char>& payload, DataType type, const std::string& fileExtension, cv::Mat& outputImage, const std::function<void(float)>& progressCallback, bool isEncrypted);
    DecodedPayload decode(const cv::Mat& image);

    size_t getCapacity(const cv::Mat& image);

} // namespace SteganoLSB