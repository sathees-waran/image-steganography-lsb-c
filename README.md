# Image Steganography using LSB (C)

## Project Status
In active development. The project structure and encode-side file 
handling are implemented and tested. Core encoding/decoding logic 
(LSB embedding and extraction) is in progress.

## Overview
Image steganography is the practice of hiding secret data (text, files) 
inside an image file in a way that does not noticeably alter how the 
image looks. This project implements steganography using the 
Least Significant Bit (LSB) technique on BMP images, written in C.

A secret file is embedded into the pixel data of a source BMP image, 
producing a "stego image" that looks identical to the original but 
contains hidden data. The same data can later be extracted from the 
stego image using a decoder.

## Understanding the BMP File Format
A BMP (Bitmap) file is an uncompressed raster image format. A standard 
24-bit BMP consists of:

- A 54-byte header (14-byte file header + 40-byte DIB/info header) 
  containing metadata such as file size, image width, and image height
- Raw pixel data immediately following the header, with each pixel 
  typically stored as 3 bytes (Blue, Green, Red)

The image width is stored at byte offset 18, and the height at offset 22, 
each as a 4-byte value. This project reads these values directly to 
determine the image's total capacity for hiding data.

## Why LSB + BMP
Because BMP stores pixel data uncompressed and byte-for-byte, modifying 
the least significant bit of each byte changes that color value by at 
most 1 (out of 255) — a change too small for the human eye to detect. 
Since the data isn't compressed or reorganized, hidden bits remain 
intact in the output file.

## Current Progress
- [x] Project structure and modular design (types, encode interface, 
      shared constants)
- [x] File handling: opening source image, secret file, and output 
      stego image with error handling
- [x] Reading BMP image dimensions to calculate image capacity
- [x] Command-line argument parsing and validation
- [x] Capacity check (ensuring secret data fits within the image)
- [x] BMP header copy to stego image
- [x] Core LSB encoding logic
- [x] Core LSB decoding logic
- [x] Decode-side implementation (decode.c, decode.h, test_decode.c)

## Project Structure
- `types.h` – common type definitions (Status, OperationType)
- `common.h` – shared constants (magic string used to identify a 
  stego image)
- `encode.h` – encoding interface and EncodeInfo structure
- `encode.c` – encoding implementation (in progress)
- `test_encode.c` – test driver for encoding functions

## Features

### Implemented
- Opening and validating source image, secret file, and output file
- Calculating BMP image capacity from header dimensions

### Planned
- Full LSB encoding pipeline (magic string, file extension, file size, 
  and data embedding)
- Full LSB decoding pipeline
- Capacity validation before encoding
- Command-line interface for encode/decode operations

## How It Works (LSB Technique)
Each byte of the secret data is broken into individual bits. Each bit 
is then stored in the least significant bit of one byte of the image's 
pixel data, replacing it with the secret bit. Since this only changes 
each affected byte by at most 1, the visual difference is imperceptible. 
To extract the data, the same bits are read back from the least 
significant bits of the stego image's pixel data, in the same order 
they were written.

## Tech Stack
- Language: C
- Image format: BMP (24-bit)
- Compiler: GCC

## Concepts Used

| Concept | Where Applied |
|---|---|
| **Structures (`struct`)** | Metadata/config structs defined in `types.h`, used across `encode.c` and `decode.c` |
| **File Handling** | `fopen`, `fread`, `fwrite`, `fclose` to read/write BMP and secret files in binary mode |
| **Pointers** | Struct pointers passed to encode/decode functions to avoid copying and to mutate state |
| **Bitwise Operators** | `&`, `\|`, `<<`, `>>` to manipulate individual bits for LSB encoding/decoding |
| **File Offsets (`fseek`/`rewind`)** | `fseek(fptr, 54, SEEK_SET)` to skip the BMP header; `rewind()` before copying header bytes |
| **String Functions** | `strcpy`, `strcmp`, `strlen` from `<string.h>` for filenames and extension handling |
| **Modular Programming** | Logic split across `encode.c`, `decode.c`, `common.h`, and `types.h` |
| **Header Guards** | `#ifndef` / `#define` / `#endif` in `encode.h`, `decode.h`, `types.h`, `common.h` |
| **`typedef`** | Used to alias `struct` types for encode/decode metadata |
| **Switch-Case** | Mode selection (encode `-e` vs decode `-d`) in `main` / `test_encode.c` |
| **Return Codes / Enums** | `Status` enum (`e_success`, `e_failure`) used as function return values for error handling |
| **Command-Line Arguments** | `argc`, `argv` used to pass BMP file, secret file, and mode flags |
| **Preprocessor Macros** | `#define MAGIC_STRING`, size constants, etc. |
| **Defined Encoding Order** | Magic string → extension size → extension string → file size → file data, embedded in fixed sequence during encoding |
| **Defined Decoding Order** | Magic string → extension size → extension string → file size → file data, read back in the same sequence to reconstruct the output file |

## Build Instructions
The current testable component verifies file handling and image size 
calculation:

```bash
encoding
gcc main.c encode.c  
./a.out -e input.bmp secret.txt setgo.bmp
decoding
gcc main.c decode.c
./a.out -d stego.bmp output

```

## Roadmap
- [ ] Implement argument parsing and validation
- [ ] Implement capacity check
- [ ] Implement BMP header copy
- [ ] Implement core LSB encode/decode functions
- [ ] Build decode-side files and test driver
- [ ] Add a simple usage interface (CLI arguments: -e/-d)

## Contact
Satheeswaran M
Email: satheeswaran.vnr@gmail.com
GitHub: sathees-waran
