<!-- ABOUT THE PROJECT -->
## About The Project

This repository contains a simple file-system written in C. It is intended for embedded systems containing MCUs with limited resources. The file-system structure does not contain folders, all the files can be found globally with their "key" (an unsigned integer 0 to 126).  

Functional requirements:
* Light - insignificant size comparing to the entire program size
* Understandable - bases on logical and easy to understand concept
* Simple - no need for complex configuration
* Embedded - only the MCU has direct access to the memory

### Hardware requirements

A system should contain at least one accessible flash memory, where each byte of the memory is programmable, and the memory is divided in erasable blocks. It can be both external IC connecter to the MCU with compatible communication interface, or an internal flash memory. MCU is the only device that has access to the memory.

### Key concept

Library treats flash memory as a list of blocks. When a file is created the first free block is chosen from the list. That block becomes a leading block, which means that it is recognized when the application looks for a specific file. Then the data is written to that block. If the data is bigger than the block, a new free block is chosen, and information about that block is being saved to the current block. That second block is a following block and it wont be recognized as an separate file. The process continues until the file is saved, in that case the last piece of information is saved to the current block and file can not be written anymore. When reading the file it needs to be opened first. This way an information about the first block is cached. Then data is read in the similar matter as it is written following the chain of the blocks. After data is read the file is closed.

Each block contains a header that consists of file key, next block number, and data size in the current block (5 bytes in total). If the block is an leading block it has a MSB set in its key.

Developer needs to implement functions: 
```
lf_result_t lf_app_init(uint16_t *blockCount, uint16_t *blockSize);
lf_result_t lf_app_write(uint16_t block, uint16_t offset, void *buffer, uint16_t length, uint8_t flush);
lf_result_t lf_app_read(uint16_t block, uint16_t offset, void *buffer, uint16_t length);
lf_result_t lf_app_delete(uint16_t block);
```

For more details please search through the source files.

### Limitations

* Maximum amount of data that can be written at once cannot be bigger that block size. In that case it needs to be divided in smaller transfers.
* No build-in buffering - data is immediately written to the memory - more smaller data transfers.
* When deleting an entry the block is instantly formatted, which can take some time.

<!-- GETTING STARTED -->
## Getting Started

TBD

<!-- USAGE EXAMPLES -->
## Usage

Example implementation can be seen in `tests/integr_test_CC26X2R1_LAUNCHXL_tirtos7_ticlang`.

<!-- ROADMAP -->
## Roadmap

- [x] Create first public version
- [x] Add possibility to append content while writing to a file
- [ ] Add garbage collection mechanism
- [ ] Add proper unit tests
- [x] Add advanced memory test (either hardware or software)
    - [ ] Update to test the entire memory
- [ ] Optimize block searching function
- [x] Remove limit of file size
- [x] Add checking if file exists

<!-- LICENSE -->
## License

Distributed under the MIT License. See `LICENSE.txt` for more information.