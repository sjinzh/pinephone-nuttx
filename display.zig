//***************************************************************************
//
// Licensed to the Apache Software Foundation (ASF) under one or more
// contributor license agreements.  See the NOTICE file distributed with
// this work for additional information regarding copyright ownership.  The
// ASF licenses this file to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance with the
// License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
// WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
// License for the specific language governing permissions and limitations
// under the License.
//
//***************************************************************************

//! PinePhone MIPI DSI Driver for Apache NuttX RTOS.
//! This MIPI DSI Interface is compatible with Zephyr MIPI DSI:
//! https://github.com/zephyrproject-rtos/zephyr/blob/main/include/zephyr/drivers/mipi_dsi.h

/// Import the Zig Standard Library
const std = @import("std");

/// Import the LoRaWAN Library from C
const c = @cImport({
    // NuttX Defines
    @cDefine("__NuttX__",  "");
    @cDefine("NDEBUG",     "");
    @cDefine("FAR",        "");

    // NuttX Header Files
    @cInclude("arch/types.h");
    @cInclude("../../nuttx/include/limits.h");
    @cInclude("nuttx/config.h");
    @cInclude("nuttx/crc16.h");
    @cInclude("inttypes.h");
    @cInclude("unistd.h");
    @cInclude("stdlib.h");
    @cInclude("stdio.h");
});

/// MIPI DSI Processor-to-Peripheral transaction types
const MIPI_DSI_DCS_LONG_WRITE = 0x39;

/// Write to MIPI DSI. See https://lupyuen.github.io/articles/dsi#transmit-packet-over-mipi-dsi
pub export fn nuttx_mipi_dsi_dcs_write(
    dev: [*c]const mipi_dsi_device,  // MIPI DSI Host Device
    channel: u8,  // Virtual Channel ID
    cmd: u8,      // DCS Command
    buf: [*c]u8,  // Transmit Buffer
    len: usize    // Length of Buffer
) isize {  // On Success: Return number of written bytes. On Error: Return negative error code
    _ = dev;
    debug("mipi_dsi_dcs_write: channel={}, cmd={x}, len={}", .{ channel, cmd, len });
    assert(cmd == MIPI_DSI_DCS_LONG_WRITE);

    // Compose Long Packet
    const pkt = composeLongPacket(channel, cmd, buf, len);

    // TODO: Dump the packet
    _ = pkt;

    // TODO
    // - Write the Long Packet to DSI_CMD_TX_REG 
    //   (DSI Low Power Transmit Package Register) at Offset 0x300 to 0x3FC.
    //
    // - Set the Packet Length (TX_Size) in Bits 0 to 7 of 
    //   DSI_CMD_CTL_REG (DSI Low Power Control Register) at Offset 0x200.
    //
    // - Set DSI_INST_JUMP_SEL_REG (Offset 0x48, undocumented) 
    //   to begin the Low Power Transmission.
    //
    // - Disable DSI Processing: Set Instru_En to 0.
    // - Then Enable DSI Processing: Set Instru_En to 1.
    //
    // - To check whether the transmission is complete, we poll on Instru_En.
    //
    // Instru_En is Bit 0 of DSI_BASIC_CTL0_REG 
    // (DSI Configuration Register 0) at Offset 0x10.

    std.debug.panic("nuttx_mipi_dsi_dcs_write not implemented", .{});
    return 0;
}

// Compose MIPI DSI Long Packet. See https://lupyuen.github.io/articles/dsi#long-packet-for-mipi-dsi
fn composeLongPacket(
    channel: u8,  // Virtual Channel ID
    cmd: u8,      // DCS Command
    buf: [*c]u8,  // Transmit Buffer
    len: usize    // Length of Buffer
) []u8 {
    _ = buf;
    debug("composeLongPacket: channel={}, cmd={x}, len={}", .{ channel, cmd, len });
    // Data Identifier (DI) (1 byte):
    // - Virtual Channel Identifier (Bits 6 to 7)
    // - Data Type (Bits 0 to 5)
    // (Virtual Channel should be 0, I think)
    assert(cmd < (1 << 6));
    const vc: u8 = 0;
    const dt: u8 = cmd;
    const di: u8 = (vc << 6) | dt;

    // Word Count (WC) (2 bytes)：
    // - Number of bytes in the Packet Payload
    const wc: u16 = @intCast(u16, len);
    const wcl: u8 = @intCast(u8, wc & 0xff);
    const wch: u8 = @intCast(u8, wc >> 8);

    // Data Identifier + Word Count (3 bytes): For computing Error Correction Code (ECC)
    const di_wc = [3]u8 { di, wcl, wch };

    // Compute Error Correction Code (ECC) for Data Identifier + Word Count
    const ecc: u8 = computeEcc(di_wc);

    // Packet Header (4 bytes):
    // - Data Identifier + Word Count + Error Correction COde
    const header = [4]u8 { di_wc[0], di_wc[1], di_wc[2], ecc };

    // Packet Payload:
    // - Data (0 to 65,541 bytes):
    // Number of data bytes should match the Word Count (WC)
    assert(len <= 65_541);
    const payload = buf[0..len];

    // Packet = Packet Header + Payload + Packet Footer
    var pkt = std.mem.zeroes([1024]u8);
    const pktlen = header.len + len + 2;  // Packet Footer is 2 bytes
    assert(pktlen <= pkt.len);  // Increase `pkt` size
    std.mem.copy(u8, pkt[0..header.len], &header); // 4 bytes
    std.mem.copy(u8, pkt[header.len..], payload);  // `len` bytes
    // Packet Footer comes later, after we have computed CRC...

    // Checksum (CS) (2 bytes):
    // - 16-bit Cyclic Redundancy Check (CRC)
    const cs: u16 = computeCrc(pkt[0..(pktlen - 2)]);
    const csl: u8 = @intCast(u8, cs & 0xff);
    const csh: u8 = @intCast(u8, cs >> 8);

    // Packet Footer (2 bytes)
    // - Checksum (CS)
    const footer = [2]u8 { csl, csh };

    // Append Packet Footer to Packet
    std.mem.copy(u8, pkt[(pktlen - 2)..], &footer);  // 2 bytes

    // Return the packet
    return pkt[0..pktlen];
}

/// Compute the Error Correction Code (ECC) (1 byte):
/// Allow single-bit errors to be corrected and 2-bit errors to be detected in the Packet Header
/// See "12.3.6.12: Error Correction Code", Page 208 of BL808 Reference Manual:
/// https://github.com/sipeed/sipeed2022_autumn_competition/blob/main/assets/BL808_RM_en.pdf)
fn computeEcc(
    di_wc: [3]u8  // Data Identifier + Word Count (3 bytes)
) u8 {
    // Combine DI and WC into a 24-bit word
    var di_wc_word: u32 = 
        di_wc[0] 
        | (@intCast(u32, di_wc[1]) << 8)
        | (@intCast(u32, di_wc[2]) << 16);

    // Extract the 24 bits from the word
    var d = std.mem.zeroes([24]u1);
    var i: usize = 0;
    while (i < 24) : (i += 1) {
        d[i] = @intCast(u1, di_wc_word & 1);
        di_wc_word >>= 1;
    }

    // Compute the ECC bits
    var ecc = std.mem.zeroes([8]u1);
    ecc[7] = 0;
    ecc[6] = 0;
    ecc[5] = d[10] ^ d[11] ^ d[12] ^ d[13] ^ d[14] ^ d[15] ^ d[16] ^ d[17] ^ d[18] ^ d[19] ^ d[21] ^ d[22] ^ d[23];
    ecc[4] = d[4]  ^ d[5]  ^ d[6]  ^ d[7]  ^ d[8]  ^ d[9]  ^ d[16] ^ d[17] ^ d[18] ^ d[19] ^ d[20] ^ d[22] ^ d[23];
    ecc[3] = d[1]  ^ d[2]  ^ d[3]  ^ d[7]  ^ d[8]  ^ d[9]  ^ d[13] ^ d[14] ^ d[15] ^ d[19] ^ d[20] ^ d[21] ^ d[23];
    ecc[2] = d[0]  ^ d[2]  ^ d[3]  ^ d[5]  ^ d[6]  ^ d[9]  ^ d[11] ^ d[12] ^ d[15] ^ d[18] ^ d[20] ^ d[21] ^ d[22];
    ecc[1] = d[0]  ^ d[1]  ^ d[3]  ^ d[4]  ^ d[6]  ^ d[8]  ^ d[10] ^ d[12] ^ d[14] ^ d[17] ^ d[20] ^ d[21] ^ d[22] ^ d[23];
    ecc[0] = d[0]  ^ d[1]  ^ d[2]  ^ d[4]  ^ d[5]  ^ d[7]  ^ d[10] ^ d[11] ^ d[13] ^ d[16] ^ d[20] ^ d[21] ^ d[22] ^ d[23];

    // Merge the ECC bits
    return @intCast(u8, ecc[0])
        | (@intCast(u8, ecc[1]) << 1)
        | (@intCast(u8, ecc[2]) << 2)
        | (@intCast(u8, ecc[3]) << 3)
        | (@intCast(u8, ecc[4]) << 4)
        | (@intCast(u8, ecc[5]) << 5)
        | (@intCast(u8, ecc[6]) << 6)
        | (@intCast(u8, ecc[7]) << 7);
}

/// Compute 16-bit Cyclic Redundancy Check (CRC).
/// See "12.3.6.13: Packet Footer", Page 210 of BL808 Reference Manual:
/// https://github.com/sipeed/sipeed2022_autumn_competition/blob/main/assets/BL808_RM_en.pdf)
fn computeCrc(
    data: []u8
) u16 {
    // Use NuttX CRC16
    return c.crc16(data.ptr, data.len);
}

///////////////////////////////////////////////////////////////////////////////
//  MIPI DSI Types

/// MIPI DSI Device
pub const mipi_dsi_device = extern struct {
    /// Number of Data Lanes
    data_lanes: u8,
    /// Display Timings
    timings: mipi_dsi_timings,
    /// Pixel Format
    pixfmt: u32,
    /// Mode Flags
    mode_flags: u32,
};

/// MIPI DSI Read / Write Message
pub const mipi_dsi_msg = extern struct {
    /// Payload Data Type
    type: u8,
    /// Flags controlling message transmission
    flags: u16,
    /// Command (only for DCS)
    cmd: u8,
    /// Transmit Buffer Length
    tx_len: usize,
    /// Transmit Buffer
    tx_buf: [*c]const u8,
    /// Receive Buffer Length
    rx_len: usize,
    /// Receive Buffer
    rx_buf: [*c]u8,
};

/// MIPI DSI Display Timings
pub const mipi_dsi_timings = extern struct {
    /// Horizontal active video
    hactive: u32,
    /// Horizontal front porch
    hfp: u32,
    /// Horizontal back porch
    hbp: u32,
    /// Horizontal sync length
    hsync: u32,
    /// Vertical active video
    vactive: u32,
    /// Vertical front porch
    vfp: u32,
    /// Vertical back porch
    vbp: u32,
    /// Vertical sync length
    vsync: u32,
};

///////////////////////////////////////////////////////////////////////////////
//  Test Functions

/// Main Function for Null App
pub export fn null_main(_argc: c_int, _argv: [*]const [*]const u8) c_int {
    _ = _argc;
    _ = _argv;
    test_zig();
    return 0;
}

/// Zig Test Function
pub export fn test_zig() void {
    _ = printf("HELLO ZIG ON PINEPHONE!\n");
    _ = nuttx_mipi_dsi_dcs_write(null, 0, MIPI_DSI_DCS_LONG_WRITE, null, 0);
}

///////////////////////////////////////////////////////////////////////////////
//  Panic Handler

/// Called by Zig when it hits a Panic. We print the Panic Message, Stack Trace and halt. See 
/// https://andrewkelley.me/post/zig-stack-traces-kernel-panic-bare-bones-os.html
/// https://github.com/ziglang/zig/blob/master/lib/std/builtin.zig#L763-L847
pub fn panic(
    message: []const u8, 
    _stack_trace: ?*std.builtin.StackTrace
) noreturn {
    // Print the Panic Message
    _ = _stack_trace;
    _ = puts("\n!ZIG PANIC!");
    _ = puts(@ptrCast([*c]const u8, message));

    // Print the Stack Trace
    _ = puts("Stack Trace:");
    var it = std.debug.StackIterator.init(@returnAddress(), null);
    while (it.next()) |return_address| {
        _ = printf("%p\n", return_address);
    }

    // Halt
    c.exit(1);
}

///////////////////////////////////////////////////////////////////////////////
//  Logging

/// Called by Zig for `std.log.debug`, `std.log.info`, `std.log.err`, ...
/// https://gist.github.com/leecannon/d6f5d7e5af5881c466161270347ce84d
pub fn log(
    comptime _message_level: std.log.Level,
    comptime _scope: @Type(.EnumLiteral),
    comptime format: []const u8,
    args: anytype,
) void {
    _ = _message_level;
    _ = _scope;

    // Format the message
    var buf: [100]u8 = undefined;  // Limit to 100 chars
    var slice = std.fmt.bufPrint(&buf, format, args)
        catch { _ = puts("*** log error: buf too small"); return; };
    
    // Terminate the formatted message with a null
    var buf2: [buf.len + 1 : 0]u8 = undefined;
    std.mem.copy(
        u8, 
        buf2[0..slice.len], 
        slice[0..slice.len]
    );
    buf2[slice.len] = 0;

    // Print the formatted message
    _ = puts(&buf2);
}

///////////////////////////////////////////////////////////////////////////////
//  Imported Functions and Variables

/// For safety, we import these functions ourselves to enforce Null-Terminated Strings.
/// We changed `[*c]const u8` to `[*:0]const u8`
extern fn printf(format: [*:0]const u8, ...) c_int;
extern fn puts(str: [*:0]const u8) c_int;

/// LoRaWAN Event Queue
extern var event_queue: c.struct_ble_npl_eventq;

/// Aliases for Zig Standard Library
const assert = std.debug.assert;
const debug  = std.log.debug;