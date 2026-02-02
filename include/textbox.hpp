#pragma once

#include <libdragon.h>
#include <cstdint>
#include <vector>
#include <string>

constexpr int TEXTBOX_MAX_QUEUE = 8;
constexpr int TEXTBOX_MAX_TEXT_LEN = 256;

enum class ETextBoxState
{
    Closed,
    Opening,
    Open,
    Closing
};

enum class ETextBoxAnim
{
    None,
    SlideBottom,
    SlideTop,
    ScaleCenter,
    Fade
};

class CTextBox
{
public:
    CTextBox() = default;
    ~CTextBox();

    void init(int fontId, int x, int y, int width, int height);
    void setSpeed(float charsPerSecond);
    void setBackgroundColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
    void setBackgroundGradient(uint8_t r1, uint8_t g1, uint8_t b1, uint8_t a1,
                               uint8_t r2, uint8_t g2, uint8_t b2, uint8_t a2);
    void setBorderColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
    void setPadding(int padding);
    void setAnimation(ETextBoxAnim style, float duration);
    void setText(const char* text);

    void clear();
    void close();

    bool update(float deltaTime);

    bool handleInput(bool aButtonPressed);

    void draw();

    bool isActive() const { return mState != ETextBoxState::Closed; }
    bool isOpen() const { return mState == ETextBoxState::Open; }
    bool isTextComplete() const { return mCurrentChar >= mCurrentTextLen; }
    bool isAllComplete() const { return mQueueHead == mQueueTail && mState == ETextBoxState::Closed; }

    void setChoices(const std::vector<std::string>& choiceTexts);
    void clearChoices();
    bool hasChoices() const { return !mChoices.empty(); }
    int getSelectedChoice() const { return mConfirmedChoice >= 0 ? mConfirmedChoice : mSelectedChoice; }
    bool handleChoiceInput(joypad_buttons_t btn);

private:
    void advanceToNextText();
    void buildParagraph();
    void startOpenAnimation();
    void startCloseAnimation();
    float easeOutBack(float t);
    float easeInBack(float t);

    int mFontId = 1;
    int mPosX = 20;
    int mPosY = 160;
    int mWidth = 280;
    int mHeight = 60;
    int mPadding = 8;

    color_t mBgColor = {0x20, 0x20, 0x40, 0xE0};
    color_t mBgColor2 = {0x20, 0x20, 0x40, 0xE0};
    color_t mBorderColor = {0xFF, 0xFF, 0xFF, 0xFF};
    bool mUseGradient = false;
    int mGradientSteps = 8;

    ETextBoxAnim mAnimStyle = ETextBoxAnim::SlideBottom;
    float mAnimDuration = 0.3f;
    float mAnimProgress = 0.0f;
    ETextBoxState mState = ETextBoxState::Closed;

    char mTextQueue[TEXTBOX_MAX_QUEUE][TEXTBOX_MAX_TEXT_LEN];
    int mQueueHead = 0;
    int mQueueTail = 0;

    char mCurrentText[TEXTBOX_MAX_TEXT_LEN] = {0};
    int mCurrentTextLen = 0;
    int mCurrentChar = 0;
    float mCharTimer = 0.0f;
    float mCharsPerSecond = 30.0f;

    rdpq_paragraph_t* mParagraph = nullptr;

    bool mNeedsRebuild = false;

    std::vector<std::string> mChoices;
    int mSelectedChoice = 0;
    int mConfirmedChoice = -1;
    float mChoiceOffsetY = 0.0f;
    float mTargetChoiceOffsetY = 0.0f;
};
