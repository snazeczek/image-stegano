# Image-Stegano: Advanced Steganography in Images and Video

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++ version](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://isocpp.org/)
[![OpenCV](https://img.shields.io/badge/OpenCV-4.x-green.svg)](https://opencv.org/)
[![SFML](https://img.shields.io/badge/SFML-2.x-orange.svg)](https://www.sfml-dev.org/)

**Image-Stegano** is a powerful and easy-to-use tool for steganography—the art of hiding secret data within image and video files. With this application, you can invisibly encrypt and conceal your messages or files within innocent-looking pictures.

## What is Steganography?

Steganography is a communication technique aimed at hiding the very existence of a secret message. Unlike cryptography, which focuses on encrypting the content, steganography conceals the secret data within a carrier medium (in this case, an image file) so that it does not arouse any suspicion.

## Key Features

* **Hide Data:** Conceal any text or file inside image formats like PNG, BMP, or JPG.
* **Encryption:** All hidden data is additionally encrypted, providing a second layer of security.
* **Video Support (Coming Soon):** We are working on extending the functionality to hide data in video files.
* **Intuitive GUI:** Thanks to the use of SFML/ImGui, the application is simple and pleasant to use.
* **Performance:** Image operations are optimized for speed using the OpenCV library.
* **Portability:** The code, written in C++, is highly portable and can be compiled on various platforms.

## Screenshots

| Main Application View | Hiding & Revealing Process |
| :---: | :---: |
| ![image](https://github.com/user-attachments/assets/58e77e51-5297-43c4-81f6-41da09d8123e) | ![image](https://github.com/user-attachments/assets/d2c1964d-e421-461a-a238-586eefca3dda) |
| ![image](https://github.com/user-attachments/assets/69fb4df5-4e19-467e-9ef5-d96012df028f) | ![image](https://github.com/user-attachments/assets/9df5dbd9-a0a5-4e69-8dd9-b5290dd06d87) |


## Technologies

This project was built using the following technologies:

* **C++:** The primary programming language, ensuring high performance.
* **OpenCV:** A powerful library for real-time image processing.
* **SFML (Simple and Fast Multimedia Library):** Used to create a simple and portable graphical user interface.
* **ImGui (Immediate Mode GUI):** A library for rendering the graphical interface within an SFML window.

## Installation

*(Place instructions here on how to compile and run your project. Below is a sample template you can customize.)*

**Prerequisites:**
* A C++17 compiler (e.g., GCC, Clang, MSVC)
* CMake
* Installed OpenCV and SFML libraries

**Installation Steps:**

1.  Clone the repository:
    ```bash
    git clone [https://github.com/snazeczek/image-stegano.git](https://github.com/snazeczek/image-stegano.git)
    cd image-stegano
    ```
2.  Configure the project using CMake:
    ```bash
    cmake .
    ```
3.  Compile the project:
    ```bash
    make
    ```
4.  Run the application:
    ```bash
    ./image-stegano
    ```

## Usage

*(In this section, describe step-by-step how to use your application. You can add command-line examples or describe the steps in the graphical interface.)*

**To hide a message:**
1.  Launch the application.
2.  Select the "Encrypt Image" option.
3.  Choose the image in which you want to hide the message.
4.  Enter or load the message/file you want to hide.
5.  Provide a password to encrypt the data.
6.  Save the new image with the hidden content.

**To reveal a message:**
1.  Launch the application.
2.  Select the "Decrypt Image" option.
3.  Choose the image from which you want to read the message.
4.  Enter the password used during the hiding process.
5.  The application will display the hidden message.

## How to Contribute

We are open to all suggestions and contributions! If you want to help, please follow these steps:

1.  `Fork` the repository.
2.  Create a new branch (`git checkout -b feature/YourNewFeature`).
3.  Make your changes.
4.  `Commit` your changes (`git commit -am 'Add a new feature'`).
5.  `Push` to your branch (`git push origin feature/YourNewFeature`).
6.  Create a new `Pull Request`.

## License

This project is licensed under the MIT License. See the [LICENSE.md](LICENSE.md) file for details.

## Acknowledgments

* To the **OpenCV** and **SFML** communities for creating such great tools.
* To all future contributors for their time and effort.
