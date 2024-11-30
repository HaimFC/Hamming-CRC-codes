# Error Detection and Correction Project

This repository focuses on the implementation of error-detecting and correcting codes, including Hamming (7,4) and CRC-4. These codes are integrated into a multi-layer communication system using UART for data transmission.

## Files in the Repository

- **`Hamming_Lab3.ino`**: Arduino implementation of the Hamming (7,4) code for error detection and correction.
- **`CRC_LAB3.ino`**: Arduino implementation of CRC-4 for error detection.
- **`Lab File.pdf`**: Contains detailed instructions for implementing Hamming and CRC codes, including layer integration and test scenarios.

## Project Overview

### Multi-Layer Communication
The project introduces a layered approach:
1. **Layer 1**: Handles UART-based communication (transmission and reception).
2. **Layer 2**: Manages error-detection and correction using Hamming (7,4) or CRC-4. It ensures seamless data exchange with Layer 1 via flags.

### Key Features
- **Hamming (7,4)**: Splits 8-bit characters into 4-bit nibbles and adds parity bits to create a 7-bit encoded frame. Can detect and correct single-bit errors.
- **CRC-4**: Encodes 8-bit characters with a 4-bit cyclic redundancy check (CRC) for error detection using the polynomial \(x^4 + x + 1\).
- **Inter-Layer Communication**: Flags are used to synchronize data flow between layers without busy waiting.

## Implementation Details

### Hamming (7,4)
1. **Hamming47_tx()**:
   - Splits an 8-bit character into two 4-bit nibbles.
   - Computes parity bits and encodes a 7-bit frame for each nibble.
   - Prints the encoded frame in binary.
   
2. **Hamming47_rx()**:
   - Decodes 7-bit frames, detects errors using the syndrome method, and corrects single-bit errors.
   - Reassembles the original ASCII character and prints it.

#### Tests:
- **No Errors**: Verify correct transmission and reassembly of characters.
- **Single-Bit Error**: Introduce a single-bit error, detect and correct it, and reassemble the character.
- **Two/Three-Bit Errors**: Evaluate the detection and correction limits of the code.

### CRC-4
1. **CRC4_tx()**:
   - Reads an 8-bit character.
   - Computes a 12-bit frame (data + CRC) using \(x^4 + x + 1\).
   - Prints the encoded frame in binary.

2. **CRC4_rx()**:
   - Validates received frames using the same polynomial.
   - Detects errors and prints the received character and CRC status.

#### Tests:
- **No Errors**: Verify correct transmission of characters.
- **Single-Bit Error**: Introduce a single-bit error and evaluate error detection.

### Layer Communication
- **Flags**:
  - `layer_1_tx_busy`: Raised during Layer 1 transmission.
  - `layer_2_tx_request`: Raised by Layer 2 to request Layer 1 transmission.
  - `layer_1_rx_busy`: Raised during Layer 1 reception.

## Tools and References

- **Hamming Code Simulator**: [Hamming Code Simulator](http://www.ecs.umass.edu/ece/koren/FaultTolerantSystems/simulator/Hamming/HammingCodes.html)
- **Hamming (7,4) Wikipedia**: [Hamming (7,4)](https://en.wikipedia.org/wiki/Hamming(7,4))
- **CRC Wikipedia**: [Cyclic Redundancy Check](https://en.wikipedia.org/wiki/Cyclic_redundancy_check)
- **CRC Online Calculator**: [CRC Calculator](https://asecuritysite.com/comms/crc_div)

## Getting Started

1. **Hardware Setup**: Use two Arduino boards connected via UART.
2. **Upload Code**: Load `Hamming_Lab3.ino` or `CRC_LAB3.ino` to test Hamming (7,4) or CRC-4, respectively.
3. **Run Tests**: Verify functionality with the provided test cases.

## Contribution

Contributions are welcome. Please adhere to the repository's structure and document any changes thoroughly.
