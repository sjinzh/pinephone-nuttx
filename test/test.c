#include <nuttx/config.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <debug.h>

#include <nuttx/arch.h>
#include "arm64_arch.h"
#include "mipi_dsi.h"
#include "a64_mipi_dsi.h"
#include "a64_mipi_dphy.h"
#include "a64_tcon0.h"
#include "a64_de.h"

// TODO: Fix test code
#include "test_mipi_dsi.c"
// TODO: #include "test_a64_mipi_dsi.c"  // For pinephone_panel_init

int pinephone_panel_init(void);
void test_pattern(void);

/// TODO: Move this

#define FB_FMT_RGBA32         21          /* BPP=32 Raw RGB with alpha */
typedef uint16_t fb_coord_t;

struct fb_videoinfo_s
{
  uint8_t    fmt;               /* see FB_FMT_*  */
  fb_coord_t xres;              /* Horizontal resolution in pixel columns */
  fb_coord_t yres;              /* Vertical resolution in pixel rows */
  uint8_t    nplanes;           /* Number of color planes supported */
#ifdef CONFIG_FB_OVERLAY
  uint8_t    noverlays;         /* Number of overlays supported */
#endif
#ifdef CONFIG_FB_MODULEINFO
  uint8_t    moduleinfo[128];   /* Module information filled by vendor */
#endif
};

struct fb_planeinfo_s
{
  FAR void  *fbmem;        /* Start of frame buffer memory */
  size_t     fblen;        /* Length of frame buffer memory in bytes */
  fb_coord_t stride;       /* Length of a line in bytes */
  uint8_t    display;      /* Display number */
  uint8_t    bpp;          /* Bits per pixel */
  uint32_t   xres_virtual; /* Virtual Horizontal resolution in pixel columns */
  uint32_t   yres_virtual; /* Virtual Vertical resolution in pixel rows */
  uint32_t   xoffset;      /* Offset from virtual to visible resolution */
  uint32_t   yoffset;      /* Offset from virtual to visible resolution */
};

struct fb_area_s
{
  fb_coord_t x;           /* x-offset of the area */
  fb_coord_t y;           /* y-offset of the area */
  fb_coord_t w;           /* Width of the area */
  fb_coord_t h;           /* Height of the area */
};

struct fb_transp_s
{
  uint8_t    transp;      /* Transparency */
  uint8_t    transp_mode; /* Transparency mode */
};

struct fb_overlayinfo_s
{
  FAR void   *fbmem;          /* Start of frame buffer memory */
  size_t     fblen;           /* Length of frame buffer memory in bytes */
  fb_coord_t stride;          /* Length of a line in bytes */
  uint8_t    overlay;         /* Overlay number */
  uint8_t    bpp;             /* Bits per pixel */
  uint8_t    blank;           /* Blank or unblank */
  uint32_t   chromakey;       /* Chroma key argb8888 formatted */
  uint32_t   color;           /* Color argb8888 formatted */
  struct fb_transp_s transp;  /* Transparency */
  struct fb_area_s sarea;     /* Selected area within the overlay */
  uint32_t   accl;            /* Supported hardware acceleration */
};

//// Done

/// NuttX Video Controller for PinePhone (3 UI Channels)
static struct fb_videoinfo_s videoInfo = {
    .fmt       = FB_FMT_RGBA32,  // Pixel format (XRGB 8888)
    .xres      = A64_TCON0_PANEL_WIDTH,      // Horizontal resolution in pixel columns
    .yres      = A64_TCON0_PANEL_HEIGHT,     // Vertical resolution in pixel rows
    .nplanes   = 1,     // Number of color planes supported (Base UI Channel)
    .noverlays = 2,     // Number of overlays supported (2 Overlay UI Channels)
};

// Framebuffer 0: (Base UI Channel)
// Fullscreen 720 x 1440 (4 bytes per XRGB 8888 pixel)
static uint32_t fb0[A64_TCON0_PANEL_WIDTH * A64_TCON0_PANEL_HEIGHT];

// Framebuffer 1: (First Overlay UI Channel)
// Square 600 x 600 (4 bytes per ARGB 8888 pixel)
#define FB1_WIDTH  600
#define FB1_HEIGHT 600
static uint32_t fb1[FB1_WIDTH * FB1_HEIGHT];

// Framebuffer 2: (Second Overlay UI Channel)
// Fullscreen 720 x 1440 (4 bytes per ARGB 8888 pixel)
static uint32_t fb2[A64_TCON0_PANEL_WIDTH * A64_TCON0_PANEL_HEIGHT];

/// NuttX Color Plane for PinePhone (Base UI Channel):
/// Fullscreen 720 x 1440 (4 bytes per XRGB 8888 pixel)
static struct fb_planeinfo_s planeInfo = {
    .fbmem   = &fb0,     // Start of frame buffer memory
    .fblen   = sizeof(fb0),  // Length of frame buffer memory in bytes
    .stride  = A64_TCON0_PANEL_WIDTH * 4,  // Length of a line in bytes (4 bytes per pixel)
    .display = 0,        // Display number (Unused)
    .bpp     = 32,       // Bits per pixel (XRGB 8888)
    .xres_virtual = A64_TCON0_PANEL_WIDTH,   // Virtual Horizontal resolution in pixel columns
    .yres_virtual = A64_TCON0_PANEL_HEIGHT,  // Virtual Vertical resolution in pixel rows
    .xoffset      = 0,     // Offset from virtual to visible resolution
    .yoffset      = 0,     // Offset from virtual to visible resolution
};

/// NuttX Overlays for PinePhone (2 Overlay UI Channels)
static struct fb_overlayinfo_s overlayInfo[2] = {
    // First Overlay UI Channel:
    // Square 600 x 600 (4 bytes per ARGB 8888 pixel)
    {
        .fbmem     = &fb1,     // Start of frame buffer memory
        .fblen     = sizeof(fb1),  // Length of frame buffer memory in bytes
        .stride    = FB1_WIDTH * 4,  // Length of a line in bytes
        .overlay   = 0,        // Overlay number (First Overlay)
        .bpp       = 32,       // Bits per pixel (ARGB 8888)
        .blank     = 0,        // TODO: Blank or unblank
        .chromakey = 0,        // TODO: Chroma key argb8888 formatted
        .color     = 0,        // TODO: Color argb8888 formatted
        .transp    = { .transp = 0, .transp_mode = 0 },  // TODO: Transparency
        .sarea     = { .x = 52, .y = 52, .w = FB1_WIDTH, .h = FB1_HEIGHT },  // Selected area within the overlay
        .accl      = 0,        // TODO: Supported hardware acceleration
    },
    // Second Overlay UI Channel:
    // Fullscreen 720 x 1440 (4 bytes per ARGB 8888 pixel)
    {
        .fbmem     = &fb2,     // Start of frame buffer memory
        .fblen     = sizeof(fb2),  // Length of frame buffer memory in bytes
        .stride    = A64_TCON0_PANEL_WIDTH * 4,  // Length of a line in bytes
        .overlay   = 1,        // Overlay number (Second Overlay)
        .bpp       = 32,       // Bits per pixel (ARGB 8888)
        .blank     = 0,        // TODO: Blank or unblank
        .chromakey = 0,        // TODO: Chroma key argb8888 formatted
        .color     = 0,        // TODO: Color argb8888 formatted
        .transp    = { .transp = 0, .transp_mode = 0 },  // TODO: Transparency
        .sarea     = { .x = 0, .y = 0, .w = A64_TCON0_PANEL_WIDTH, .h = A64_TCON0_PANEL_HEIGHT },  // Selected area within the overlay
        .accl      = 0,        // TODO: Supported hardware acceleration
    },
};

int main()
{
  int ret;

  ginfo("TODO: Turn on Display Backlight\n");

  // Init Timing Controller TCON0
  ret = a64_tcon0_init();
  assert(ret == OK);
  
  ginfo("TODO: Init PMIC\n");

  // Enable MIPI DSI Block
  ret = a64_mipi_dsi_enable();
  assert(ret == OK);

  // Enable MIPI Display Physical Layer (DPHY)
  ret = a64_mipi_dphy_enable();
  assert(ret == OK);

  ginfo("TODO: Reset LCD Panel\n");

  // Initialise LCD Controller (ST7703)
  ret = pinephone_panel_init();
  assert(ret == OK);

  // Start MIPI DSI HSC and HSD
  ret = a64_mipi_dsi_start();
  assert(ret == OK);

  // Fill Framebuffer with Test Pattern
  test_pattern();

  // Init Display Engine
  ret = a64_de_init();
  assert(ret == OK);

  // Wait 160 milliseconds
  up_mdelay(160);

  // Init the UI Blender for PinePhone's A64 Display Engine
  ret = a64_de_blender_init();
  assert(ret == OK);

  // Init the Base UI Channel
  ret = a64_de_ui_channel_init(
    1,  // UI Channel Number (1 for Base UI Channel)
    planeInfo.fbmem,    // Start of frame buffer memory
    planeInfo.fblen,    // Length of frame buffer memory in bytes
    planeInfo.stride,   // Length of a line in bytes (4 bytes per pixel)
    planeInfo.xres_virtual,  // Horizontal resolution in pixel columns
    planeInfo.yres_virtual,  // Vertical resolution in pixel rows
    planeInfo.xoffset,  // Horizontal offset in pixel columns
    planeInfo.yoffset  // Vertical offset in pixel rows
  );
  assert(ret == OK);

  // Init the 2 Overlay UI Channels
  #define CHANNELS 3
  for (int i = 0; i < sizeof(overlayInfo) / sizeof(overlayInfo[0]); i++)
  {
    const struct fb_overlayinfo_s *ov = &overlayInfo[i];
    ret = a64_de_ui_channel_init(
      i + 2,  // UI Channel Number (2 and 3 for Overlay UI Channels)
      (CHANNELS == 3) ? ov->fbmem : NULL,  // Start of frame buffer memory
      ov->fblen,    // Length of frame buffer memory in bytes
      ov->stride,   // Length of a line in bytes (4 bytes per pixel)
      ov->sarea.w,  // Horizontal resolution in pixel columns
      ov->sarea.h,  // Vertical resolution in pixel rows
      ov->sarea.x,  // Horizontal offset in pixel columns
      ov->sarea.y  // Vertical offset in pixel rows
    );
    assert(ret == OK);
  }

  // Set UI Blender Route, enable Blender Pipes and apply the settings
  ret = a64_de_enable();
  assert(ret == OK);

  // Test MIPI DSI
  void mipi_dsi_test(void);
  mipi_dsi_test();
}

#warning TODO: test_pattern
void test_pattern(void)
{
  memset(fb0, 0, sizeof(fb0));
  memset(fb1, 0, sizeof(fb1));
  memset(fb2, 0, sizeof(fb2));
}

// TODO: Sync with nuttx/libs/libc/misc/lib_crc16ccitt.c
static uint16_t crc16ccitt_tab[256] =
{
  0x0000, 0x1189, 0x2312, 0x329b, 0x4624, 0x57ad, 0x6536, 0x74bf,
  0x8c48, 0x9dc1, 0xaf5a, 0xbed3, 0xca6c, 0xdbe5, 0xe97e, 0xf8f7,
  0x1081, 0x0108, 0x3393, 0x221a, 0x56a5, 0x472c, 0x75b7, 0x643e,
  0x9cc9, 0x8d40, 0xbfdb, 0xae52, 0xdaed, 0xcb64, 0xf9ff, 0xe876,
  0x2102, 0x308b, 0x0210, 0x1399, 0x6726, 0x76af, 0x4434, 0x55bd,
  0xad4a, 0xbcc3, 0x8e58, 0x9fd1, 0xeb6e, 0xfae7, 0xc87c, 0xd9f5,
  0x3183, 0x200a, 0x1291, 0x0318, 0x77a7, 0x662e, 0x54b5, 0x453c,
  0xbdcb, 0xac42, 0x9ed9, 0x8f50, 0xfbef, 0xea66, 0xd8fd, 0xc974,
  0x4204, 0x538d, 0x6116, 0x709f, 0x0420, 0x15a9, 0x2732, 0x36bb,
  0xce4c, 0xdfc5, 0xed5e, 0xfcd7, 0x8868, 0x99e1, 0xab7a, 0xbaf3,
  0x5285, 0x430c, 0x7197, 0x601e, 0x14a1, 0x0528, 0x37b3, 0x263a,
  0xdecd, 0xcf44, 0xfddf, 0xec56, 0x98e9, 0x8960, 0xbbfb, 0xaa72,
  0x6306, 0x728f, 0x4014, 0x519d, 0x2522, 0x34ab, 0x0630, 0x17b9,
  0xef4e, 0xfec7, 0xcc5c, 0xddd5, 0xa96a, 0xb8e3, 0x8a78, 0x9bf1,
  0x7387, 0x620e, 0x5095, 0x411c, 0x35a3, 0x242a, 0x16b1, 0x0738,
  0xffcf, 0xee46, 0xdcdd, 0xcd54, 0xb9eb, 0xa862, 0x9af9, 0x8b70,
  0x8408, 0x9581, 0xa71a, 0xb693, 0xc22c, 0xd3a5, 0xe13e, 0xf0b7,
  0x0840, 0x19c9, 0x2b52, 0x3adb, 0x4e64, 0x5fed, 0x6d76, 0x7cff,
  0x9489, 0x8500, 0xb79b, 0xa612, 0xd2ad, 0xc324, 0xf1bf, 0xe036,
  0x18c1, 0x0948, 0x3bd3, 0x2a5a, 0x5ee5, 0x4f6c, 0x7df7, 0x6c7e,
  0xa50a, 0xb483, 0x8618, 0x9791, 0xe32e, 0xf2a7, 0xc03c, 0xd1b5,
  0x2942, 0x38cb, 0x0a50, 0x1bd9, 0x6f66, 0x7eef, 0x4c74, 0x5dfd,
  0xb58b, 0xa402, 0x9699, 0x8710, 0xf3af, 0xe226, 0xd0bd, 0xc134,
  0x39c3, 0x284a, 0x1ad1, 0x0b58, 0x7fe7, 0x6e6e, 0x5cf5, 0x4d7c,
  0xc60c, 0xd785, 0xe51e, 0xf497, 0x8028, 0x91a1, 0xa33a, 0xb2b3,
  0x4a44, 0x5bcd, 0x6956, 0x78df, 0x0c60, 0x1de9, 0x2f72, 0x3efb,
  0xd68d, 0xc704, 0xf59f, 0xe416, 0x90a9, 0x8120, 0xb3bb, 0xa232,
  0x5ac5, 0x4b4c, 0x79d7, 0x685e, 0x1ce1, 0x0d68, 0x3ff3, 0x2e7a,
  0xe70e, 0xf687, 0xc41c, 0xd595, 0xa12a, 0xb0a3, 0x8238, 0x93b1,
  0x6b46, 0x7acf, 0x4854, 0x59dd, 0x2d62, 0x3ceb, 0x0e70, 0x1ff9,
  0xf78f, 0xe606, 0xd49d, 0xc514, 0xb1ab, 0xa022, 0x92b9, 0x8330,
  0x7bc7, 0x6a4e, 0x58d5, 0x495c, 0x3de3, 0x2c6a, 0x1ef1, 0x0f78,
};

// TODO: Sync with nuttx/libs/libc/misc/lib_crc16ccitt.c
uint16_t crc16ccittpart(FAR const uint8_t *src, size_t len,
                        uint16_t crc16val)
{
  size_t i;
  uint16_t v = crc16val;

  for (i = 0; i < len; i++)
    {
      v = (v >> 8) ^ crc16ccitt_tab[(v ^ src[i]) & 0xff];
    }

  return v;
}

void dump_buffer(const uint8_t *data, size_t len)
{
    char buf[8 * 3];
    memset(buf, ' ', sizeof(buf));
    buf[sizeof(buf) - 1] = 0;

	for (int i = 0; i < len; i++) {
        const int mod = i % 8;
        const int d1 = data[i] >> 4;
        const int d2 = data[i] & 0b1111;
        buf[mod * 3] = (d1 < 10) ? ('0' + d1) : ('a' + d1 - 10);
        buf[mod * 3 + 1] = (d2 < 10) ? ('0' + d2) : ('a' + d2 - 10);

        if ((i + 1) % 8 == 0 || i == len - 1) {
            ginfo("%s\n", buf);
            if (i == len - 1) { break; }

            memset(buf, ' ', sizeof(buf));
            buf[sizeof(buf) - 1] = 0;
        }
	}
}

/// Modify the specified bits in a memory mapped register.
/// Based on https://github.com/apache/nuttx/blob/master/arch/arm64/src/common/arm64_arch.h#L473
void modreg32(
    uint32_t val,   // Bits to set, like (1 << bit)
    uint32_t mask,  // Bits to clear, like (1 << bit)
    unsigned long addr  // Address to modify
)
{
  ginfo("  *0x%lx: clear 0x%x, set 0x%x\n", addr, mask, val & mask);
  assert((val & mask) == val);
}

uint32_t getreg32(unsigned long addr)
{
  return 0;
}

void putreg32(uint32_t data, unsigned long addr)
{
  ginfo("  *0x%lx = 0x%x\n", addr, data);
}

void up_mdelay(unsigned int milliseconds)
{
  ginfo("  up_mdelay %d ms\n", milliseconds);
}
