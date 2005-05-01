#include "types.h"
#include "keyboard.h"
#include "mouse.h"
#include "control.h"
#include "_control.h"
#include "util_lib.h"

#include "baselayer.h"
#include "compat.h"


boolean CONTROL_JoyPresent = false;
boolean CONTROL_JoystickEnabled = false;
boolean CONTROL_MousePresent = false;
boolean CONTROL_MouseEnabled = false;
uint32  CONTROL_ButtonState1 = 0;
uint32  CONTROL_ButtonHeldState1 = 0;
uint32  CONTROL_ButtonState2 = 0;
uint32  CONTROL_ButtonHeldState2 = 0;

static int32 CONTROL_UserInputDelay = -1;
static int32 CONTROL_MouseSensitivity = DEFAULTMOUSESENSITIVITY;
static int32 CONTROL_NumButtons = 0;
static int32 CONTROL_NumAxes = 0;
static controlflags       CONTROL_Flags[CONTROL_NUM_FLAGS];
static controlbuttontype  CONTROL_DeviceButtonMapping[MAXBUTTONS];
static controlkeymaptype  CONTROL_KeyMapping[CONTROL_NUM_FLAGS];
static controlaxismaptype CONTROL_AxesMap[MAXAXES];	// maps physical axes onto virtual ones
static controlaxistype    CONTROL_Axes[MAXAXES];	// physical axes
static controlaxistype    CONTROL_LastAxes[MAXAXES];
static int32 CONTROL_AxesScale[MAXAXES];
static int32 CONTROL_DeviceButtonState[MAXBUTTONS];
static int32 CONTROL_ButtonClickedTime[MAXBUTTONS];
static boolean CONTROL_ButtonClickedState[MAXBUTTONS];
static boolean CONTROL_ButtonClicked[MAXBUTTONS];
static byte  CONTROL_ButtonClickedCount[MAXBUTTONS];
static boolean CONTROL_UserInputCleared[3];
static int32 (*GetTime)(void);
static boolean CONTROL_Started = false;
static int32 ticrate;
static int32 CONTROL_DoubleClickSpeed;


void CONTROL_GetMouseDelta(int32 *x, int32 *y)
{
	MOUSE_GetDelta(x,y);

	/* What in the name of all things sacred is this?
	if (labs(*x) > labs(*y)) {
		*x /= 3;
	} else {
		*y /= 3;
	}
	*y = *y * 96;
	*x = (*x * 32 * CONTROL_MouseSensitivity) >> 15;
	*/
}

int32 CONTROL_GetMouseSensitivity(void)
{
	return (CONTROL_MouseSensitivity - MINIMUMMOUSESENSITIVITY);
}

void CONTROL_SetMouseSensitivity(int32 newsensitivity)
{
	CONTROL_MouseSensitivity = newsensitivity + MINIMUMMOUSESENSITIVITY;
}

byte CONTROL_GetMouseButtons(void)
{
	return MOUSE_GetButtons();
}

boolean CONTROL_StartMouse(void)
{
	CONTROL_NumButtons = MAXMOUSEBUTTONS;
	return MOUSE_Init();
}

void CONTROL_GetJoyAbs( int32 joy )
{
}

void CONTROL_FilterJoyDelta()
{
/*
012C                          CONTROL_FilterJoyDelta_:
012C    51                        push        ecx
012D    56                        push        esi
012E    57                        push        edi
012F    55                        push        ebp
0130    89 D1                     mov         ecx,edx
0132    89 DE                     mov         esi,ebx
0134    89 C2                     mov         edx,eax
0136    C1 E0 02                  shl         eax,0x00000002
0139    29 D0                     sub         eax,edx
013B    C1 E0 04                  shl         eax,0x00000004
013E    05 00 00 00 00            add         eax,offset _JoyDefs
0143    8B 11                     mov         edx,[ecx]
0145    8B 78 08                  mov         edi,0x8[eax]
0148    8B 1B                     mov         ebx,[ebx]
014A    39 FA                     cmp         edx,edi
014C    7D 1B                     jge         L$6
014E    8B 28                     mov         ebp,[eax]
0150    39 EA                     cmp         edx,ebp
0152    7D 02                     jge         L$5
0154    89 EA                     mov         edx,ebp
0156                          L$5:
0156    2B 50 08                  sub         edx,0x8[eax]
0159    8B 68 20                  mov         ebp,0x20[eax]
015C    F7 DA                     neg         edx
015E    0F AF D5                  imul        edx,ebp
0161    89 11                     mov         [ecx],edx
0163    F7 DA                     neg         edx
0165    89 11                     mov         [ecx],edx
0167    EB 23                     jmp         L$9
0169                          L$6:
0169    3B 50 10                  cmp         edx,0x10[eax]
016C    7E 18                     jle         L$8
016E    8B 78 18                  mov         edi,0x18[eax]
0171    39 FA                     cmp         edx,edi
0173    7E 02                     jle         L$7
0175    89 FA                     mov         edx,edi
0177                          L$7:
0177    8B 68 10                  mov         ebp,0x10[eax]
017A    8B 78 28                  mov         edi,0x28[eax]
017D    29 EA                     sub         edx,ebp
017F    0F AF D7                  imul        edx,edi
0182    89 11                     mov         [ecx],edx
0184    EB 06                     jmp         L$9
0186                          L$8:
0186    C7 01 00 00 00 00         mov         dword ptr [ecx],0x00000000
018C                          L$9:
018C    3B 58 0C                  cmp         ebx,0xc[eax]
018F    7D 1F                     jge         L$11
0191    8B 78 04                  mov         edi,0x4[eax]
0194    39 FB                     cmp         ebx,edi
0196    7D 02                     jge         L$10
0198    89 FB                     mov         ebx,edi
019A                          L$10:
019A    2B 58 0C                  sub         ebx,0xc[eax]
019D    8B 50 24                  mov         edx,0x24[eax]
01A0    F7 DB                     neg         ebx
01A2    0F AF DA                  imul        ebx,edx
01A5    89 1E                     mov         [esi],ebx
01A7    F7 DB                     neg         ebx
01A9    89 1E                     mov         [esi],ebx
01AB    5D                        pop         ebp
01AC    5F                        pop         edi
01AD    5E                        pop         esi
01AE    59                        pop         ecx
01AF    C3                        ret         
01B0                          L$11:
01B0    3B 58 14                  cmp         ebx,0x14[eax]
01B3    7E 1B                     jle         L$13
01B5    8B 68 1C                  mov         ebp,0x1c[eax]
01B8    39 EB                     cmp         ebx,ebp
01BA    7E 02                     jle         L$12
01BC    89 EB                     mov         ebx,ebp
01BE                          L$12:
01BE    8B 50 14                  mov         edx,0x14[eax]
01C1    8B 48 2C                  mov         ecx,0x2c[eax]
01C4    29 D3                     sub         ebx,edx
01C6    0F AF D9                  imul        ebx,ecx
01C9    89 1E                     mov         [esi],ebx
01CB    5D                        pop         ebp
01CC    5F                        pop         edi
01CD    5E                        pop         esi
01CE    59                        pop         ecx
01CF    C3                        ret         
01D0                          L$13:
01D0    C7 06 00 00 00 00         mov         dword ptr [esi],0x00000000
01D6    5D                        pop         ebp
01D7    5F                        pop         edi
01D8    5E                        pop         esi
01D9    59                        pop         ecx
01DA    C3                        ret         
*/
}

void CONTROL_GetJoyDelta( int32 joy, int32 *dx, int32 *dy, int32 *rudder, int32 *throttle )
{
/*
01DC                          CONTROL_GetJoyDelta_:
01DC    56                        push        esi
01DD    57                        push        edi
01DE    8B 7C 24 0C               mov         edi,0xc[esp]
01E2    89 C6                     mov         esi,eax
01E4    E8 00 00 00 00            call        CONTROL_GetJoyAbs_
01E9    A1 00 00 00 00            mov         eax,_CONTROL_JoyXAxis
01EE    89 02                     mov         [edx],eax
01F0    A1 00 00 00 00            mov         eax,_CONTROL_JoyYAxis
01F5    89 03                     mov         [ebx],eax
01F7    89 F0                     mov         eax,esi
01F9    E8 00 00 00 00            call        CONTROL_FilterJoyDelta_
01FE    85 F6                     test        esi,esi
0200    75 2E                     jne         L$15
0202    83 3D 00 00 00 00 00      cmp         dword ptr _CONTROL_RudderEnabled,0x00000000
0209    75 09                     jne         L$14
020B    83 3D 00 00 00 00 00      cmp         dword ptr _CONTROL_ThrottleEnabled,0x00000000
0212    74 1C                     je          L$15
0214                          L$14:
0214    A1 00 00 00 00            mov         eax,_CONTROL_JoyXAxis2
0219    89 FB                     mov         ebx,edi
021B    89 01                     mov         [ecx],eax
021D    A1 00 00 00 00            mov         eax,_CONTROL_JoyYAxis2
0222    89 CA                     mov         edx,ecx
0224    89 07                     mov         [edi],eax
0226    B8 01 00 00 00            mov         eax,0x00000001
022B    E8 00 00 00 00            call        CONTROL_FilterJoyDelta_
0230                          L$15:
0230    5F                        pop         edi
0231    5E                        pop         esi
0232    C2 04 00                  ret         0x00000004
*/
}

byte CONTROL_JoyButtons( int32 joy )
{
/*
0238                          L$16:
0238    D2 02 00 00                                     DD	L$26
023C    DB 02 00 00                                     DD	L$27
0240    E4 02 00 00                                     DD	L$28
0244    ED 02 00 00                                     DD	L$29

0248                          CONTROL_JoyButtons_:
0248    53                        push        ebx
0249    51                        push        ecx
024A    52                        push        edx
024B    89 C1                     mov         ecx,eax
024D    BA 01 02 00 00            mov         edx,0x00000201
0252    29 C0                     sub         eax,eax
0254    EC                        in          al,dx
0255    85 C9                     test        ecx,ecx
0257    74 07                     je          L$17
0259    88 C3                     mov         bl,al
025B    C0 EB 06                  shr         bl,0x06
025E    EB 05                     jmp         L$18
0260                          L$17:
0260    88 C3                     mov         bl,al
0262    C0 EB 04                  shr         bl,0x04
0265                          L$18:
0265    85 C9                     test        ecx,ecx
0267    75 04                     jne         L$19
0269    B1 0F                     mov         cl,0x0f
026B    EB 02                     jmp         L$20
026D                          L$19:
026D    B1 03                     mov         cl,0x03
026F                          L$20:
026F    20 CB                     and         bl,cl
0271    8B 15 00 00 00 00         mov         edx,_CONTROL_FlightStickMode
0277    30 CB                     xor         bl,cl
0279    85 D2                     test        edx,edx
027B    74 2E                     je          L$25
027D    80 FB 07                  cmp         bl,0x07
0280    72 14                     jb          L$21
0282    76 1B                     jbe         L$22
0284    80 FB 0B                  cmp         bl,0x0b
0287    0F 82 63 00 00 00         jb          L$30
028D    76 14                     jbe         L$23
028F    80 FB 0F                  cmp         bl,0x0f
0292    74 13                     je          L$24
0294    EB 5A                     jmp         L$30
0296                          L$21:
0296    80 FB 03                  cmp         bl,0x03
0299    75 55                     jne         L$30
029B    B3 80                     mov         bl,0x80
029D    EB 51                     jmp         L$30
029F                          L$22:
029F    B3 40                     mov         bl,0x40
02A1    EB 4D                     jmp         L$30
02A3                          L$23:
02A3    B3 20                     mov         bl,0x20
02A5    EB 49                     jmp         L$30
02A7                          L$24:
02A7    B3 10                     mov         bl,0x10
02A9    EB 45                     jmp         L$30
02AB                          L$25:
02AB    83 3D 00 00 00 00 00      cmp         dword ptr _CONTROL_HatEnabled,0x00000000
02B2    74 3C                     je          L$30
02B4    A1 00 00 00 00            mov         eax,_CONTROL_JoyYAxis2
02B9    89 C2                     mov         edx,eax
02BB    B9 64 00 00 00            mov         ecx,0x00000064
02C0    C1 FA 1F                  sar         edx,0x0000001f
02C3    F7 F9                     idiv        ecx
02C5    83 F8 03                  cmp         eax,0x00000003
02C8    77 26                     ja          L$30
02CA    2E FF 24 85 38 02 00 00
                                  jmp         dword ptr cs:L$16[eax*4]
02D2                          L$26:
02D2    80 CB 10                  or          bl,0x10
02D5    88 D8                     mov         al,bl
02D7    5A                        pop         edx
02D8    59                        pop         ecx
02D9    5B                        pop         ebx
02DA    C3                        ret         
02DB                          L$27:
02DB    80 CB 20                  or          bl,0x20
02DE    88 D8                     mov         al,bl
02E0    5A                        pop         edx
02E1    59                        pop         ecx
02E2    5B                        pop         ebx
02E3    C3                        ret         
02E4                          L$28:
02E4    80 CB 40                  or          bl,0x40
02E7    88 D8                     mov         al,bl
02E9    5A                        pop         edx
02EA    59                        pop         ecx
02EB    5B                        pop         ebx
02EC    C3                        ret         
02ED                          L$29:
02ED    80 CB 80                  or          bl,0x80
02F0                          L$30:
02F0    88 D8                     mov         al,bl
02F2    5A                        pop         edx
02F3    59                        pop         ecx
02F4    5B                        pop         ebx
02F5    C3                        ret         
*/
	return 0;
}

void CONTROL_SetJoyScale( int32 joy )
{
/*
02F8                          CONTROL_SetJoyScale_:
02F8    53                        push        ebx
02F9    51                        push        ecx
02FA    52                        push        edx
02FB    56                        push        esi
02FC    57                        push        edi
02FD    55                        push        ebp
02FE    89 C6                     mov         esi,eax
0300    8D 0C 85 00 00 00 00      lea         ecx,[eax*4]
0307    29 C1                     sub         ecx,eax
0309    BE 00 00 00 00            mov         esi,offset _JoyDefs
030E    C1 E1 04                  shl         ecx,0x00000004
0311    01 CE                     add         esi,ecx
0313    8B 4E 08                  mov         ecx,0x8[esi]
0316    8B 16                     mov         edx,[esi]
0318    BB 00 0A 00 00            mov         ebx,0x00000a00
031D    29 D1                     sub         ecx,edx
031F    75 05                     jne         L$31
0321    B9 01 00 00 00            mov         ecx,0x00000001
0326                          L$31:
0326    89 DA                     mov         edx,ebx
0328    89 D8                     mov         eax,ebx
032A    C1 FA 1F                  sar         edx,0x0000001f
032D    F7 F9                     idiv        ecx
032F    8B 7E 10                  mov         edi,0x10[esi]
0332    8B 4E 18                  mov         ecx,0x18[esi]
0335    89 46 20                  mov         0x20[esi],eax
0338    29 F9                     sub         ecx,edi
033A    75 05                     jne         L$32
033C    B9 01 00 00 00            mov         ecx,0x00000001
0341                          L$32:
0341    89 DA                     mov         edx,ebx
0343    89 D8                     mov         eax,ebx
0345    C1 FA 1F                  sar         edx,0x0000001f
0348    F7 F9                     idiv        ecx
034A    8B 6E 04                  mov         ebp,0x4[esi]
034D    8B 4E 0C                  mov         ecx,0xc[esi]
0350    89 46 28                  mov         0x28[esi],eax
0353    29 E9                     sub         ecx,ebp
0355    75 05                     jne         L$33
0357    B9 01 00 00 00            mov         ecx,0x00000001
035C                          L$33:
035C    89 DA                     mov         edx,ebx
035E    89 D8                     mov         eax,ebx
0360    C1 FA 1F                  sar         edx,0x0000001f
0363    F7 F9                     idiv        ecx
0365    89 46 24                  mov         0x24[esi],eax
0368    8B 4E 1C                  mov         ecx,0x1c[esi]
036B    2B 4E 14                  sub         ecx,0x14[esi]
036E    75 05                     jne         L$34
0370    B9 01 00 00 00            mov         ecx,0x00000001
0375                          L$34:
0375    89 DA                     mov         edx,ebx
0377    89 D8                     mov         eax,ebx
0379    C1 FA 1F                  sar         edx,0x0000001f
037C    F7 F9                     idiv        ecx
037E    89 46 2C                  mov         0x2c[esi],eax
0381    5D                        pop         ebp
0382    5F                        pop         edi
0383    5E                        pop         esi
0384    5A                        pop         edx
0385    59                        pop         ecx
0386    5B                        pop         ebx
0387    C3                        ret         
*/
}

void CONTROL_SetupJoy( int32 joy, int32 minx, int32 maxx, int32 miny, int32 maxy, int32 centerx, int32 centery )
{
}

void CONTROL_CenterJoystick
   (
   void ( *CenterCenter )( void ),
   void ( *UpperLeft )( void ),
   void ( *LowerRight )( void ),
   void ( *CenterThrottle )( void ),
   void ( *CenterRudder )( void )
   )
{
}

boolean CONTROL_StartJoy(int32 joy)
{
	return (inputdevices & 3) == 3;
}

void CONTROL_ShutJoy(int32 joy)
{
	CONTROL_JoyPresent = false;
}

int32 CONTROL_GetTime(void)
{
	static int32 t = 0;

	t += 5;

	return t;
}

void CONTROL_CheckRange(int32 which)
{
	if ((uint32)which < (uint32)CONTROL_NUM_FLAGS) return;
	Error("CONTROL_CheckRange: Index %d out of valid range for %d control flags.",
		which, CONTROL_NUM_FLAGS);
}

void CONTROL_SetFlag(int32 which, boolean active)
{
	CONTROL_CheckRange(which);

	if (CONTROL_Flags[which].toggle == INSTANT_ONOFF) {
		CONTROL_Flags[which].active = active;
	} else {
		if (active) {
			CONTROL_Flags[which].buttonheld = false;
		} else if (CONTROL_Flags[which].buttonheld == false) {
			CONTROL_Flags[which].buttonheld = true;
			CONTROL_Flags[which].active = (CONTROL_Flags[which].active ? false : true);
		}
	}
}

boolean CONTROL_KeyboardFunctionPressed(int32 which)
{
	boolean key1, key2;

	CONTROL_CheckRange(which);

	if (!CONTROL_Flags[which].used) return false;

	key1 = KB_KeyDown[ CONTROL_KeyMapping[which].key1 ] ? true : false;
	key2 = KB_KeyDown[ CONTROL_KeyMapping[which].key2 ] ? true : false;

	return (key1 | key2);
}

void CONTROL_ClearKeyboardFunction(int32 which)
{
	CONTROL_CheckRange(which);

	if (!CONTROL_Flags[which].used) return;

	KB_KeyDown[ CONTROL_KeyMapping[which].key1 ] =
	KB_KeyDown[ CONTROL_KeyMapping[which].key2 ] = 0;
}

void CONTROL_DefineFlag( int32 which, boolean toggle )
{
	CONTROL_CheckRange(which);

	CONTROL_Flags[which].active     = false;
	CONTROL_Flags[which].used       = true;
	CONTROL_Flags[which].toggle     = toggle;
	CONTROL_Flags[which].buttonheld = false;
	CONTROL_Flags[which].cleared    = 0;
}

boolean CONTROL_FlagActive( int32 which )
{
	CONTROL_CheckRange(which);

	return CONTROL_Flags[which].used;
}

void CONTROL_MapKey( int32 which, kb_scancode key1, kb_scancode key2 )
{
	CONTROL_CheckRange(which);

	CONTROL_KeyMapping[which].key1 = key1;
	CONTROL_KeyMapping[which].key2 = key2;
}

void CONTROL_PrintKeyMap(void)
{
	int32 i;

	for (i=0;i<CONTROL_NUM_FLAGS;i++) {
		initprintf("function %2ld key1=%3x key2=%3x\n",
			i, CONTROL_KeyMapping[i].key1, CONTROL_KeyMapping[i].key2);
	}
}

void CONTROL_PrintControlFlag(int32 which)
{
	initprintf("function %2ld active=%d used=%d toggle=%d buttonheld=%d cleared=%ld\n",
		which, CONTROL_Flags[which].active, CONTROL_Flags[which].used,
		CONTROL_Flags[which].toggle, CONTROL_Flags[which].buttonheld,
		CONTROL_Flags[which].cleared);
}

void CONTROL_PrintAxes(void)
{
	int32 i;

	initprintf("numaxes=%ld\n", CONTROL_NumAxes);
	for (i=0;i<CONTROL_NumAxes;i++) {
		initprintf("axis=%ld analog=%d digital1=%d digital2=%d\n",
				i, CONTROL_AxesMap[i].analogmap,
				CONTROL_AxesMap[i].minmap, CONTROL_AxesMap[i].maxmap);
	}
}

void CONTROL_MapButton( int32 whichfunction, int32 whichbutton, boolean doubleclicked, controldevice device )
{
	CONTROL_CheckRange(whichfunction);
	if ((uint32)whichbutton >= (uint32)CONTROL_NumButtons)
		Error("CONTROL_MapButton: button %d out of valid range for %d buttons.", whichbutton, CONTROL_NumButtons);

	if (doubleclicked)
		CONTROL_DeviceButtonMapping[whichbutton].doubleclicked = whichfunction;
	else
		CONTROL_DeviceButtonMapping[whichbutton].singleclicked = whichfunction;
}

void CONTROL_MapAnalogAxis( int32 whichaxis, int32 whichanalog, controldevice device )
{
	if ((uint32)whichaxis >= (uint32)MAXAXES)
		Error("CONTROL_MapAnalogAxis: axis %d out of valid range for %d axes.", whichaxis, MAXAXES);
	if ((uint32)whichanalog >= (uint32)analog_maxtype)
		Error("CONTROL_MapAnalogAxis: analog function %d out of valid range for %d analog functions.",
				whichanalog, analog_maxtype);

	CONTROL_AxesMap[whichaxis].analogmap = whichanalog;
}

void CONTROL_SetAnalogAxisScale( int32 whichaxis, int32 axisscale, controldevice device )
{
	if ((uint32)whichaxis >= (uint32)MAXAXES)
		Error("CONTROL_SetAnalogAxisScale: axis %d out of valid range for %d axes.", whichaxis, MAXAXES);

	CONTROL_AxesScale[whichaxis] = axisscale;
}

void CONTROL_MapDigitalAxis( int32 whichaxis, int32 whichfunction, int32 direction, controldevice device )
{
	CONTROL_CheckRange(whichfunction);
	if ((uint32)whichaxis >= (uint32)MAXAXES)
		Error("CONTROL_MapDigitalAxis: axis %d out of valid range for %d axes.", whichaxis, MAXAXES);

	switch (direction) {	// JBF: this is all very much a guess. The ASM puzzles me.
		case axis_up:
		case axis_left:
			CONTROL_AxesMap[whichaxis].minmap = whichfunction;
			break;
		case axis_down:
		case axis_right:
			CONTROL_AxesMap[whichaxis].maxmap = whichfunction;
			break;
		default:
			break;
	}
}

void CONTROL_ClearFlags(void)
{
	int32 i;

	for (i=0;i<CONTROL_NUM_FLAGS;i++)
		CONTROL_Flags[i].used = false;
}

void CONTROL_ClearAssignments( void )
{
	int32 i;

	memset(CONTROL_DeviceButtonMapping, BUTTONUNDEFINED, sizeof(CONTROL_DeviceButtonMapping));
	memset(CONTROL_KeyMapping,          KEYUNDEFINED,    sizeof(CONTROL_KeyMapping));
	memset(CONTROL_AxesMap,             AXISUNDEFINED,   sizeof(CONTROL_AxesMap));
	memset(CONTROL_Axes,                0,               sizeof(CONTROL_Axes));
	memset(CONTROL_LastAxes,            0,               sizeof(CONTROL_LastAxes));
	for (i=0;i<MAXAXES;i++)
		CONTROL_AxesScale[i] = NORMALAXISSCALE;
}

void CONTROL_GetDeviceButtons(void)
{
	int32 t,i;
	int32 b=0,bs;

	t = GetTime();

	if (CONTROL_MouseEnabled)
		b = MOUSE_GetButtons();

	if (CONTROL_JoystickEnabled)
		b = CONTROL_JoyButtons(0);

	if (CONTROL_NumButtons <= 0) return;

	for (i=0;i<CONTROL_NumButtons;i++) {
		bs = (b >> i) & 1;

		CONTROL_DeviceButtonState[i] = bs;
		CONTROL_ButtonClickedState[i] = false;

		if (bs) {
			if (CONTROL_ButtonClicked[i] == false) {
				CONTROL_ButtonClicked[i] = true;

				if (CONTROL_ButtonClickedCount[i] == false) {
					CONTROL_ButtonClickedTime[i] = t + CONTROL_DoubleClickSpeed;
					CONTROL_ButtonClickedCount[i] = 1;
				}
				else if (t < CONTROL_ButtonClickedTime[i]) {
					CONTROL_ButtonClickedState[i] = true;
					CONTROL_ButtonClickedTime[i]  = 0;
					CONTROL_ButtonClickedCount[i] = 2;
				}
			}
			else if (CONTROL_ButtonClickedCount[i] == 2) {
				CONTROL_ButtonClickedState[i] = true;
			}
		} else {
			if (CONTROL_ButtonClickedCount[i] == 2)
				CONTROL_ButtonClickedCount[i] = 0;

			CONTROL_ButtonClicked[i] = false;
		}
	}
}

void CONTROL_DigitizeAxis(int32 axis)
{
	if (CONTROL_Axes[axis].analog > 0) {
		if (CONTROL_Axes[axis].analog > THRESHOLD) {			// if very much in one direction,
			CONTROL_Axes[axis].digital = 1;				// set affirmative
		} else {
			if (CONTROL_Axes[axis].analog > MINTHRESHOLD) {		// if hanging in limbo,
				if (CONTROL_LastAxes[axis].digital == 1)	// set if in same direction as last time
					CONTROL_Axes[axis].digital = 1;
			}
		}
	} else {
		if (CONTROL_Axes[axis].analog < -THRESHOLD) {
			CONTROL_Axes[axis].digital = -1;
		} else {
			if (CONTROL_Axes[axis].analog < -MINTHRESHOLD) {
				if (CONTROL_LastAxes[axis].digital == -1)
					CONTROL_LastAxes[axis].digital = -1;
			}
		}
	}
}

void CONTROL_ScaleAxis(int32 axis)
{
	CONTROL_Axes[axis].analog = (int32)(((int64)CONTROL_Axes[axis].analog * (int64)CONTROL_AxesScale[axis]) >> 16);
}

void CONTROL_ApplyAxis(int32 axis, ControlInfo *info)
{
	switch (CONTROL_AxesMap[axis].analogmap) {
		case analog_turning:          info->dyaw   += CONTROL_Axes[axis].analog; break;
		case analog_strafing:         info->dx     += CONTROL_Axes[axis].analog; break;
		case analog_lookingupanddown: info->dpitch += CONTROL_Axes[axis].analog; break;
		case analog_elevation:        info->dy     += CONTROL_Axes[axis].analog; break;
		case analog_rolling:          info->droll  += CONTROL_Axes[axis].analog; break;
		case analog_moving:           info->dz     += CONTROL_Axes[axis].analog; break;
		default: break;
	}
}

void CONTROL_PollDevices(ControlInfo *info)
{
	int32 i;

	memcpy(CONTROL_LastAxes, CONTROL_Axes, sizeof(CONTROL_Axes));

	memset(CONTROL_Axes, 0, sizeof(CONTROL_Axes));
	memset(info, 0, sizeof(ControlInfo));

	if (CONTROL_MouseEnabled) {
		CONTROL_GetMouseDelta(&CONTROL_Axes[0].analog, &CONTROL_Axes[1].analog);
	}
	else if (CONTROL_JoystickEnabled) {
		CONTROL_GetJoyDelta(0,
				&CONTROL_Axes[0].analog,
				&CONTROL_Axes[1].analog,
				&CONTROL_Axes[2].analog,
				&CONTROL_Axes[3].analog
				);

		// Why?
		//CONTROL_Axes[0].analog /= 2;
		//CONTROL_Axes[2].analog /= 2;
	}
	
	if (CONTROL_NumAxes > 0) {
		for (i=0; i<MAXAXES; i++) {
			CONTROL_DigitizeAxis(i);
			CONTROL_ScaleAxis(i);
			LIMITCONTROL(&CONTROL_Axes[i].analog);
			CONTROL_ApplyAxis(i, info);
		}
	}

	CONTROL_GetDeviceButtons();
}

void CONTROL_AxisFunctionState(int32 *p1)
{
	int32 i, j;
	
	for (i=0; i<CONTROL_NumAxes; i++) {
		if (!CONTROL_Axes[i].digital) continue;

		if (CONTROL_Axes[i].digital < 0)
			j = CONTROL_AxesMap[i].minmap;
		else
			j = CONTROL_AxesMap[i].maxmap;

		if (j != AXISUNDEFINED)
			p1[j] = 1;
	}
}

void CONTROL_ButtonFunctionState( int32 *p1 )
{
	int32 i, j;

	for (i=0; i<CONTROL_NumButtons; i++) {
		j = CONTROL_DeviceButtonMapping[i].doubleclicked;
		if (j != KEYUNDEFINED)
			p1[j] |= CONTROL_ButtonClickedState[i];

		j = CONTROL_DeviceButtonMapping[i].singleclicked;
		if (j != KEYUNDEFINED)
			p1[j] |= CONTROL_ButtonClickedState[i];
	}
}

void CONTROL_GetUserInput( UserInput *info )
{
	ControlInfo ci;

	CONTROL_PollDevices( &ci );

	info->dir = dir_None;

	// checks if CONTROL_UserInputDelay is too far in the future?
	if (GetTime() + ((ticrate * USERINPUTDELAY) / 1000) < CONTROL_UserInputDelay)
		CONTROL_UserInputDelay = -1;

	if (GetTime() >= CONTROL_UserInputDelay) {
		if (CONTROL_Axes[1].digital == -1)
			info->dir = dir_North;
		else if (CONTROL_Axes[1].digital == 1)
			info->dir = dir_South;
		else if (CONTROL_Axes[0].digital == -1)
			info->dir = dir_West;
		else if (CONTROL_Axes[0].digital == 1)
			info->dir = dir_East;
	}

	info->button0 = CONTROL_DeviceButtonState[0];
	info->button1 = CONTROL_DeviceButtonState[1];

	if (KB_KeyDown[sc_kpad_8] || KB_KeyDown[sc_UpArrow])
		info->dir = dir_North;
	else if (KB_KeyDown[sc_kpad_2] || KB_KeyDown[sc_DownArrow])
		info->dir = dir_South;
	else if (KB_KeyDown[sc_kpad_4] || KB_KeyDown[sc_LeftArrow])
		info->dir = dir_West;
	else if (KB_KeyDown[sc_kpad_6] || KB_KeyDown[sc_RightArrow])
		info->dir = dir_East;

	if (KB_KeyDown[BUTTON0_SCAN_1] || KB_KeyDown[BUTTON0_SCAN_2] || KB_KeyDown[BUTTON0_SCAN_3])
		info->button0 = 1;
	if (KB_KeyDown[BUTTON1_SCAN])
		info->button1 = 1;

	if (CONTROL_UserInputCleared[1]) {
		if (!info->button0)
			CONTROL_UserInputCleared[1] = false;
		else
			info->button0 = false;
	}
	if (CONTROL_UserInputCleared[2]) {
		if (!info->button1)
			CONTROL_UserInputCleared[2] = false;
		else
			info->button1 = false;
	}
}

void CONTROL_ClearUserInput( UserInput *info )
{
	switch (info->dir) {
		case dir_North:
		case dir_South:
		case dir_East:
		case dir_West:
			CONTROL_UserInputCleared[0] = true;
			CONTROL_UserInputDelay = GetTime() + ((ticrate * USERINPUTDELAY) / 1000);
			switch (info->dir) {
				case dir_North: KB_KeyDown[sc_UpArrow]    = KB_KeyDown[sc_kpad_8] = 0; break;
				case dir_South: KB_KeyDown[sc_DownArrow]  = KB_KeyDown[sc_kpad_2] = 0; break;
				case dir_East:  KB_KeyDown[sc_LeftArrow]  = KB_KeyDown[sc_kpad_4] = 0; break;
				case dir_West:  KB_KeyDown[sc_RightArrow] = KB_KeyDown[sc_kpad_6] = 0; break;
				default: break;
			}
			break;
		default: break;
	}
	if (info->button0) CONTROL_UserInputCleared[1] = true;
	if (info->button1) CONTROL_UserInputCleared[2] = true;
}

void CONTROL_ClearButton( int32 whichbutton )
{
	CONTROL_CheckRange( whichbutton );
	BUTTONCLEAR( whichbutton );
	CONTROL_Flags[whichbutton].cleared = true;
}

void CONTROL_GetInput( ControlInfo *info )
{
	int32 i, periphs[CONTROL_NUM_FLAGS];

	CONTROL_PollDevices( info );

	memset(periphs, 0, sizeof(periphs));
	CONTROL_ButtonFunctionState(periphs);
	CONTROL_AxisFunctionState(periphs);

	CONTROL_ButtonHeldState1 = CONTROL_ButtonState1;
	CONTROL_ButtonHeldState2 = CONTROL_ButtonState2;
	CONTROL_ButtonState1 = CONTROL_ButtonState2 = 0;

	for (i=0; i<CONTROL_NUM_FLAGS; i++) {
		CONTROL_SetFlag(i, CONTROL_KeyboardFunctionPressed(i) | periphs[i]);

		if (CONTROL_Flags[i].cleared && !CONTROL_Flags[i].active)
			CONTROL_Flags[i].cleared = 0;
		else
			BUTTONSET(i, CONTROL_Flags[i].active);
	}
}

void CONTROL_WaitRelease( void )
{
/*
155C                          CONTROL_WaitRelease_:
155C    83 EC 0C                  sub         esp,0x0000000c
155F                          L$170:
155F    89 E0                     mov         eax,esp
1561    E8 00 00 00 00            call        CONTROL_GetUserInput_
1566    80 7C 24 08 08            cmp         byte ptr 0x8[esp],0x08
156B    75 F2                     jne         L$170
156D    83 3C 24 00               cmp         dword ptr [esp],0x00000000
1571    75 EC                     jne         L$170
1573    83 7C 24 04 00            cmp         dword ptr 0x4[esp],0x00000000
1578    75 E5                     jne         L$170
157A    83 C4 0C                  add         esp,0x0000000c
157D    C3                        ret         
157E    8B C0                     mov         eax,eax
*/
}

void CONTROL_Ack( void )
{
/*
1580                          CONTROL_Ack_:
1580    53                        push        ebx
1581    51                        push        ecx
1582    52                        push        edx
1583    56                        push        esi
1584    55                        push        ebp
1585    83 EC 18                  sub         esp,0x00000018
1588    8D 44 24 0C               lea         eax,0xc[esp]
158C    E8 00 00 00 00            call        CONTROL_GetUserInput_
1591    31 DB                     xor         ebx,ebx
1593                          L$171:
1593    89 E0                     mov         eax,esp
1595    E8 00 00 00 00            call        CONTROL_GetUserInput_
159A    8B 14 24                  mov         edx,[esp]
159D    39 D3                     cmp         ebx,edx
159F    75 04                     jne         L$172
15A1    89 54 24 0C               mov         0xc[esp],edx
15A5                          L$172:
15A5    8B 4C 24 04               mov         ecx,0x4[esp]
15A9    39 CB                     cmp         ebx,ecx
15AB    75 04                     jne         L$173
15AD    89 4C 24 10               mov         0x10[esp],ecx
15B1                          L$173:
15B1    8B 74 24 0C               mov         esi,0xc[esp]
15B5    39 F3                     cmp         ebx,esi
15B7    75 0C                     jne         L$174
15B9    3B 34 24                  cmp         esi,[esp]
15BC    74 07                     je          L$174
15BE    B8 01 00 00 00            mov         eax,0x00000001
15C3    EB 02                     jmp         L$175
15C5                          L$174:
15C5    89 D8                     mov         eax,ebx
15C7                          L$175:
15C7    8B 6C 24 10               mov         ebp,0x10[esp]
15CB    89 C2                     mov         edx,eax
15CD    39 EB                     cmp         ebx,ebp
15CF    75 0D                     jne         L$176
15D1    3B 6C 24 04               cmp         ebp,0x4[esp]
15D5    74 07                     je          L$176
15D7    B8 01 00 00 00            mov         eax,0x00000001
15DC    EB 02                     jmp         L$177
15DE                          L$176:
15DE    89 D8                     mov         eax,ebx
15E0                          L$177:
15E0    85 D2                     test        edx,edx
15E2    75 04                     jne         L$178
15E4    85 C0                     test        eax,eax
15E6    74 AB                     je          L$171
15E8                          L$178:
15E8    83 C4 18                  add         esp,0x00000018
15EB    5D                        pop         ebp
15EC    5E                        pop         esi
15ED    5A                        pop         edx
15EE    59                        pop         ecx
15EF    5B                        pop         ebx
15F0    C3                        ret         
*/
}

boolean CONTROL_Startup(controltype which, int32 ( *TimeFunction )( void ), int32 ticspersecond)
{
	int32 i;
	char *p;
	
	if (CONTROL_Started) return false;

	if (TimeFunction) GetTime = TimeFunction;
	else GetTime = CONTROL_GetTime;

	ticrate = ticspersecond;

	CONTROL_DoubleClickSpeed = (ticspersecond*57)/100;
	if (CONTROL_DoubleClickSpeed <= 0)
		CONTROL_DoubleClickSpeed = 1;

	if (initinput()) return true;

	CONTROL_MousePresent = false;
	CONTROL_JoyPresent   = false;
	switch (which) {
		case controltype_keyboard:
			break;

		case controltype_keyboardandmouse:
			CONTROL_NumAxes      = MAXMOUSEAXES;
			CONTROL_NumButtons   = MAXMOUSEBUTTONS;
			CONTROL_MousePresent = (inputdevices & 1);
			break;

		case controltype_keyboardandjoystick:
			CONTROL_NumAxes      = joynumaxes; //MAXJOYAXES;
			CONTROL_NumButtons   = joynumbuttons;
			CONTROL_JoyPresent   = ((inputdevices & 3) == 3);
			break;
	}

	CONTROL_MouseEnabled = false;
	CONTROL_JoystickEnabled = false;
	
	if (CONTROL_MousePresent)
		initprintf("CONTROL_Startup: Mouse Present\n");
	if (CONTROL_JoyPresent)
		initprintf("CONTROL_Startup: Joystick Present\n");

	CONTROL_ButtonState1     = 0;
	CONTROL_ButtonState2     = 0;
	CONTROL_ButtonHeldState1 = 0;
	CONTROL_ButtonHeldState2 = 0;

	memset(CONTROL_UserInputCleared, 0, sizeof(CONTROL_UserInputCleared));

	for (i=0; i<CONTROL_NUM_FLAGS; i++)
		CONTROL_Flags[i].used = false;

	CONTROL_Started = true;

	return false;
}

void CONTROL_Shutdown(void)
{
	int i;
	
	if (!CONTROL_Started) return;

	CONTROL_JoyPresent = false;

	MOUSE_Shutdown();

	CONTROL_Started = false;
}

#include "build.h"
#include <stdarg.h>
static int dbgy;
static void db(char *f,...)
{
	char t[256];
	va_list va;
	va_start(va,f);
	vsprintf(t,f,va);
	va_end(va);
	printext256(4,dbgy,128,0,t,0);
	dbgy+=7;
}
void CONTROL_DebugState(void)
{
	int i;
	dbgy=4;
	db("JoyPresent=%d JoyEnabled=%d MousePresent=%d MouseEnabled=%d",
		CONTROL_JoyPresent, CONTROL_JoystickEnabled, CONTROL_MousePresent, CONTROL_MouseEnabled);
	db("ButtonState1=%08x ButtonHeldState1=%08x ButtonState2=%08x ButtonHeldState2=%08x",
		CONTROL_ButtonState1, CONTROL_ButtonHeldState1, CONTROL_ButtonState2, CONTROL_ButtonHeldState2);
	dbgy+=7;
	db("UserInputDelay=%d MouseSensitivity=%d NumButtons=%d NumAxes=%d",
		CONTROL_UserInputDelay, CONTROL_MouseSensitivity, CONTROL_NumButtons, CONTROL_NumAxes);
	for (i=0;i<CONTROL_NUM_FLAGS;i++) {
		if (!CONTROL_Flags[i].used) continue;
		db("Flag%2d active=%d toggle=%d buttonheld=%d cleared=%d",
			i,CONTROL_Flags[i].active,CONTROL_Flags[i].toggle,
			CONTROL_Flags[i].buttonheld,CONTROL_Flags[i].cleared);
	}
}
