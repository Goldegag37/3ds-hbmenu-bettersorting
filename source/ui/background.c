#include "background.h"

static u32 wifiStatus;
static u8 batteryLevel = 5;
static u8 charging;
static char timeString[9];
static char versionString[64];

#define SECONDS_IN_DAY 86400
#define SECONDS_IN_HOUR 3600
#define SECONDS_IN_MINUTE 60

static bubble_t bubbles[BUBBLE_COUNT];
static float logoPosX, logoPosY;
static ImageId logoImg = imgId_logo;

static const ImageId batteryLevels[] =
{
	imgId_battery0,
	imgId_battery0,
	imgId_battery1,
	imgId_battery2,
	imgId_battery3,
	imgId_battery4,
};

static float randf()
{
	return (float)rand()/(float)RAND_MAX;
}

void backgroundInit(void)
{
	int i = 0;
	for (i = 0; i < BUBBLE_COUNT; i ++)
	{
		bubbles[i].x = rand() % 400;
		bubbles[i].y = 172 + (rand() % 308);
		bubbles[i].z = randf();
		bubbles[i].angle = randf();
		bubbles[i].angv = 0.02f*randf();
		bubbles[i].fade = 15;
	}
	sprintf(versionString, "%s \xEE\x80\x9D %s", launchGetLoader()->name, VERSION);
}

static void bubbleUpdate(bubble_t* bubble)
{
	// Float up the screen.
	bubble->y -= 2;

	// Check if faded away, then reset if gone.
	if (bubble->fade < 10)
	{
		bubble->x = rand() % 400;
		bubble->y = 470 + (rand() % 10);
		bubble->z = randf();
		bubble->angle = randf();
		bubble->angv = 0.02f*randf();
		bubble->fade = 15;
	}
	// Check if too far up screen and start fizzling away.
	else if (bubble->y < 172)
		bubble->fade -= 10;
	// Otherwise make sure the bubble is visible.
	else if (bubble->fade < 255)
		bubble->fade += 10;

	bubble->angle += bubble->angv;
}

static bool checkLogoAdv(u32 down)
{
	static const u32 params[] = { KEY_UP, KEY_UP, KEY_DOWN, KEY_DOWN, KEY_LEFT, KEY_RIGHT, KEY_LEFT, KEY_RIGHT, KEY_SELECT };
	static u32 state, timeout;

	if (down & params[state])
	{
		state++;
		timeout = 30;
		if (state == sizeof(params)/sizeof(params[0]))
		{
			state = 0;
			return true;
		}
	}

	if (timeout && !--timeout)
		state = 0;

	return false;
}

void backgroundUpdate(void)
{
	int i;
	u32 kDown = hidKeysDown();

	if(ACU_GetWifiStatus(&wifiStatus) != 0)
		wifiStatus = 0;
	PTMU_GetBatteryLevel(&batteryLevel);
	PTMU_GetBatteryChargeState(&charging);

	u64 timeInSeconds = osGetTime() / 1000;
	u64 dayTime = timeInSeconds % SECONDS_IN_DAY;
	u8 hour = dayTime / SECONDS_IN_HOUR;
	u8 min = (dayTime % SECONDS_IN_HOUR) / SECONDS_IN_MINUTE;
	u8 seconds = dayTime % SECONDS_IN_MINUTE;
	sprintf(timeString, "%02d:%02d:%02d", hour, min, seconds);

	// Update bubble
	for (i = 0; i < BUBBLE_COUNT; i ++)
		bubbleUpdate(&bubbles[i]);

	// Update logo
	if (logoImg == imgId_logo2)
		logoPosX += 1.0f/64;
	logoPosY -= 1.0f/192;
	if (checkLogoAdv(kDown))
		logoImg = imgId_logo2;
}

void bubbleDraw(bubble_t* bubble, float top, float iod)
{
	if ((bubble->y+32) <= top)
		return; // Nothing to do
	u32 color = ((u32)bubble->fade << 24) | 0xFFFFFF;
	float x = bubble->x + iod*(10+10*bubble->z) + 16*sinf(M_TAU*bubble->angle);
	float y = bubble->y - top;
	if (top > 0.0f)
		x -= (400-320)/2;
	drawingDrawImage(imgId_bubble, color, x, y);
}

void backgroundDrawTop(float iod)
{
	int i;

	drawingSetMode(DRAW_MODE_DRAWING);

	// Clear screen
	drawingSetZ(1.0f);
	drawingEnableDepth(false);
	drawingWithColor(0xFFFF8400);
	drawingDrawQuad(0.0f, 0.0f, 400.0f, 240.0f);
	drawingEnableDepth(true);

	// Draw bubbles!
	drawingSetZ(0.6f);
	for (i = 0; i < BUBBLE_COUNT; i ++)
		bubbleDraw(&bubbles[i], 0.0f, iod);

	// Draw HUD
	drawingSetZ(0.5f);

	textSetColor(0xFFFFFFFF);
	textDrawInBox(timeString, 0, 0.5f, 0.5f, 15.0f, 0.0f, 400.0f);
	textDrawInBox(versionString, 1, 0.5f, 0.5f, 200.0f, 80.0f, 80.0f+271-10);

	float posX = 20.0f*sinf(C3D_Angle(logoPosX));
	float posY =  6.0f*sinf(C3D_Angle(logoPosY));
	drawingDrawImage(logoImg, 0xFFFFFFFF, 80.0f+posX+iod*8, 63.0f+posY);
	drawingDrawImage(wifiStatus ? imgId_wifi3 : imgId_wifiNull, 0xFFFFFFFF, 0.0f, 0.0f);
	drawingDrawImage(charging ? imgId_batteryCharge : batteryLevels[batteryLevel], 0xFFFFFFFF, 400.0f-27, 0.0f);
}

void backgroundDrawBot(void)
{
	int i;

	drawingSetMode(DRAW_MODE_DRAWING);

	// Clear screen
	drawingSetZ(0.8f);
	drawingEnableDepth(false);
	drawingWithColor(0xFFFFA342);
	drawingDrawQuad(0.0f, 0.0f, 320.0f, 240.0f);
	drawingSubmitPrim(GPU_TRIANGLE_STRIP, 4);
	drawingEnableDepth(true);

	// Draw bubbles!
	drawingSetZ(0.6f);
	for (i = 0; i < BUBBLE_COUNT; i ++)
		bubbleDraw(&bubbles[i], 240.0f, 0.0f);
}
