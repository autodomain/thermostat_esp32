//<File !Start!>
// FILE: [gui_design_GSLC.h]
// Created by GUIslice Builder version: [0.16.b009]
//
// GUIslice Builder Generated GUI Framework File
//
// For the latest guides, updates and support view:
// https://github.com/ImpulseAdventure/GUIslice
//
//<File !End!>

#ifndef _GUISLICE_GEN_H
#define _GUISLICE_GEN_H

// ------------------------------------------------
// Headers to include
// ------------------------------------------------
#include "GUIslice.h"
#include "GUIslice_drv.h"

// Include any extended elements
//<Includes !Start!>
// Include extended elements
#include "elem/XCheckbox.h"
#include "elem/XGraph.h"
#include "elem/XSlider.h"
//<Includes !End!>

// ------------------------------------------------
// Headers and Defines for fonts
// Note that font files are located within the Adafruit-GFX library folder:
// ------------------------------------------------
//<Fonts !Start!>
#if !defined(DRV_DISP_TFT_ESPI)
  #error Project tab->Target Platform should be arduino
#endif
#include <TFT_eSPI.h>
//<Fonts !End!>

// ------------------------------------------------
// Defines for resources
// ------------------------------------------------
//<Resources !Start!>
//<Resources !End!>

// ------------------------------------------------
// Enumerations for pages, elements, fonts, images
// ------------------------------------------------
//<Enum !Start!>
enum {E_PG_MAIN,E_PG2};
enum {E_DRAW_LINE1,E_ELEM_BOX4,E_ELEM_BOX5,E_ELEM_BOX6,E_ELEM_BOX8
      ,E_ELEM_BTN10,E_ELEM_BTN_DEC,E_ELEM_BTN_INC,E_ELEM_BTN_MAIN_PAGE
      ,E_ELEM_BTN_MENU,E_ELEM_BTN_MODE,E_ELEM_BTN_SAVE
      ,E_ELEM_BTN_WAKE_UP,E_ELEM_BTN_WAKE_UP2,E_ELEM_GRAPH_TEMPERATURE
      ,E_ELEM_RADIO1,E_ELEM_RADIO2,E_ELEM_RADIO3,E_ELEM_RADIO4
      ,E_ELEM_RADIO5,E_ELEM_SLIDER_TEMP_OFFSET,E_ELEM_TEXT10
      ,E_ELEM_TEXT16,E_ELEM_TEXT18,E_ELEM_TEXT22,E_ELEM_TEXT23
      ,E_ELEM_TEXT29,E_ELEM_TEXT30,E_ELEM_TEXT31,E_ELEM_TEXT32
      ,E_ELEM_TEXT33,E_ELEM_TEXT34,E_ELEM_TEXT41,E_ELEM_TEXT44
      ,E_ELEM_TEXT48,E_ELEM_TEXT5,E_ELEM_TEXT50,E_ELEM_TEXT6
      ,E_ELEM_TEXT_STAT,E_ELEM_TEXT_TEMP_OFFSET
      ,E_ELEM_TEXT_TEMP_OUTSIDE,E_ELEM_TEXT_TEMP_SETPOINT
      ,E_ELEM_TEXT_status,E_ELEM_TEXT_WEATHER
      ,E_ELEM_BOX_STATUS_BAR,E_ELEM_BOX_WIFI_BARS};
// Must use separate enum for fonts with MAX_FONT at end to use gslc_FontSet.
enum {E_BUILTIN10X16,E_BUILTIN5X8,E_FREESANS12,E_FREESANS18
      ,E_FREESANS24,E_FREESANS9,MAX_FONT};
//<Enum !End!>

// ------------------------------------------------
// Instantiate the GUI
// ------------------------------------------------

// ------------------------------------------------
// Define the maximum number of elements and pages
// ------------------------------------------------
//<ElementDefines !Start!>
#define MAX_PAGE                2

#define MAX_ELEM_PG_MAIN 36 // # Elems total on page
#define MAX_ELEM_PG_MAIN_RAM MAX_ELEM_PG_MAIN // # Elems in RAM

#define MAX_ELEM_PG2 10 // # Elems total on page
#define MAX_ELEM_PG2_RAM MAX_ELEM_PG2 // # Elems in RAM
//<ElementDefines !End!>

// ------------------------------------------------
// Create element storage
// ------------------------------------------------
gslc_tsGui                      m_gui;
gslc_tsDriver                   m_drv;
gslc_tsFont                     m_asFont[MAX_FONT];
gslc_tsPage                     m_asPage[MAX_PAGE];

//<GUI_Extra_Elements !Start!>
gslc_tsElem                     m_asPage1Elem[MAX_ELEM_PG_MAIN_RAM];
gslc_tsElemRef                  m_asPage1ElemRef[MAX_ELEM_PG_MAIN];
gslc_tsElem                     m_asPage2Elem[MAX_ELEM_PG2_RAM];
gslc_tsElemRef                  m_asPage2ElemRef[MAX_ELEM_PG2];
gslc_tsXCheckbox                m_asXRadio3;
gslc_tsXCheckbox                m_asXRadio1;
gslc_tsXCheckbox                m_asXRadio2;
gslc_tsXCheckbox                m_asXRadio4;
gslc_tsXCheckbox                m_asXRadio5;
gslc_tsXGraph                   m_sGraph1;
int16_t                         m_anGraphBuf1[220]; // NRows=220
gslc_tsXSlider                  m_sXSlider1;

#define MAX_STR                 100

//<GUI_Extra_Elements !End!>

// ------------------------------------------------
// Program Globals
// ------------------------------------------------

// Element References for direct access
//<Extern_References !Start!>
extern gslc_tsElemRef* btn_dec_setpoint;
extern gslc_tsElemRef* btn_inc_setpoint;
extern gslc_tsElemRef* btn_main_page;
extern gslc_tsElemRef* btn_reset;
extern gslc_tsElemRef* m_pElemBtn_menu;
extern gslc_tsElemRef* m_pElemBtn_mode;
extern gslc_tsElemRef* m_pElemBtn_save;
extern gslc_tsElemRef* m_pElemBtn_wake_up;
extern gslc_tsElemRef* m_pElemBtn_wake_up2;
extern gslc_tsElemRef* m_pElemGraph_Temperature;
extern gslc_tsElemRef* m_pElemOutTxt5;
extern gslc_tsElemRef* m_pElemOutTxt6;
extern gslc_tsElemRef* m_pElemOutTxt_Date;
extern gslc_tsElemRef* m_pElemOutTxt_Ip_Address16;
extern gslc_tsElemRef* m_pElemOutTxt_Temp_Outside;
extern gslc_tsElemRef* m_pElemOutTxt_Temp_Setpoint;
extern gslc_tsElemRef* m_pElemOutTxt_Time;
extern gslc_tsElemRef* m_pElemOutTxt_stat;
extern gslc_tsElemRef* m_pElemOutTxt_status;
extern gslc_tsElemRef* m_pElemOutTxt_temp_offset;
extern gslc_tsElemRef* m_pElemRB5;
extern gslc_tsElemRef* m_pElemRB_sample;
extern gslc_tsElemRef* m_pElemRB_sample2;
extern gslc_tsElemRef* m_pElemRB_sample2_3;
extern gslc_tsElemRef* m_pElemRB_sample2_4;
extern gslc_tsElemRef* m_pElemSlider_temp_offset;
extern gslc_tsElemRef* m_pElemOutTxt_Weather;
extern gslc_tsElemRef* m_pElemOutTxt_SetpointLbl;
extern gslc_tsElemRef* m_pElemBox_StatusBar;
extern gslc_tsElemRef* m_pElemBox_WifiBars;
extern gslc_tsElemRef* m_pElemOutTxt_IndoorLbl;
extern gslc_tsElemRef* m_pElemOutTxt_GraphInfo;
//<Extern_References !End!>

// Define debug message function
static int16_t DebugOut(char ch);

// ------------------------------------------------
// Callback Methods
// ------------------------------------------------
bool CbBtnCommon(void* pvGui,void *pvElemRef,gslc_teTouch eTouch,int16_t nX,int16_t nY);
bool CbCheckbox(void* pvGui, void* pvElemRef, int16_t nSelId, bool bState);
bool CbDrawScanner(void* pvGui,void* pvElemRef,gslc_teRedrawType eRedraw);
bool CbDrawStatusBar(void* pvGui,void* pvElemRef,gslc_teRedrawType eRedraw);
bool CbDrawWifiBars(void* pvGui,void* pvElemRef,gslc_teRedrawType eRedraw);
bool CbKeypad(void* pvGui, void *pvElemRef, int16_t nState, void* pvData);
bool CbListbox(void* pvGui, void* pvElemRef, int16_t nSelId);
bool CbSlidePos(void* pvGui,void* pvElemRef,int16_t nPos);
bool CbSpinner(void* pvGui, void *pvElemRef, int16_t nState, void* pvData);
bool CbTickScanner(void* pvGui,void* pvScope);

// ------------------------------------------------
// Create page elements
// ------------------------------------------------
void InitGUIslice_gen()
{
  gslc_tsElemRef* pElemRef = NULL;

  if (!gslc_Init(&m_gui,&m_drv,m_asPage,MAX_PAGE,m_asFont,MAX_FONT)) { return; }

  // ------------------------------------------------
  // Load Fonts
  // ------------------------------------------------
//<Load_Fonts !Start!>
    if (!gslc_FontSet(&m_gui,E_BUILTIN10X16,GSLC_FONTREF_PTR,NULL,2)) { return; }
    if (!gslc_FontSet(&m_gui,E_BUILTIN5X8,GSLC_FONTREF_PTR,NULL,1)) { return; }
    if (!gslc_FontSet(&m_gui,E_FREESANS12,GSLC_FONTREF_PTR,&FreeSans12pt7b,1)) { return; }
    if (!gslc_FontSet(&m_gui,E_FREESANS18,GSLC_FONTREF_PTR,&FreeSans18pt7b,1)) { return; }
    if (!gslc_FontSet(&m_gui,E_FREESANS24,GSLC_FONTREF_PTR,&FreeSans24pt7b,1)) { return; }
    if (!gslc_FontSet(&m_gui,E_FREESANS9,GSLC_FONTREF_PTR,&FreeSans9pt7b,1)) { return; }
//<Load_Fonts !End!>

//<InitGUI !Start!>
  gslc_PageAdd(&m_gui,E_PG_MAIN,m_asPage1Elem,MAX_ELEM_PG_MAIN_RAM,m_asPage1ElemRef,MAX_ELEM_PG_MAIN);
  gslc_PageAdd(&m_gui,E_PG2,m_asPage2Elem,MAX_ELEM_PG2_RAM,m_asPage2ElemRef,MAX_ELEM_PG2);

  // NOTE: The current page defaults to the first page added. Here we explicitly
  //       ensure that the main page is the correct page no matter the add order.
  gslc_SetPageCur(&m_gui,E_PG_MAIN);
  
  // Set Background to a flat color
  gslc_SetBkgndColor(&m_gui,((gslc_tsColor){0,65,128}));

  // -----------------------------------
  // PAGE: E_PG_MAIN
  
  
  // create E_ELEM_BTN_INC button with text label
  pElemRef = gslc_ElemCreateBtnTxt(&m_gui,E_ELEM_BTN_INC,E_PG_MAIN,
    (gslc_tsRect){185,49,130,50},(char*)"+",0,E_FREESANS18,&CbBtnCommon);
  gslc_ElemSetCol(&m_gui,pElemRef,((gslc_tsColor){0,95,218}),((gslc_tsColor){0,75,153}),((gslc_tsColor){0,136,255}));
  gslc_ElemSetRoundEn(&m_gui, pElemRef, true);
  btn_inc_setpoint = pElemRef;
  
  // create E_ELEM_BTN_DEC button with text label
  pElemRef = gslc_ElemCreateBtnTxt(&m_gui,E_ELEM_BTN_DEC,E_PG_MAIN,
    (gslc_tsRect){185,180,130,50},(char*)"-",0,E_FREESANS18,&CbBtnCommon);
  gslc_ElemSetCol(&m_gui,pElemRef,((gslc_tsColor){0,95,218}),((gslc_tsColor){0,75,153}),((gslc_tsColor){0,136,255}));
  gslc_ElemSetRoundEn(&m_gui, pElemRef, true);
  btn_dec_setpoint = pElemRef;
   
  // Create E_ELEM_BOX4 box
  pElemRef = gslc_ElemCreateBox(&m_gui,E_ELEM_BOX4,E_PG_MAIN,(gslc_tsRect){4,104,170,125});
  gslc_ElemSetRoundEn(&m_gui, pElemRef, true);
  gslc_ElemSetCol(&m_gui,pElemRef,((gslc_tsColor){0,95,218}),((gslc_tsColor){0,75,153}),GSLC_COL_BLACK);
  
  // Create E_ELEM_TEXT6 runtime modifiable text
  static char m_sDisplayText6[8] = "--.-";
  pElemRef = gslc_ElemCreateTxt(&m_gui,E_ELEM_TEXT6,E_PG_MAIN,(gslc_tsRect){39,120,75,28},
    (char*)m_sDisplayText6,8,E_FREESANS18);
  gslc_ElemSetTxtCol(&m_gui,pElemRef,((gslc_tsColor){135,211,255}));
  gslc_ElemSetCol(&m_gui,pElemRef,GSLC_COL_GRAY,((gslc_tsColor){0,75,153}),GSLC_COL_BLACK);
  m_pElemOutTxt6 = pElemRef;

  // Create E_DRAW_LINE1 line 
  pElemRef = gslc_ElemCreateLine(&m_gui,E_DRAW_LINE1,E_PG_MAIN,9,42,309,42);
  gslc_ElemSetCol(&m_gui,pElemRef,GSLC_COL_BLACK,GSLC_COL_GRAY_LT2,GSLC_COL_GRAY_LT2);
  
  // Create E_ELEM_TEXT10 text label
  pElemRef = gslc_ElemCreateTxt(&m_gui,E_ELEM_TEXT10,E_PG_MAIN,(gslc_tsRect){124,133,30,15},
    (char*)"H%",0,E_FREESANS9);
  gslc_ElemSetCol(&m_gui,pElemRef,GSLC_COL_GRAY,((gslc_tsColor){0,75,153}),GSLC_COL_BLACK);
  gslc_ElemSetTxtEnc(&m_gui,pElemRef,GSLC_TXT_ENC_UTF8);
   
  // Create E_ELEM_BOX5 box
  pElemRef = gslc_ElemCreateBox(&m_gui,E_ELEM_BOX5,E_PG_MAIN,(gslc_tsRect){185,104,130,68});
  gslc_ElemSetRoundEn(&m_gui, pElemRef, true);
  gslc_ElemSetCol(&m_gui,pElemRef,((gslc_tsColor){0,95,218}),((gslc_tsColor){0,75,153}),GSLC_COL_BLACK);
  
  // Create E_ELEM_TEXT5 runtime modifiable text
  static char m_sDisplayText5[8] = "--.-";
  pElemRef = gslc_ElemCreateTxt(&m_gui,E_ELEM_TEXT5,E_PG_MAIN,(gslc_tsRect){14,178,102,37},
    (char*)m_sDisplayText5,8,E_FREESANS24);
  gslc_ElemSetTxtCol(&m_gui,pElemRef,((gslc_tsColor){135,211,255}));
  gslc_ElemSetCol(&m_gui,pElemRef,((gslc_tsColor){0,136,255}),((gslc_tsColor){0,75,153}),((gslc_tsColor){0,75,153}));
  m_pElemOutTxt5 = pElemRef;
  
  // Create E_ELEM_TEXT29 text label
  pElemRef = gslc_ElemCreateTxt(&m_gui,E_ELEM_TEXT29,E_PG_MAIN,(gslc_tsRect){297,149,12,15},
    (char*)"C",0,E_FREESANS9);
  gslc_ElemSetTxtCol(&m_gui,pElemRef,((gslc_tsColor){135,211,255}));
  gslc_ElemSetCol(&m_gui,pElemRef,GSLC_COL_GRAY,((gslc_tsColor){0,75,153}),GSLC_COL_BLACK);
  gslc_ElemSetTxtEnc(&m_gui,pElemRef,GSLC_TXT_ENC_UTF8);
  
  // Create E_ELEM_TEXT_TEMP_SETPOINT runtime modifiable text
  static char m_sDisplayText13[8] = "25.0";
  pElemRef = gslc_ElemCreateTxt(&m_gui,E_ELEM_TEXT_TEMP_SETPOINT,E_PG_MAIN,(gslc_tsRect){188,124,102,37},
    (char*)m_sDisplayText13,8,E_FREESANS24);
  gslc_ElemSetTxtCol(&m_gui,pElemRef,((gslc_tsColor){135,211,255}));
  gslc_ElemSetCol(&m_gui,pElemRef,GSLC_COL_GRAY,((gslc_tsColor){0,75,153}),GSLC_COL_BLACK);
  m_pElemOutTxt_Temp_Setpoint = pElemRef;
  
  // Create radio button E_ELEM_RADIO3
  pElemRef = gslc_ElemXCheckboxCreate(&m_gui,E_ELEM_RADIO3,E_PG_MAIN,&m_asXRadio3,
    (gslc_tsRect){295,144,6,6},true,GSLCX_CHECKBOX_STYLE_ROUND,GSLC_COL_GREEN,false);
  gslc_ElemSetCol(&m_gui,pElemRef,((gslc_tsColor){135,211,255}),((gslc_tsColor){0,75,153}),((gslc_tsColor){0,75,153}));
  m_pElemRB_sample2_3 = pElemRef;
  
  // Create E_ELEM_TEXT22 text label
  pElemRef = gslc_ElemCreateTxt(&m_gui,E_ELEM_TEXT22,E_PG_MAIN,(gslc_tsRect){137,186,23,28},
    (char*)"C",0,E_FREESANS18);
  gslc_ElemSetCol(&m_gui,pElemRef,GSLC_COL_GRAY,((gslc_tsColor){0,75,153}),GSLC_COL_BLACK);
  gslc_ElemSetTxtEnc(&m_gui,pElemRef,GSLC_TXT_ENC_UTF8);
  
  // Create radio button E_ELEM_RADIO1
  pElemRef = gslc_ElemXCheckboxCreate(&m_gui,E_ELEM_RADIO1,E_PG_MAIN,&m_asXRadio1,
    (gslc_tsRect){9,109,12,12},true,GSLCX_CHECKBOX_STYLE_ROUND,GSLC_COL_GREEN,false);
  gslc_ElemSetCol(&m_gui,pElemRef,((gslc_tsColor){0,75,153}),((gslc_tsColor){0,75,153}),((gslc_tsColor){0,75,153}));
  m_pElemRB_sample = pElemRef;
   
  // Create E_ELEM_BOX6 box
  pElemRef = gslc_ElemCreateBox(&m_gui,E_ELEM_BOX6,E_PG_MAIN,(gslc_tsRect){4,49,170,50});
  gslc_ElemSetRoundEn(&m_gui, pElemRef, true);
  gslc_ElemSetCol(&m_gui,pElemRef,((gslc_tsColor){0,95,218}),((gslc_tsColor){0,75,153}),((gslc_tsColor){0,75,153}));
  
  // Create E_ELEM_TEXT18 text label
  pElemRef = gslc_ElemCreateTxt(&m_gui,E_ELEM_TEXT18,E_PG_MAIN,(gslc_tsRect){132,9,10,15},
    (char*)"T",0,E_FREESANS9);
  gslc_ElemSetCol(&m_gui,pElemRef,GSLC_COL_GRAY,((gslc_tsColor){0,65,128}),GSLC_COL_BLACK);
  gslc_ElemSetTxtCol(&m_gui,pElemRef,((gslc_tsColor){0,136,255}));
  
  // Create E_ELEM_TEXT_TEMP_OUTSIDE runtime modifiable text
  static char m_sDisplayText19[8] = "--.-";
  pElemRef = gslc_ElemCreateTxt(&m_gui,E_ELEM_TEXT_TEMP_OUTSIDE,E_PG_MAIN,(gslc_tsRect){146,9,40,15},
    (char*)m_sDisplayText19,8,E_FREESANS9);
  gslc_ElemSetCol(&m_gui,pElemRef,GSLC_COL_GRAY,((gslc_tsColor){0,65,128}),GSLC_COL_BLACK);
  gslc_ElemSetTxtCol(&m_gui,pElemRef,((gslc_tsColor){135,211,255}));
  m_pElemOutTxt_Temp_Outside = pElemRef;
  
  // Create E_ELEM_TEXT23 text label
  pElemRef = gslc_ElemCreateTxt(&m_gui,E_ELEM_TEXT23,E_PG_MAIN,(gslc_tsRect){190,9,12,15},
    (char*)"C",0,E_FREESANS9);
  gslc_ElemSetTxtCol(&m_gui,pElemRef,((gslc_tsColor){0,136,255}));
  gslc_ElemSetCol(&m_gui,pElemRef,GSLC_COL_GRAY,((gslc_tsColor){0,65,128}),GSLC_COL_BLACK);
  gslc_ElemSetTxtEnc(&m_gui,pElemRef,GSLC_TXT_ENC_UTF8);
  
  // Create radio button E_ELEM_RADIO2
  pElemRef = gslc_ElemXCheckboxCreate(&m_gui,E_ELEM_RADIO2,E_PG_MAIN,&m_asXRadio2,
    (gslc_tsRect){185,7,6,6},true,GSLCX_CHECKBOX_STYLE_ROUND,GSLC_COL_GREEN,false);
  gslc_ElemSetCol(&m_gui,pElemRef,((gslc_tsColor){0,136,255}),((gslc_tsColor){0,65,128}),((gslc_tsColor){0,75,153}));
  m_pElemRB_sample2 = pElemRef;
  
  // Create E_ELEM_TEXT30 runtime modifiable text
  static char m_sDisplayText30[20] = "";
  pElemRef = gslc_ElemCreateTxt(&m_gui,E_ELEM_TEXT30,E_PG_MAIN,(gslc_tsRect){9,9,120,15},
    (char*)m_sDisplayText30,20,E_FREESANS9);
  gslc_ElemSetTxtCol(&m_gui,pElemRef,((gslc_tsColor){0,136,255}));
  gslc_ElemSetCol(&m_gui,pElemRef,GSLC_COL_GRAY,((gslc_tsColor){0,65,128}),GSLC_COL_BLACK);
  gslc_ElemSetTxtEnc(&m_gui,pElemRef,GSLC_TXT_ENC_UTF8);
  m_pElemOutTxt_Date = pElemRef;
  
  // Create E_ELEM_TEXT31 runtime modifiable text label (SETPOINT / CRONO F1..F4)
  static char m_sDisplayText31[16] = "SETPOINT";
  pElemRef = gslc_ElemCreateTxt(&m_gui,E_ELEM_TEXT31,E_PG_MAIN,(gslc_tsRect){210,106,80,10},
    (char*)m_sDisplayText31,16,E_BUILTIN5X8);
  gslc_ElemSetTxtAlign(&m_gui,pElemRef,GSLC_ALIGN_MID_MID);
  gslc_ElemSetTxtCol(&m_gui,pElemRef,((gslc_tsColor){0,136,255}));
  gslc_ElemSetCol(&m_gui,pElemRef,GSLC_COL_GRAY,((gslc_tsColor){0,75,153}),((gslc_tsColor){0,75,153}));
  gslc_ElemSetTxtEnc(&m_gui,pElemRef,GSLC_TXT_ENC_UTF8);
  m_pElemOutTxt_SetpointLbl = pElemRef;
  
  // Create E_ELEM_TEXT32 text label
  static char m_sDisplayText32[12] = "INDOOR";
  pElemRef = gslc_ElemCreateTxt(&m_gui,E_ELEM_TEXT32,E_PG_MAIN,(gslc_tsRect){65,105,48,10},
    (char*)m_sDisplayText32,12,E_BUILTIN5X8);
  gslc_ElemSetTxtAlign(&m_gui,pElemRef,GSLC_ALIGN_MID_MID);
  gslc_ElemSetTxtCol(&m_gui,pElemRef,((gslc_tsColor){0,136,255}));
  gslc_ElemSetCol(&m_gui,pElemRef,GSLC_COL_GRAY,((gslc_tsColor){0,75,153}),((gslc_tsColor){0,75,153}));
  gslc_ElemSetTxtEnc(&m_gui,pElemRef,GSLC_TXT_ENC_UTF8);
  m_pElemOutTxt_IndoorLbl = pElemRef;
  
  // Create radio button E_ELEM_RADIO4
  pElemRef = gslc_ElemXCheckboxCreate(&m_gui,E_ELEM_RADIO4,E_PG_MAIN,&m_asXRadio4,
    (gslc_tsRect){135,181,6,6},true,GSLCX_CHECKBOX_STYLE_ROUND,GSLC_COL_GREEN,false);
  gslc_ElemSetCol(&m_gui,pElemRef,GSLC_COL_YELLOW,((gslc_tsColor){0,75,153}),((gslc_tsColor){0,75,153}));
  m_pElemRB_sample2_4 = pElemRef;
  
  // Create E_ELEM_TEXT33 runtime modifiable text
  static char m_sDisplayText33[6] = "";
  pElemRef = gslc_ElemCreateTxt(&m_gui,E_ELEM_TEXT33,E_PG_MAIN,(gslc_tsRect){260,8,50,15},
    (char*)m_sDisplayText33,6,E_FREESANS9);
  gslc_ElemSetTxtCol(&m_gui,pElemRef,((gslc_tsColor){0,136,255}));
  gslc_ElemSetCol(&m_gui,pElemRef,GSLC_COL_GRAY,((gslc_tsColor){0,65,128}),GSLC_COL_BLACK);
  gslc_ElemSetTxtEnc(&m_gui,pElemRef,GSLC_TXT_ENC_UTF8);
  m_pElemOutTxt_Time = pElemRef;
  
  // Create E_ELEM_TEXT44 text label
  pElemRef = gslc_ElemCreateTxt(&m_gui,E_ELEM_TEXT44,E_PG_MAIN,(gslc_tsRect){119,50,37,10},
    (char*)"STATUS",0,E_BUILTIN5X8);
  gslc_ElemSetTxtCol(&m_gui,pElemRef,((gslc_tsColor){0,136,255}));
  gslc_ElemSetCol(&m_gui,pElemRef,GSLC_COL_GRAY,((gslc_tsColor){0,75,153}),((gslc_tsColor){0,75,153}));
  gslc_ElemSetTxtEnc(&m_gui,pElemRef,GSLC_TXT_ENC_UTF8);
  
  // Create E_ELEM_TEXT_status runtime modifiable text
  static char m_sDisplayText45[12] = "";
  pElemRef = gslc_ElemCreateTxt(&m_gui,E_ELEM_TEXT_status,E_PG_MAIN,(gslc_tsRect){6,70,77,20},
    (char*)m_sDisplayText45,12,E_FREESANS12);
  gslc_ElemSetTxtAlign(&m_gui,pElemRef,GSLC_ALIGN_MID_RIGHT);
  gslc_ElemSetTxtCol(&m_gui,pElemRef,GSLC_COL_WHITE);
  gslc_ElemSetCol(&m_gui,pElemRef,GSLC_COL_GRAY,((gslc_tsColor){0,75,153}),GSLC_COL_BLACK);
  gslc_ElemSetFillEn(&m_gui,pElemRef,true);
  m_pElemOutTxt_status = pElemRef;
  
  // Create E_ELEM_TEXT_STAT runtime modifiable text
  static char m_sDisplayText46[10] = "";
  pElemRef = gslc_ElemCreateTxt(&m_gui,E_ELEM_TEXT_STAT,E_PG_MAIN,(gslc_tsRect){102,70,64,20},
    (char*)m_sDisplayText46,10,E_FREESANS12);
  gslc_ElemSetTxtAlign(&m_gui,pElemRef,GSLC_ALIGN_MID_MID);
  gslc_ElemSetTxtCol(&m_gui,pElemRef,GSLC_COL_WHITE);
  gslc_ElemSetCol(&m_gui,pElemRef,GSLC_COL_GRAY,((gslc_tsColor){0,75,153}),GSLC_COL_BLACK);
  gslc_ElemSetFillEn(&m_gui,pElemRef,true);
  m_pElemOutTxt_stat = pElemRef;
  
  // Create radio button E_ELEM_RADIO5
  pElemRef = gslc_ElemXCheckboxCreate(&m_gui,E_ELEM_RADIO5,E_PG_MAIN,&m_asXRadio5,
    (gslc_tsRect){222,4,20,20},true,GSLCX_CHECKBOX_STYLE_ROUND,GSLC_COL_RED,false);
  gslc_ElemSetCol(&m_gui,pElemRef,GSLC_COL_RED_DK2,((gslc_tsColor){0,65,128}),GSLC_COL_BLACK);
  m_pElemRB5 = pElemRef;
  
  // Create E_ELEM_TEXT16 runtime modifiable text
  static char m_sDisplayText47[17] = "";
  pElemRef = gslc_ElemCreateTxt(&m_gui,E_ELEM_TEXT16,E_PG_MAIN,(gslc_tsRect){221,28,97,11},
    (char*)m_sDisplayText47,17,E_BUILTIN5X8);
  gslc_ElemSetCol(&m_gui,pElemRef,GSLC_COL_GRAY,((gslc_tsColor){0,65,128}),GSLC_COL_BLACK);
  m_pElemOutTxt_Ip_Address16 = pElemRef;
  
  // Create E_ELEM_TEXT48 text label
  pElemRef = gslc_ElemCreateTxt(&m_gui,E_ELEM_TEXT48,E_PG_MAIN,(gslc_tsRect){36,50,25,10},
    (char*)"MODE",0,E_BUILTIN5X8);
  gslc_ElemSetTxtCol(&m_gui,pElemRef,((gslc_tsColor){0,136,255}));
  gslc_ElemSetCol(&m_gui,pElemRef,GSLC_COL_GRAY,((gslc_tsColor){0,75,153}),((gslc_tsColor){0,75,153}));
  gslc_ElemSetTxtEnc(&m_gui,pElemRef,GSLC_TXT_ENC_UTF8);
  
  // Create E_ELEM_TEXT50 text label
  pElemRef = gslc_ElemCreateTxt(&m_gui,E_ELEM_TEXT50,E_PG_MAIN,(gslc_tsRect){207,28,13,11},
    (char*)"IP",0,E_BUILTIN5X8);
  gslc_ElemSetTxtAlign(&m_gui,pElemRef,GSLC_ALIGN_MID_RIGHT);
  gslc_ElemSetCol(&m_gui,pElemRef,GSLC_COL_GRAY,((gslc_tsColor){0,65,128}),((gslc_tsColor){0,65,128}));
  
  // create E_ELEM_BTN_MODE button with text label
  pElemRef = gslc_ElemCreateBtnTxt(&m_gui,E_ELEM_BTN_MODE,E_PG_MAIN,
    (gslc_tsRect){3,48,95,50},(char*)" ",0,E_BUILTIN5X8,&CbBtnCommon);
  gslc_ElemSetFillEn(&m_gui,pElemRef,false);
  gslc_ElemSetFrameEn(&m_gui,pElemRef,false);
  m_pElemBtn_mode = pElemRef;
  
  // create E_ELEM_BTN_MENU button with text label
  pElemRef = gslc_ElemCreateBtnTxt(&m_gui,E_ELEM_BTN_MENU,E_PG_MAIN,
    (gslc_tsRect){278,4,40,30},(char*)" ",0,E_BUILTIN5X8,&CbBtnCommon);
  gslc_ElemSetFillEn(&m_gui,pElemRef,false);
  gslc_ElemSetFrameEn(&m_gui,pElemRef,false);
  m_pElemBtn_menu = pElemRef;
  
  // Create E_ELEM_TEXT_WEATHER runtime modifiable text (Pressione + Meteo + Comfort)
  static char m_sDisplayTextWeather[28] = "--- hPa";
  pElemRef = gslc_ElemCreateTxt(&m_gui,E_ELEM_TEXT_WEATHER,E_PG_MAIN,(gslc_tsRect){8,156,162,12},
    (char*)m_sDisplayTextWeather,28,E_BUILTIN5X8);
  gslc_ElemSetTxtAlign(&m_gui,pElemRef,GSLC_ALIGN_MID_MID);
  gslc_ElemSetTxtCol(&m_gui,pElemRef,((gslc_tsColor){135,255,180}));
  gslc_ElemSetCol(&m_gui,pElemRef,GSLC_COL_GRAY,((gslc_tsColor){0,75,153}),GSLC_COL_BLACK);
  m_pElemOutTxt_Weather = pElemRef;

  // Create E_ELEM_BOX_STATUS_BAR box for 8 indicator badges (CRN, GEL, WIN, PMP, NIT, IST, BUZ, MQT)
  pElemRef = gslc_ElemCreateBox(&m_gui,E_ELEM_BOX_STATUS_BAR,E_PG_MAIN,(gslc_tsRect){5,27,200,13});
  gslc_ElemSetFillEn(&m_gui,pElemRef,false);
  gslc_ElemSetFrameEn(&m_gui,pElemRef,false);
  gslc_ElemSetDrawFunc(&m_gui,pElemRef,&CbDrawStatusBar);
  m_pElemBox_StatusBar = pElemRef;

  // Create E_ELEM_BOX_WIFI_BARS box for WiFi signal bars
  pElemRef = gslc_ElemCreateBox(&m_gui,E_ELEM_BOX_WIFI_BARS,E_PG_MAIN,(gslc_tsRect){243,6,18,13});
  gslc_ElemSetFillEn(&m_gui,pElemRef,false);
  gslc_ElemSetFrameEn(&m_gui,pElemRef,false);
  gslc_ElemSetDrawFunc(&m_gui,pElemRef,&CbDrawWifiBars);
  m_pElemBox_WifiBars = pElemRef;

  // create E_ELEM_BTN_WAKE_UP button with text label
  pElemRef = gslc_ElemCreateBtnTxt(&m_gui,E_ELEM_BTN_WAKE_UP,E_PG_MAIN,
    (gslc_tsRect){10,5,300,230},(char*)" ",0,E_BUILTIN5X8,&CbBtnCommon);
  gslc_ElemSetFillEn(&m_gui,pElemRef,false);
  gslc_ElemSetFrameEn(&m_gui,pElemRef,false);
  m_pElemBtn_wake_up = pElemRef;

  // -----------------------------------
  // PAGE: E_PG2
  
  
  // create E_ELEM_BTN_MAIN_PAGE button with text label
  pElemRef = gslc_ElemCreateBtnTxt(&m_gui,E_ELEM_BTN_MAIN_PAGE,E_PG2,
    (gslc_tsRect){248,168,62,65},(char*)"MAIN",0,E_FREESANS9,&CbBtnCommon);
  gslc_ElemSetCol(&m_gui,pElemRef,((gslc_tsColor){0,95,218}),((gslc_tsColor){0,75,153}),((gslc_tsColor){0,136,255}));
  gslc_ElemSetRoundEn(&m_gui, pElemRef, true);
  btn_main_page = pElemRef;
   
  // Create E_ELEM_BOX8 box
  pElemRef = gslc_ElemCreateBox(&m_gui,E_ELEM_BOX8,E_PG2,(gslc_tsRect){10,3,300,100});
  gslc_ElemSetRoundEn(&m_gui, pElemRef, true);
  gslc_ElemSetCol(&m_gui,pElemRef,GSLC_COL_GRAY,((gslc_tsColor){0,65,128}),((gslc_tsColor){0,65,128}));

  // Create graph E_ELEM_GRAPH_TEMPERATURE
  pElemRef = gslc_ElemXGraphCreate(&m_gui,E_ELEM_GRAPH_TEMPERATURE,E_PG2,
    &m_sGraph1,(gslc_tsRect){10,117,220,120},E_BUILTIN5X8,(int16_t*)&m_anGraphBuf1,
        220,((gslc_tsColor){140,255,150}));
  gslc_ElemXGraphSetStyle(&m_gui,pElemRef, GSLCX_GRAPH_STYLE_DOT, 5);
  gslc_ElemSetCol(&m_gui,pElemRef,GSLC_COL_GRAY,((gslc_tsColor){0,75,153}),GSLC_COL_BLACK);
  m_pElemGraph_Temperature = pElemRef;
  
  // Create E_ELEM_TEXT34 text label
  static char m_sDisplayText34[36] = "Grafico Temperatura";
  pElemRef = gslc_ElemCreateTxt(&m_gui,E_ELEM_TEXT34,E_PG2,(gslc_tsRect){10,103,220,12},
    (char*)m_sDisplayText34,36,E_BUILTIN5X8);
  gslc_ElemSetCol(&m_gui,pElemRef,GSLC_COL_GRAY,((gslc_tsColor){0,65,128}),GSLC_COL_BLACK);
  m_pElemOutTxt_GraphInfo = pElemRef;
  
  // Create E_ELEM_TEXT41 text label
  pElemRef = gslc_ElemCreateTxt(&m_gui,E_ELEM_TEXT41,E_PG2,(gslc_tsRect){18,13,115,10},
    (char*)"Temperature Offset:",0,E_BUILTIN5X8);
  gslc_ElemSetCol(&m_gui,pElemRef,GSLC_COL_GRAY,((gslc_tsColor){0,65,128}),GSLC_COL_BLACK);
  
  // create E_ELEM_BTN10 button with text label
  pElemRef = gslc_ElemCreateBtnTxt(&m_gui,E_ELEM_BTN10,E_PG2,
    (gslc_tsRect){248,118,62,40},(char*)"RESET",0,E_FREESANS9,&CbBtnCommon);
  gslc_ElemSetCol(&m_gui,pElemRef,((gslc_tsColor){0,95,218}),((gslc_tsColor){0,75,153}),((gslc_tsColor){0,136,255}));
  gslc_ElemSetRoundEn(&m_gui, pElemRef, true);
  btn_reset = pElemRef;

  // Create slider E_ELEM_SLIDER_TEMP_OFFSET 
  pElemRef = gslc_ElemXSliderCreate(&m_gui,E_ELEM_SLIDER_TEMP_OFFSET,E_PG2,&m_sXSlider1,
          (gslc_tsRect){14,49,290,45},0,100,50,15,false);
  gslc_ElemXSliderSetStyle(&m_gui,pElemRef,true,((gslc_tsColor){0,131,252}),20,5,((gslc_tsColor){0,131,252}));
  gslc_ElemXSliderSetPosFunc(&m_gui,pElemRef,&CbSlidePos);
  gslc_ElemSetCol(&m_gui,pElemRef,GSLC_COL_GRAY,((gslc_tsColor){0,65,128}),((gslc_tsColor){0,65,128}));
  m_pElemSlider_temp_offset = pElemRef;
  
  // Create E_ELEM_TEXT_TEMP_OFFSET runtime modifiable text
  static char m_sDisplayText51[6] = "";
  pElemRef = gslc_ElemCreateTxt(&m_gui,E_ELEM_TEXT_TEMP_OFFSET,E_PG2,(gslc_tsRect){151,8,61,18},
    (char*)m_sDisplayText51,6,E_BUILTIN10X16);
  gslc_ElemSetTxtCol(&m_gui,pElemRef,((gslc_tsColor){0,136,255}));
  gslc_ElemSetCol(&m_gui,pElemRef,GSLC_COL_GRAY,((gslc_tsColor){0,65,128}),GSLC_COL_BLACK);
  m_pElemOutTxt_temp_offset = pElemRef;
  
  // Create E_ELEM_BTN_SAVE button with modifiable text label
  static char m_strbtn14[5] = "SAVE";
  pElemRef = gslc_ElemCreateBtnTxt(&m_gui,E_ELEM_BTN_SAVE,E_PG2,
    (gslc_tsRect){232,7,70,35},
    (char*)m_strbtn14,5,E_FREESANS12,&CbBtnCommon);
  gslc_ElemSetCol(&m_gui,pElemRef,((gslc_tsColor){0,95,218}),((gslc_tsColor){0,75,153}),((gslc_tsColor){0,136,255}));
  gslc_ElemSetRoundEn(&m_gui, pElemRef, true);
  m_pElemBtn_save = pElemRef;
  
  // create E_ELEM_BTN_WAKE_UP2 button with text label
  pElemRef = gslc_ElemCreateBtnTxt(&m_gui,E_ELEM_BTN_WAKE_UP2,E_PG2,
    (gslc_tsRect){9,5,300,230},(char*)" ",0,E_BUILTIN5X8,&CbBtnCommon);
  gslc_ElemSetFillEn(&m_gui,pElemRef,false);
  gslc_ElemSetFrameEn(&m_gui,pElemRef,false);
  m_pElemBtn_wake_up2 = pElemRef;
//<InitGUI !End!>

//<Startup !Start!>
  gslc_GuiRotate(&m_gui, 1);
//<Startup !End!>

}

#endif // end _GUISLICE_GEN_H
