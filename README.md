# Bluetooth Mesh Vendor Model Demo

This sample demonstrates how to implement a custom vendor model in Bluetooth Mesh technology. It showcases how to create, implement, and use vendor-specific models for custom data exchange between mesh nodes.

## Overview

The application implements both a vendor server model and a vendor client model within the same firmware. This allows a single device to both send and receive custom vendor model messages.

### Vendor Model Architecture

The vendor model is defined with:
- Company ID: 0x0059 (Nordic Semiconductor ASA)
- Model IDs:
  - Vendor Server Model: 0x1000
  - Vendor Client Model: 0x1001

### Message Types

The sample implements four message types:

1. **Vendor_SET (Opcode: 0x10 + Company ID)**
   - Sent from client to server
   - STATUS response is expected to be sent by the application
   - Acknowledged message type

   | Field Name | Size (octets) | Description                                 |
   |------------|--------------|----------------------------------------------|
   | Opcode     | 3            | 0x10 + Company ID (Little Endian)            |
   | Data       | 0–377        | Arbitrary data payload                       |

2. **Vendor_Set_Unack (Opcode: 0x11 + Company ID)**
   - Sent from client to server
   - No STATUS response is sent by the application
   - Unacknowledged message type (send and forget)

   | Field Name | Size (octets) | Description                                 |
   |------------|--------------|----------------------------------------------|
   | Opcode     | 3            | 0x11 + Company ID (Little Endian)            |
   | Data       | 0–377        | Arbitrary data payload                       |

3. **Vendor_GET (Opcode: 0x12 + Company ID)**
   - Sent from client to server
   - Supports an optional `length` parameter
   - If the `length` parameter is provided, the server will limit the STATUS response payload to the specified number of bytes
   - If not provided, the server sends the full response
   - Requires acknowledgment with a Vendor_STATUS response

   | Field Name | Size (octets) | Description                                 |
   |------------|--------------|----------------------------------------------|
   | Opcode     | 3            | 0x12 + Company ID (Little Endian)            |
   | Length     | 2 (optional) | Optional. Number of bytes requested in reply.|

4. **Vendor_STATUS (Opcode: 0x13 + Company ID)**
   - Sent from server to client
   - Sent in response to GET or SET messages
   - Automatically sent by the server when the handler returns success (0)

   | Field Name | Size (octets) | Description                                 |
   |------------|--------------|----------------------------------------------|
   | Opcode     | 3            | 0x13 + Company ID (Little Endian)            |
   | Data       | 0–377        | Response data payload                        |

## Requirements

### Hardware

* Two nRF52 or nRF53 series development boards (e.g., nRF52840 DK, nRF5340 DK)
* Smartphone with nRF Mesh mobile app (for provisioning)

### Software

* nRF Connect SDK (NCS) v3.0.0 or later
* nRF Mesh mobile app for [Android](https://play.google.com/store/apps/details?id=no.nordicsemi.android.nrfmeshprovisioner) or [iOS](https://apps.apple.com/us/app/nrf-mesh/id1380726771)

## Building and Running

1. Clone the nRF Connect SDK and set up your development environment
2. Build the application for your target board (e.g., nRF52840 DK):

   ```
   west build -b nrf52840dk/nrf52840
   ```

3. Flash the application to two development boards:

   ```
   west flash --erase
   ```

## Testing

1. Build and flash the firmware to two development kits.
2. Provision both devices using the nRF Mesh mobile app:
   * Scan for unprovisioned devices and provision them one by one
   * Add them to the same network
   * Configure publish/subscribe addresses to establish communication between the devices
3. Press Button 1 on one of the devices to send a "Hello World" message using Vendor_Set message (acknowledged)
4. Press Button 2 on one of the devices to send a "Hello World" message using Vendor_Set_Unack message (unacknowledged)
5. Press Button 3 on one of the devices to send a Vendor_Get request message (no parameters, full response)
6. Press Button 4 on one of the devices to send a Vendor_Get request message with the optional `length` parameter set to 1 (response will be truncated to 1 byte)
7. Observe the message exchange in the console logs

### Expected Output

#### For Acknowledged SET Message (Button 1)

On the device that receives the SET message, you should see:

```
[00:00:05.774,597] <inf> model_handler: Sending SET message: "Hello World- 0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF"
[00:00:05.774,658] <dbg> vnd_cli: bt_mesh_vendor_cli_set: Sending SET message, data length 301
[00:00:09.211,883] <dbg> vnd_cli: handle_status: Received STATUS message, data length 301
[00:00:09.212,066] <inf> model_handler: Received STATUS response: "Response OK- 0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF"
```

## Implementation Details

The sample consists of the following components:
1. **Vendor Model Definitions**
   * `include/vnd_common.h` - Common definitions shared by client and server
   * `include/vnd_srv.h` - Vendor server model API and context definitions
   * `include/vnd_cli.h` - Vendor client model API and context definitions

2. **Vendor Model Implementations**
   * `src/vnd_srv.c` - Vendor server model implementation
   * `src/vnd_cli.c` - Vendor client model implementation

3. **Application Logic**
   * `src/model_handler.c` - Model instance initialization and message handling
   * `src/main.c` - Main application, mesh initialization, and button handling

### Asynchronous Response Support

The vendor server model supports asynchronous responses:

1. The server model's callback handlers (`set` and `get`) return an integer value:
   * Return 0 to send the response immediately
   * Return non-zero to delay the response (to be sent later)

2. When delaying a response, the application can call `bt_mesh_vendor_srv_status_send()` later when the response is ready. Make sure to save the `ctx` so that response can be sent to the correct destination.

3. This allows for scenarios where response data isn't immediately available, such as:
   * Hardware operations that take time to complete
   * Communication with other subsystems
   * Operations that require user input
