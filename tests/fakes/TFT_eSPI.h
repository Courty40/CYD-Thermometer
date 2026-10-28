#pragma once
#include <string>
#include <vector>
#define TL_DATUM 0
#define MC_DATUM 1
#define TC_DATUM 2
#define TR_DATUM 3
#define TFT_BLUE 31
#define TFT_WHITE 65535
static bool fakeSpriteAvailable = true;
static bool fakeDisplayInverted = false;
static int fakeRotation = -1;
static std::vector<std::string> drawn;
class TFT_eSPI {
public:
 void init() {} void setRotation(int value) { fakeRotation=value; } void invertDisplay(bool value) { fakeDisplayInverted=value; }
 template<class... A> void fillScreen(A...) { drawn.clear(); }
 template<class... A> void setTextSize(A...) {}
 template<class... A> void setTextDatum(A...) {}
 template<class... A> void setTextColor(A...) {}
 template<class... A> void fillCircle(A...) {}
 template<class... A> void drawCircle(A...) {}
 template<class... A> void drawFastHLine(A...) {}
 template<class... A> void fillRoundRect(A...) {}
 template<class... A> void drawRoundRect(A...) {}
 template<class... A> void drawLine(A...) {}
 template<class... A> void fillTriangle(A...) {}
 void drawString(const char* s,int,int,int) { drawn.emplace_back(s); }
 int textWidth(const char* s,int) { return std::string(s).size()*24; }
};
class TFT_eSprite : public TFT_eSPI {
public:
 explicit TFT_eSprite(TFT_eSPI*) {}
 void setColorDepth(int) {}
 void* createSprite(int,int) { return fakeSpriteAvailable ? this : nullptr; }
 void pushSprite(int,int) {}
};
