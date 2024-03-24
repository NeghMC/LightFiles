<!-- ABOUT THE PROJECT -->
## About The Project

This repository contains a simple file-system. It is intended for embedded systems containing MCUs with limited resources.

Functional requirements:
* Light - insignificant size comparing to the entire program size
* Understandable - basis on logical and easy to understand concept
* Simple - no need for complex configuration

### Hardware requirements

A system should contain at least one accessible flash memory, where each byte of the memory is programmable, and the memory is divided in erasable blocks. It can be both external IC connecter to the MCU with compatible communication interface, or an internal flash memory.

### Key concept

Library allows to use flash memory as a dictionary, where the key is an unsigned integer in range of 0 to 65534 (0xffff - 1), and the value is a binary content. Content size is limited by the block size minus block header size. Each key can be created only once. If more data needs to be written it has to be removed and written again.

Developer needs to provide functions: 
```
lf_result_t lf_app_init(uint16_t *blockCount, uint16_t *blockSize);
lf_result_t lf_app_write(uint16_t block, uint16_t offset, void *buffer, uint16_t length, uint8_t flush);
lf_result_t lf_app_read(uint16_t block, uint16_t offset, void *buffer, uint16_t length);
lf_result_t lf_app_delete(uint16_t block);
```

For more details please search through the source files.

### Limitations

* Maximum file size is equal to block size minus header size (4 bytes).
* Each entry can be written only once - data cannot be appended.
* When deleting an entry the block is instantly formatted, which can take some time.

<!-- GETTING STARTED -->
## Getting Started

TBD

<!-- USAGE EXAMPLES -->
## Usage

Example implentation can be seen in `tests/integr_test_CC26X2R1_LAUNCHXL_tirtos7_ticlang`.

<!-- ROADMAP -->
## Roadmap

- [x] Create first public version
- [ ] Add possibility to append content
- [ ] Add garbage collection mechanism
- [ ] Add proper unit tests
- [ ] Add advanced memory test (either hardware or software)

<!-- LICENSE -->
## License

Distributed under the MIT License. See `LICENSE.txt` for more information.