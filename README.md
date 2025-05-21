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

The sample implements three message types:

1. **Vendor_SET (Opcode: 0x10 + Company ID)**
   - Sent from client to server
   - Carries arbitrary length data payload (up to 377 bytes)
   - STATUS response is expected to be sent by the application

2. **Vendor_GET (Opcode: 0x11 + Company ID)**
   - Sent from client to server
   - No payload
   - Requires acknowledgment with a Vendor_STATUS response

3. **Vendor_STATUS (Opcode: 0x12 + Company ID)**
   - Sent from server to client
   - Contains response data (up to 377 bytes)
   - Sent in response to GET or SET messages
   - Automatically sent by the server when the handler returns success (0)

## Requirements

### Hardware

* Two nRF52 or nRF53 series development boards (e.g., nRF52840 DK, nRF5340 DK)
* Smartphone with nRF Mesh mobile app (for provisioning)

### Software

* nRF Connect SDK (NCS) v2.4.0 or later
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
3. Press Button 1 on one of the devices to send a "Hello World" message using Vendor_SET message
4. Press Button 2 on one of the devices to send a Vendor_GET request message
5. Observe the message exchange in the console logs

### Expected Output

On the device that receives the SET message, you should see:

```
[00:00:06.156,066] <dbg> vnd_srv: handle_set: Received SET message, data length 301
[00:00:06.156,250] <inf> model_handler: Received SET message: "Hello World- 0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF"
[00:00:06.156,311] <dbg> vnd_srv: bt_mesh_vendor_srv_status_send: Sending STATUS message, data length 301
```

On the device that sends the SET message, you should see:

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

2. When delaying a response, the application can call `bt_mesh_vendor_srv_status_send()` later when the response is ready.

3. This allows for scenarios where response data isn't immediately available, such as:
   * Hardware operations that take time to complete
   * Communication with other subsystems
   * Operations that require user input
