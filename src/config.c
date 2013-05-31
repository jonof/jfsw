//-------------------------------------------------------------------------
/*
Copyright (C) 1997, 2005 - 3D Realms Entertainment

This file is part of Shadow Warrior version 1.2

Shadow Warrior is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

Original Source: 1997 - Frank Maddin and Jim Norwood
Prepared for public release: 03/28/2005 - Charlie Wiederhold, 3D Realms
*/
//-------------------------------------------------------------------------
#include "build.h"
#include "baselayer.h"
#include "osd.h"

#include "settings.h"
#include "mytypes.h"
#include "develop.h"
#include "scriplib.h"
#include "file_lib.h"
#include "gamedefs.h"
#include "keyboard.h"
#include "util_lib.h"
#include "function.h"
#include "control.h"
#include "fx_man.h"
#include "sounds.h"
#include "config.h"
#include "common.h"

// we load this in to get default button and key assignments
// as well as setting up function mappings

#include "_functio.h"
#include "_config.h"

extern void ReadGameSetup(int32 scripthandle);
extern void WriteGameSetup(int32 scripthandle);

//
// Comm variables
//

char CommPlayerName[32];
int32 NumberPlayers,CommPort,PortSpeed,IrqNumber,UartAddress;

//
// Sound variables
//
int32 FXDevice    = 0;
int32 MusicDevice = 0;
int32 NumVoices   = 32;
int32 NumChannels = 2;
int32 NumBits     = 16;
int32 MixRate     = 44100;

int32 UseMouse = 1, UseJoystick = 0;

byte KeyboardKeys[NUMGAMEFUNCTIONS][2];
int32 MouseFunctions[MAXMOUSEBUTTONS][2];
int32 MouseDigitalFunctions[MAXMOUSEAXES][2];
int32 MouseAnalogueAxes[MAXMOUSEAXES];
int32 MouseAnalogueScale[MAXMOUSEAXES];
int32 JoystickFunctions[MAXJOYBUTTONS][2];
int32 JoystickDigitalFunctions[MAXJOYAXES][2];
int32 JoystickAnalogueAxes[MAXJOYAXES];
int32 JoystickAnalogueScale[MAXJOYAXES];
int32 JoystickAnalogueDead[MAXJOYAXES];
int32 JoystickAnalogueSaturate[MAXJOYAXES];

//
// Screen variables
//

int32 ScreenMode = 1;
int32 ScreenWidth = 640;
int32 ScreenHeight = 480;
int32 ScreenBPP = 8;
int32 ForceSetup = 1;

extern char WangBangMacro[10][64];
char  RTSName[MAXRTSNAMELENGTH];
//static char setupfilename[64]={SETUPFILENAME};
char setupfilename[64]={SETUPFILENAME};
static int32 scripthandle = -1;

char  MouseAnalogAxes[MAXMOUSEAXES][MAXFUNCTIONLENGTH];
char  JoystickAnalogAxes[MAXJOYAXES][MAXFUNCTIONLENGTH];

char  MouseDigitalAxes[MAXMOUSEAXES][2][MAXFUNCTIONLENGTH];
char  GamePadDigitalAxes[MAXGAMEPADAXES][2][MAXFUNCTIONLENGTH];
char  JoystickDigitalAxes[MAXJOYAXES][2][MAXFUNCTIONLENGTH];


/*
===================
=
= CONFIG_FunctionNameToNum
=
===================
*/

int32 CONFIG_FunctionNameToNum( const char * func )
   {
   int32 i;

   if (!func) return -1;
   for (i=0;i<NUMGAMEFUNCTIONS;i++)
      {
      if (!Bstrcasecmp(func,gamefunctions[i]))
         {
         return i;
         }
      }
   return -1;
   }

/*
===================
=
= CONFIG_FunctionNumToName
=
===================
*/

const char * CONFIG_FunctionNumToName( int32 func )
   {
   if ((unsigned)func >= (unsigned)NUMGAMEFUNCTIONS)
      {
      return NULL;
      }
   else
      {
      return gamefunctions[func];
      }
   }

/*
===================
=
= CONFIG_AnalogNameToNum
=
===================
*/
int32 CONFIG_AnalogNameToNum( const char * func )
   {
   if (!Bstrcasecmp(func,"analog_turning"))
      {
      return analog_turning;
      }
   if (!Bstrcasecmp(func,"analog_strafing"))
      {
      return analog_strafing;
      }
   if (!Bstrcasecmp(func,"analog_moving"))
      {
      return analog_moving;
      }
   if (!Bstrcasecmp(func,"analog_lookingupanddown"))
      {
      return analog_lookingupanddown;
      }
   return -1;
   }

const char * CONFIG_AnalogNumToName( int32 func )
   {
   switch (func) {
	case analog_turning:
		return "analog_turning";
	case analog_strafing:
		return "analog_strafing";
	case analog_moving:
   		return "analog_moving";
	case analog_lookingupanddown:
		return "analog_lookingupanddown";
	default: break;
   }

   return NULL;
   }

/*
===================
=
= CONFIG_SetDefaults
=
===================
*/

void CONFIG_SetDefaults( void )
   {
   // JBF 20031211
   int32 i,f;
   byte k1,k2;

   ScreenMode = 1;
   ScreenWidth = 640;
   ScreenHeight = 480;
   ScreenBPP = 8;
   FXDevice = 0;
   MusicDevice = 0;
   NumVoices = 32;
   NumChannels = 2;
   NumBits = 16;
   MixRate = 44100;
   memcpy(&gs, &gs_defaults, sizeof(gs));

   Bstrcpy(RTSName, DEFAULTRTSFILE);
   Bstrcpy(CommPlayerName, DEFAULTPLAYERNAME);

   Bstrcpy(WangBangMacro[0], MACRO1);
   Bstrcpy(WangBangMacro[1], MACRO2);
   Bstrcpy(WangBangMacro[2], MACRO3);
   Bstrcpy(WangBangMacro[3], MACRO4);
   Bstrcpy(WangBangMacro[4], MACRO5);
   Bstrcpy(WangBangMacro[5], MACRO6);
   Bstrcpy(WangBangMacro[6], MACRO7);
   Bstrcpy(WangBangMacro[7], MACRO8);
   Bstrcpy(WangBangMacro[8], MACRO9);
   Bstrcpy(WangBangMacro[9], MACRO10);

   SetDefaultKeyDefinitions(0);
   SetMouseDefaults(0);

   memset(MouseDigitalFunctions, -1, sizeof(MouseDigitalFunctions));
   for (i=0; i<MAXMOUSEAXES; i++) {
      MouseAnalogueScale[i] = 65536;

      MouseDigitalFunctions[i][0] = CONFIG_FunctionNameToNum( mousedigitaldefaults[i*2] );
      MouseDigitalFunctions[i][1] = CONFIG_FunctionNameToNum( mousedigitaldefaults[i*2+1] );

      MouseAnalogueAxes[i] = CONFIG_AnalogNameToNum( mouseanalogdefaults[i] );
   }
   CONTROL_SetMouseSensitivity(gs.MouseSpeed<<2);

   memset(JoystickFunctions, -1, sizeof(JoystickFunctions));
   for (i=0; i < (int32)(sizeof(joystickdefaults)/sizeof(char*)); i++) {
      JoystickFunctions[i][0] = CONFIG_FunctionNameToNum( joystickdefaults[i] );
      JoystickFunctions[i][1] = CONFIG_FunctionNameToNum( joystickclickeddefaults[i] );
   }

   memset(JoystickDigitalFunctions, -1, sizeof(JoystickDigitalFunctions));
   for (i=0; i < (int32)(sizeof(joystickanalogdefaults)/sizeof(char*)); i++) {
	JoystickAnalogueScale[i] = 65536;
	JoystickAnalogueDead[i] = 1000;
	JoystickAnalogueSaturate[i] = 9500;

	JoystickDigitalFunctions[i][0] = CONFIG_FunctionNameToNum( joystickdigitaldefaults[i*2] );
	JoystickDigitalFunctions[i][1] = CONFIG_FunctionNameToNum( joystickdigitaldefaults[i*2+1] );

	JoystickAnalogueAxes[i] = CONFIG_AnalogNameToNum( joystickanalogdefaults[i] );
   }
}


void SetDefaultKeyDefinitions(int style)
{
   int numkeydefaults;
   char **keydefaultset;
   int i, f, k1, k2;

   if (style) {
      numkeydefaults = sizeof(keydefaults_modern) / sizeof(char *) / 3;
      keydefaultset = keydefaults_modern;
   } else {
      numkeydefaults = sizeof(keydefaults) / sizeof(char *) / 3;
      keydefaultset = keydefaults;
   }

   memset(KeyboardKeys, 0xff, sizeof(KeyboardKeys));
   for (i=0; i < numkeydefaults; i++) {
      f = CONFIG_FunctionNameToNum( keydefaultset[3*i+0] );
      if (f == -1) continue;
      k1 = KB_StringToScanCode( keydefaultset[3*i+1] );
      k2 = KB_StringToScanCode( keydefaultset[3*i+2] );

      CONTROL_MapKey( i, k1, k2 );

      KeyboardKeys[f][0] = k1;
      KeyboardKeys[f][1] = k2;
   }
}

void SetMouseDefaults(int style)
{
   int nummousedefaults;
   char **mousedefaultset, **mouseclickeddefaultset;
   int i;

   if (style) {
      nummousedefaults = sizeof(mousedefaults_modern) / sizeof(char *);
      mousedefaultset = mousedefaults_modern;
      mouseclickeddefaultset = mouseclickeddefaults_modern;
   } else {
      nummousedefaults = sizeof(mousedefaults) / sizeof(char *);
      mousedefaultset = mousedefaults;
      mouseclickeddefaultset = mouseclickeddefaults;
   }

   memset(MouseFunctions, -1, sizeof(MouseFunctions));
   for (i=0; i < nummousedefaults; i++) {
      MouseFunctions[i][0] = CONFIG_FunctionNameToNum( mousedefaultset[i] );
      CONTROL_MapButton( MouseFunctions[i][0], i, FALSE, controldevice_mouse );

      if (i<4) continue;

      MouseFunctions[i][1] = CONFIG_FunctionNameToNum( mouseclickeddefaultset[i] );
      CONTROL_MapButton( MouseFunctions[i][1], i, TRUE,  controldevice_mouse );
   }
}

/*
===================
=
= CONFIG_ReadKeys
=
===================
*/

void CONFIG_ReadKeys( int32 scripthandle )
   {
   int32 i;
   int32 numkeyentries;
   int32 function;
   char keyname1[80];
   char keyname2[80];
   kb_scancode key1,key2;

   if (scripthandle < 0) return;

   numkeyentries = SCRIPT_NumberEntries( scripthandle,"KeyDefinitions" );

   for (i=0;i<numkeyentries;i++)
      {
      function = CONFIG_FunctionNameToNum(SCRIPT_Entry(scripthandle,"KeyDefinitions", i ));
      if (function != -1)
         {
         memset(keyname1,0,sizeof(keyname1));
         memset(keyname2,0,sizeof(keyname2));
         SCRIPT_GetDoubleString
            (
            scripthandle,
            "KeyDefinitions",
            SCRIPT_Entry( scripthandle, "KeyDefinitions", i ),
            keyname1,
            keyname2
            );
         key1 = 0xff;
         key2 = 0xff;
         if (keyname1[0])
            {
            key1 = (byte) KB_StringToScanCode( keyname1 );
            }
         if (keyname2[0])
            {
            key2 = (byte) KB_StringToScanCode( keyname2 );
            }
         KeyboardKeys[function][0] = key1;
         KeyboardKeys[function][1] = key2;
         }
      }

   for (i=0; i<NUMGAMEFUNCTIONS; i++)
      {
         if (i == gamefunc_Show_Console)
            OSD_CaptureKey(KeyboardKeys[i][0]);
         else
            CONTROL_MapKey( i, KeyboardKeys[i][0], KeyboardKeys[i][1] );
      }
   }

/*
===================
=
= CONFIG_SetupMouse
=
===================
*/

void CONFIG_SetupMouse( void )
   {
   int32 i;
   char str[80],*p;
   char temp[80];
   int32 function, scale;

   if (scripthandle < 0) return;

   for (i=0;i<MAXMOUSEBUTTONS;i++)
      {
      Bsprintf(str,"MouseButton%d",i); temp[0] = 0;
      if (!SCRIPT_GetString( scripthandle,"Controls", str,temp))
         MouseFunctions[i][0] = CONFIG_FunctionNameToNum(temp);
	  
      Bsprintf(str,"MouseButtonClicked%d",i); temp[0] = 0;
      if (!SCRIPT_GetString( scripthandle,"Controls", str,temp))
         MouseFunctions[i][1] = CONFIG_FunctionNameToNum(temp);
      }

   // map over the axes
   for (i=0;i<MAXMOUSEAXES;i++)
      {
      Bsprintf(str,"MouseAnalogAxes%d",i); temp[0] = 0;
      if (!SCRIPT_GetString(scripthandle, "Controls", str,temp))
         MouseAnalogueAxes[i] = CONFIG_AnalogNameToNum(temp);

      Bsprintf(str,"MouseDigitalAxes%d_0",i); temp[0] = 0;
      if (!SCRIPT_GetString(scripthandle, "Controls", str,temp))
         MouseDigitalFunctions[i][0] = CONFIG_FunctionNameToNum(temp);

      Bsprintf(str,"MouseDigitalAxes%d_1",i); temp[0] = 0;
      if (!SCRIPT_GetString(scripthandle, "Controls", str,temp))
         MouseDigitalFunctions[i][1] = CONFIG_FunctionNameToNum(temp);

      Bsprintf(str,"MouseAnalogScale%d",i);
      scale = MouseAnalogueScale[i];
      SCRIPT_GetNumber(scripthandle, "Controls", str,&scale);
      MouseAnalogueScale[i] = scale;
      }

   // 0 to 65536  
   SCRIPT_GetNumber( scripthandle, "Controls","MouseSensitivity",&function);
   gs.MouseSpeed = function;

   for (i=0; i<MAXMOUSEBUTTONS; i++)
      {
      CONTROL_MapButton( MouseFunctions[i][0], i, FALSE, controldevice_mouse );
      CONTROL_MapButton( MouseFunctions[i][1], i, TRUE,  controldevice_mouse );
      }
   for (i=0; i<MAXMOUSEAXES; i++)
      {
      CONTROL_MapAnalogAxis( i, MouseAnalogueAxes[i], controldevice_mouse);
      CONTROL_MapDigitalAxis( i, MouseDigitalFunctions[i][0], 0,controldevice_mouse );
      CONTROL_MapDigitalAxis( i, MouseDigitalFunctions[i][1], 1,controldevice_mouse );
      CONTROL_SetAnalogAxisScale( i, MouseAnalogueScale[i], controldevice_mouse );
      }
   
   CONTROL_SetMouseSensitivity(gs.MouseSpeed<<2);
   }

/*
===================
=
= CONFIG_SetupJoystick
=
===================
*/

void CONFIG_SetupJoystick( void )
   {
   int32 i;
   char str[80],*p;
   char temp[80];
   int32 function, scale;

   if (scripthandle < 0) return;

   for (i=0;i<MAXJOYBUTTONS;i++)
      {
      Bsprintf(str,"JoystickButton%d",i); temp[0] = 0;
      if (!SCRIPT_GetString( scripthandle,"Controls", str,temp))
         JoystickFunctions[i][0] = CONFIG_FunctionNameToNum(temp);
	  
      Bsprintf(str,"JoystickButtonClicked%d",i); temp[0] = 0;
      if (!SCRIPT_GetString( scripthandle,"Controls", str,temp))
         JoystickFunctions[i][1] = CONFIG_FunctionNameToNum(temp);
      }

   // map over the axes
   for (i=0;i<MAXJOYAXES;i++)
      {
      Bsprintf(str,"JoystickAnalogAxes%d",i); temp[0] = 0;
      if (!SCRIPT_GetString(scripthandle, "Controls", str,temp))
         JoystickAnalogueAxes[i] = CONFIG_AnalogNameToNum(temp);
	  
      Bsprintf(str,"JoystickDigitalAxes%d_0",i); temp[0] = 0;
      if (!SCRIPT_GetString(scripthandle, "Controls", str,temp))
         JoystickDigitalFunctions[i][0] = CONFIG_FunctionNameToNum(temp);
	  
      Bsprintf(str,"JoystickDigitalAxes%d_1",i); temp[0] = 0;
      if (!SCRIPT_GetString(scripthandle, "Controls", str,temp))
	  JoystickDigitalFunctions[i][1] = CONFIG_FunctionNameToNum(temp);
	  
      Bsprintf(str,"JoystickAnalogScale%d",i);
      scale = JoystickAnalogueScale[i];
      SCRIPT_GetNumber(scripthandle, "Controls", str,&scale);
      JoystickAnalogueScale[i] = scale;

      Bsprintf(str,"JoystickAnalogDead%d",i);
      scale = JoystickAnalogueDead[i];
      SCRIPT_GetNumber(scripthandle, "Controls", str,&scale);
      JoystickAnalogueDead[i] = scale;

      Bsprintf(str,"JoystickAnalogSaturate%d",i);
      scale = JoystickAnalogueSaturate[i];
      SCRIPT_GetNumber(scripthandle, "Controls", str,&scale);
      JoystickAnalogueSaturate[i] = scale;
      }

   for (i=0;i<MAXJOYBUTTONS;i++)
      {
         CONTROL_MapButton( JoystickFunctions[i][0], i, FALSE, controldevice_joystick );
         CONTROL_MapButton( JoystickFunctions[i][1], i, TRUE,  controldevice_joystick );
      }
   for (i=0;i<MAXJOYAXES;i++)
      {
         CONTROL_MapAnalogAxis(i, JoystickAnalogueAxes[i], controldevice_joystick);
         CONTROL_MapDigitalAxis( i, JoystickDigitalFunctions[i][0], 0, controldevice_joystick );
         CONTROL_MapDigitalAxis( i, JoystickDigitalFunctions[i][1], 1, controldevice_joystick );
         CONTROL_SetAnalogAxisScale( i, JoystickAnalogueScale[i], controldevice_joystick );
      }
   }

/*
===================
=
= CONFIG_ReadSetup
=
===================
*/

int32 CONFIG_ReadSetup( void )
   {
   int32 dummy;
   char ret;
   extern char ds[];
   extern char PlayerNameArg[32];
   
   char oggtrackname[MAXOGGTRACKLENGTH] = {0};

   CONTROL_ClearAssignments();
   CONFIG_SetDefaults();
   
   if (SafeFileExists(setupfilename))
      scripthandle = SCRIPT_Load(setupfilename);
   
   if (scripthandle < 0) return -1;

      SCRIPT_GetNumber( scripthandle, "Screen Setup", "ScreenMode",&ScreenMode);
      SCRIPT_GetNumber( scripthandle, "Screen Setup", "ScreenWidth",&ScreenWidth);
      SCRIPT_GetNumber( scripthandle, "Screen Setup", "ScreenHeight",&ScreenHeight);
      SCRIPT_GetNumber( scripthandle, "Screen Setup", "ScreenBPP", &ScreenBPP);
      if (ScreenBPP < 8) ScreenBPP = 8;

#ifdef RENDERTYPEWIN
      SCRIPT_GetNumber( scripthandle, "Screen Setup", "MaxRefreshFreq", (int32*)&maxrefreshfreq);
#endif
 
      SCRIPT_GetNumber( scripthandle, "Screen Setup", "GLTextureMode", &gltexfiltermode);
      SCRIPT_GetNumber( scripthandle, "Screen Setup", "GLAnisotropy", &glanisotropy);
      SCRIPT_GetNumber( scripthandle, "Screen Setup", "GLUseTextureCompr", &glusetexcompr);

      SCRIPT_GetNumber( scripthandle, "Sound Setup", "FXDevice",&FXDevice);
      SCRIPT_GetNumber( scripthandle, "Sound Setup", "MusicDevice",&MusicDevice);
      SCRIPT_GetNumber( scripthandle, "Sound Setup", "FXVolume",&dummy);
      gs.SoundVolume = dummy;
      SCRIPT_GetNumber( scripthandle, "Sound Setup", "MusicVolume",&dummy);
      gs.MusicVolume = dummy;
 
      SCRIPT_GetNumber( scripthandle, "Sound Setup", "NumVoices",&NumVoices);
      SCRIPT_GetNumber( scripthandle, "Sound Setup", "NumChannels",&NumChannels);
      SCRIPT_GetNumber( scripthandle, "Sound Setup", "NumBits",&NumBits);
      SCRIPT_GetNumber( scripthandle, "Sound Setup", "MixRate",&MixRate);
      SCRIPT_GetNumber( scripthandle, "Sound Setup", "ReverseStereo",&dummy);
      gs.FlipStereo = dummy;
      if (gs.FlipStereo) gs.FlipStereo = 1;
      
      SCRIPT_GetString( scripthandle, "Sound Setup", "OggTrackName", oggtrackname);
      if(oggtrackname[0] != '\0')
        memcpy(gs.OggTrackName, oggtrackname, MAXOGGTRACKLENGTH);

      SCRIPT_GetNumber( scripthandle, "Setup", "ForceSetup",&ForceSetup);
      SCRIPT_GetNumber( scripthandle, "Controls","UseMouse",&UseMouse);
      SCRIPT_GetNumber( scripthandle, "Controls","UseJoystick",&UseJoystick);
      SCRIPT_GetString( scripthandle, "Comm Setup", "RTSName",RTSName);
   
      SCRIPT_GetString( scripthandle, "Comm Setup","PlayerName",CommPlayerName);
    
   ReadGameSetup(scripthandle);
    
   CONFIG_ReadKeys(scripthandle);

   //CONFIG_SetupMouse(scripthandle);
   //CONFIG_SetupJoystick(scripthandle);
   
   if (PlayerNameArg[0] != '\0')
      {
      strcpy(CommPlayerName, PlayerNameArg);
      }
   return 0;
   }

/*
===================
=
= CONFIG_WriteSetup
=
===================
*/

void CONFIG_WriteSetup( void )
   {
   int32 dummy;
   char buf[80];

   if (scripthandle < 0)
	   scripthandle = SCRIPT_Init(setupfilename);
      
   SCRIPT_PutNumber( scripthandle, "Screen Setup", "ScreenWidth", ScreenWidth,FALSE,FALSE);
   SCRIPT_PutNumber( scripthandle, "Screen Setup", "ScreenHeight",ScreenHeight,FALSE,FALSE);
   SCRIPT_PutNumber( scripthandle, "Screen Setup", "ScreenMode",ScreenMode,FALSE,FALSE);
   SCRIPT_PutNumber( scripthandle, "Screen Setup", "ScreenBPP",ScreenBPP,FALSE,FALSE);
#ifdef RENDERTYPEWIN
   SCRIPT_PutNumber( scripthandle, "Screen Setup", "MaxRefreshFreq",maxrefreshfreq,FALSE,FALSE);
#endif
   SCRIPT_PutNumber( scripthandle, "Screen Setup", "GLTextureMode",gltexfiltermode,FALSE,FALSE);
   SCRIPT_PutNumber( scripthandle, "Screen Setup", "GLAnisotropy",glanisotropy,FALSE,FALSE);
   SCRIPT_PutNumber( scripthandle, "Screen Setup", "GLUseTextureCompr",glusetexcompr,FALSE,FALSE);
   
   SCRIPT_PutNumber( scripthandle, "Sound Setup", "FXDevice", FXDevice, FALSE, FALSE);
   SCRIPT_PutNumber( scripthandle, "Sound Setup", "MusicDevice", MusicDevice, FALSE, FALSE);
   SCRIPT_PutNumber( scripthandle, "Sound Setup", "NumVoices", NumVoices, FALSE, FALSE);
   SCRIPT_PutNumber( scripthandle, "Sound Setup", "NumChannels", NumChannels, FALSE, FALSE);
   SCRIPT_PutNumber( scripthandle, "Sound Setup", "NumBits", NumBits, FALSE, FALSE);
   SCRIPT_PutNumber( scripthandle, "Sound Setup", "MixRate", MixRate, FALSE, FALSE);
   SCRIPT_PutNumber( scripthandle, "Sound Setup", "FXVolume",gs.SoundVolume,FALSE,FALSE);
   SCRIPT_PutNumber( scripthandle, "Sound Setup", "MusicVolume",gs.MusicVolume,FALSE,FALSE);
   dummy = gs.FlipStereo;
   SCRIPT_PutNumber( scripthandle, "Sound Setup", "ReverseStereo",dummy,FALSE,FALSE);
   SCRIPT_PutString( scripthandle, "Sound Setup", "OggTrackName", gs.OggTrackName);
   
   SCRIPT_PutNumber( scripthandle, "Setup", "ForceSetup",ForceSetup,FALSE,FALSE);
   SCRIPT_PutNumber( scripthandle, "Controls","UseMouse",UseMouse,FALSE,FALSE);
   SCRIPT_PutNumber( scripthandle, "Controls","UseJoystick",UseJoystick,FALSE,FALSE);
   SCRIPT_PutNumber( scripthandle, "Controls","MouseSensitivity",gs.MouseSpeed,FALSE,FALSE);
   
   WriteGameSetup(scripthandle);
   
   for (dummy=0;dummy<NUMGAMEFUNCTIONS;dummy++) {
       SCRIPT_PutDoubleString( scripthandle, "KeyDefinitions", CONFIG_FunctionNumToName(dummy),
           KB_ScanCodeToString(KeyboardKeys[dummy][0]), KB_ScanCodeToString(KeyboardKeys[dummy][1]));
   }

   for (dummy=0;dummy<MAXMOUSEBUTTONS;dummy++) {
       Bsprintf(buf,"MouseButton%d",dummy);
       SCRIPT_PutString( scripthandle,"Controls", buf, CONFIG_FunctionNumToName( MouseFunctions[dummy][0] ));

       if (dummy >= (MAXMOUSEBUTTONS-2)) continue;	// scroll wheel
		
       Bsprintf(buf,"MouseButtonClicked%d",dummy);
       SCRIPT_PutString( scripthandle,"Controls", buf, CONFIG_FunctionNumToName( MouseFunctions[dummy][1] ));
   }

   for (dummy=0;dummy<MAXMOUSEAXES;dummy++) {
       Bsprintf(buf,"MouseAnalogAxes%d",dummy);
       SCRIPT_PutString(scripthandle, "Controls", buf, CONFIG_AnalogNumToName( MouseAnalogueAxes[dummy] ));

       Bsprintf(buf,"MouseDigitalAxes%d_0",dummy);
       SCRIPT_PutString(scripthandle, "Controls", buf, CONFIG_FunctionNumToName(MouseDigitalFunctions[dummy][0]));

       Bsprintf(buf,"MouseDigitalAxes%d_1",dummy);
       SCRIPT_PutString(scripthandle, "Controls", buf, CONFIG_FunctionNumToName(MouseDigitalFunctions[dummy][1]));
		
       Bsprintf(buf,"MouseAnalogScale%d",dummy);
       SCRIPT_PutNumber(scripthandle, "Controls", buf, MouseAnalogueScale[dummy], FALSE, FALSE);
   }

   for (dummy=0;dummy<MAXJOYBUTTONS;dummy++) {
       Bsprintf(buf,"JoystickButton%d",dummy);
       SCRIPT_PutString( scripthandle,"Controls", buf, CONFIG_FunctionNumToName( JoystickFunctions[dummy][0] ));

       Bsprintf(buf,"JoystickButtonClicked%d",dummy);
       SCRIPT_PutString( scripthandle,"Controls", buf, CONFIG_FunctionNumToName( JoystickFunctions[dummy][1] ));
   }

   for (dummy=0;dummy<MAXJOYAXES;dummy++) {
       Bsprintf(buf,"JoystickAnalogAxes%d",dummy);
       SCRIPT_PutString(scripthandle, "Controls", buf, CONFIG_AnalogNumToName( JoystickAnalogueAxes[dummy] ));

       Bsprintf(buf,"JoystickDigitalAxes%d_0",dummy);
       SCRIPT_PutString(scripthandle, "Controls", buf, CONFIG_FunctionNumToName(JoystickDigitalFunctions[dummy][0]));

       Bsprintf(buf,"JoystickDigitalAxes%d_1",dummy);
       SCRIPT_PutString(scripthandle, "Controls", buf, CONFIG_FunctionNumToName(JoystickDigitalFunctions[dummy][1]));
		
       Bsprintf(buf,"JoystickAnalogScale%d",dummy);
       SCRIPT_PutNumber(scripthandle, "Controls", buf, JoystickAnalogueScale[dummy], FALSE, FALSE);

       Bsprintf(buf,"JoystickAnalogDead%d",dummy);
       SCRIPT_PutNumber(scripthandle, "Controls", buf, JoystickAnalogueDead[dummy], FALSE, FALSE);

       Bsprintf(buf,"JoystickAnalogSaturate%d",dummy);
       SCRIPT_PutNumber(scripthandle, "Controls", buf, JoystickAnalogueSaturate[dummy], FALSE, FALSE);
   }
   
   SCRIPT_Save (scripthandle, setupfilename);
   SCRIPT_Free (scripthandle);
   }

