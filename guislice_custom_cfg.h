#ifndef _GUISLICE_CONFIG_ARD_H_
#define _GUISLICE_CONFIG_ARD_H_

#ifdef __cplusplus
extern "C" {
#endif

  #define DRV_DISP_TFT_ESPI         // bodmer/TFT_eSPI
  #define DRV_TOUCH_TFT_ESPI        // TFT_eSPI integrated XPT2046 touch driver
  #define DRV_TOUCH_IN_DISP         // Use display driver (TFT_eSPI) for touch events
  #define DRV_TOUCH_TFT_ESPI_FILTER // Enable 5-sample debounce filter on XPT2046 reads

  #define ADAGFX_PIN_SDCS     14

  // Set Default rotation of the display (0,1,2,3)
  #define GSLC_ROTATE         1

  // Calibration settings for AZ-Touch MOD XPT2046 (from esp-tftespi-default-xpt2046.h):
  #define ADATOUCH_X_MIN      246
  #define ADATOUCH_X_MAX      3837
  #define ADATOUCH_Y_MIN      3925
  #define ADATOUCH_Y_MAX      370
  #define ADATOUCH_REMAP_YX   0

  #define TFT_ESPI_TOUCH_CALIB { 286,3534,283,3600,6 }

  #define ADATOUCH_PRESS_MIN  400
  #define ADATOUCH_PRESS_MAX  4000

  #define DEBUG_ERR           1
  // IMPORTANTE: NON definire DBG_TOUCH! In GUIslice.c viene controllato con #ifdef DBG_TOUCH
  // (anche #define DBG_TOUCH 0 lo attiva!) e disegna quadratini gialli 3x3 sullo schermo ad ogni tocco!
  //#define DBG_TOUCH

  #define GSLC_FEATURE_COMPOUND       0
  #define GSLC_FEATURE_XTEXTBOX_EMBED 0
  #define GSLC_FEATURE_INPUT          0

  #define GSLC_SD_EN          0
  #define GSLC_SPIFFS_EN      0

  #define GSLC_TOUCH_MAX_EVT  1
  #define GSLC_SD_BUFFPIXEL   50
  #define GSLC_CLIP_EN        1
  #define GSLC_BMP_TRANS_EN   1
  #define GSLC_BMP_TRANS_RGB  0xFF,0x00,0xFF
  #define GSLC_USE_FLOAT      0
  #define GSLC_DEV_TOUCH      ""
  #define GSLC_USE_PROGMEM    0
  #define GSLC_LOCAL_STR      0
  #define GSLC_LOCAL_STR_LEN  30

#ifdef __cplusplus
}
#endif
#endif // _GUISLICE_CONFIG_ARD_H_
