#include "textbox.hpp"
#include <cstring>
#include <cmath>

CTextBox::~CTextBox()
{
    if (mParagraph) {
        rdpq_paragraph_free(mParagraph);
        mParagraph = nullptr;
    }
}

void CTextBox::init(int fontId, int x, int y, int width, int height)
{
    mFontId = fontId;
    mPosX = x;
    mPosY = y;
    mWidth = width;
    mHeight = height;
    
    clear();
}

void CTextBox::setSpeed(float charsPerSecond)
{
    mCharsPerSecond = charsPerSecond;
    if (mCharsPerSecond < 1.0f) mCharsPerSecond = 1.0f;
}

void CTextBox::setBackgroundColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    mBgColor = {r, g, b, a};
    mBgColor2 = {r, g, b, a};
    mUseGradient = false;
}

void CTextBox::setBackgroundGradient(uint8_t r1, uint8_t g1, uint8_t b1, uint8_t a1,
                                     uint8_t r2, uint8_t g2, uint8_t b2, uint8_t a2)
{
    mBgColor = {r1, g1, b1, a1};
    mBgColor2 = {r2, g2, b2, a2};
    mUseGradient = true;
}

void CTextBox::setBorderColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    mBorderColor = {r, g, b, a};
}

void CTextBox::setPadding(int padding)
{
    mPadding = padding;
}

void CTextBox::setAnimation(ETextBoxAnim style, float duration)
{
    mAnimStyle = style;
    mAnimDuration = duration;
    if (mAnimDuration < 0.01f) mAnimDuration = 0.01f;
}

float CTextBox::easeOutBack(float t)
{
    const float c1 = 1.70158f;
    const float c3 = c1 + 1.0f;
    float tm1 = t - 1.0f;
    return 1.0f + c3 * tm1 * tm1 * tm1 + c1 * tm1 * tm1;
}

float CTextBox::easeInBack(float t)
{
    const float c1 = 1.70158f;
    const float c3 = c1 + 1.0f;
    return c3 * t * t * t - c1 * t * t;
}

void CTextBox::startOpenAnimation()
{
    mState = ETextBoxState::Opening;
    mAnimProgress = 0.0f;
}

void CTextBox::startCloseAnimation()
{
    mState = ETextBoxState::Closing;
    mAnimProgress = 0.0f;
}

void CTextBox::setText(const char* text)
{
    if (!text) return;
    
    int nextHead = (mQueueHead + 1) % TEXTBOX_MAX_QUEUE;
    if (nextHead == mQueueTail) {
        return;
    }
    
    strncpy(mTextQueue[mQueueHead], text, TEXTBOX_MAX_TEXT_LEN - 1);
    mTextQueue[mQueueHead][TEXTBOX_MAX_TEXT_LEN - 1] = '\0';
    mQueueHead = nextHead;
    
    if (mState == ETextBoxState::Closed) {
        advanceToNextText();
        startOpenAnimation();
    }
}

void CTextBox::clear()
{
    mQueueHead = 0;
    mQueueTail = 0;
    mCurrentChar = 0;
    mCurrentTextLen = 0;
    mCurrentText[0] = '\0';
    mCharTimer = 0.0f;
    mState = ETextBoxState::Closed;
    mAnimProgress = 0.0f;
    mNeedsRebuild = true;
    mChoiceOffsetY = 0.0f;
    mTargetChoiceOffsetY = 0.0f;
    
    if (mParagraph) {
        rdpq_paragraph_free(mParagraph);
        mParagraph = nullptr;
    }
}

void CTextBox::close()
{
    if (mState == ETextBoxState::Open || mState == ETextBoxState::Opening) {
        startCloseAnimation();
    }
}

void CTextBox::advanceToNextText()
{
    if (mQueueTail == mQueueHead) {
        startCloseAnimation();
        return;
    }
    
    strcpy(mCurrentText, mTextQueue[mQueueTail]);
    mCurrentTextLen = strlen(mCurrentText);
    mQueueTail = (mQueueTail + 1) % TEXTBOX_MAX_QUEUE;
    
    mCurrentChar = 0;
    mCharTimer = 0.0f;
    mNeedsRebuild = true;
}

bool CTextBox::update(float deltaTime)
{
    switch (mState) {
        case ETextBoxState::Closed:
            return false;
            
        case ETextBoxState::Opening:
            mAnimProgress += deltaTime / mAnimDuration;
            if (mAnimProgress >= 1.0f) {
                mAnimProgress = 1.0f;
                mState = ETextBoxState::Open;
            }
            break;
            
        case ETextBoxState::Closing:
            mAnimProgress += deltaTime / mAnimDuration;
            if (mAnimProgress >= 1.0f) {
                mAnimProgress = 1.0f;
                mState = ETextBoxState::Closed;
                mCurrentText[0] = '\0';
                mCurrentTextLen = 0;
                mCurrentChar = 0;
                return false;
            }
            break;
            
        case ETextBoxState::Open:
            if (mCurrentChar < mCurrentTextLen) {
                mCharTimer += deltaTime * mCharsPerSecond;
                
                int prevChar = mCurrentChar;
                while (mCharTimer >= 1.0f && mCurrentChar < mCurrentTextLen) {
                    mCharTimer -= 1.0f;
                    mCurrentChar++;
                }
                
                if (mCurrentChar != prevChar) {
                    mNeedsRebuild = true;
                }
            }
            break;
    }
    
    if (mChoiceOffsetY != mTargetChoiceOffsetY) {
        float offsetSpeed = 300.0f;
        float diff = mTargetChoiceOffsetY - mChoiceOffsetY;
        if (fabs(diff) < offsetSpeed * deltaTime) {
            mChoiceOffsetY = mTargetChoiceOffsetY;
        } else {
            mChoiceOffsetY += (diff > 0 ? 1.0f : -1.0f) * offsetSpeed * deltaTime;
        }
    }
    
    return mState != ETextBoxState::Closed;
}

bool CTextBox::handleInput(bool aButtonPressed)
{
    if (mState == ETextBoxState::Closed) return false;
    
    if (mState == ETextBoxState::Opening || mState == ETextBoxState::Closing) {
        return true;
    }
    
    if (aButtonPressed && mState == ETextBoxState::Open) {
        if (mCurrentChar < mCurrentTextLen) {
            mCurrentChar = mCurrentTextLen;
            mNeedsRebuild = true;
        } else {
            advanceToNextText();
        }
    }
    
    return mState != ETextBoxState::Closed;
}

void CTextBox::buildParagraph()
{
    if (mParagraph) {
        rdpq_paragraph_free(mParagraph);
        mParagraph = nullptr;
    }
    
    if (mCurrentChar <= 0) return;
    
    int nbytes = mCurrentChar;
    rdpq_textparms_t parms = {};
    parms.width = (int16_t)(mWidth - mPadding * 2);
    parms.height = (int16_t)(mHeight - mPadding * 2);
    parms.align = ALIGN_LEFT;
    parms.valign = VALIGN_TOP;
    parms.wrap = WRAP_WORD;
    
    mParagraph = rdpq_paragraph_build(&parms, mFontId, mCurrentText, &nbytes);
}

void CTextBox::draw()
{
    if (mState == ETextBoxState::Closed) return;
    
    if (mNeedsRebuild) {
        buildParagraph();
        mNeedsRebuild = false;
    }
    
    float animT = 0.0f;
    if (mState == ETextBoxState::Opening) {
        animT = easeOutBack(mAnimProgress);
    } else if (mState == ETextBoxState::Closing) {
        animT = 1.0f - easeInBack(mAnimProgress);
    } else {
        animT = 1.0f;
    }
    
    int drawX = mPosX;
    int drawY = mPosY;
    int drawW = mWidth;
    int drawH = mHeight;
    uint8_t alpha = mBgColor.a;
    
    switch (mAnimStyle) {
        case ETextBoxAnim::None:
            break;
            
        case ETextBoxAnim::SlideBottom:
            drawY = mPosY + (int)((1.0f - animT) * (240 - mPosY + mHeight));
            break;
            
        case ETextBoxAnim::SlideTop:
            drawY = mPosY - (int)((1.0f - animT) * (mPosY + mHeight));
            break;
            
        case ETextBoxAnim::ScaleCenter:
            {
                int centerX = mPosX + mWidth / 2;
                int centerY = mPosY + mHeight / 2;
                drawW = (int)(mWidth * animT);
                drawH = (int)(mHeight * animT);
                drawX = centerX - drawW / 2;
                drawY = centerY - drawH / 2;
                if (drawW < 4) drawW = 4;
                if (drawH < 4) drawH = 4;
            }
            break;
            
        case ETextBoxAnim::Fade:
            alpha = (uint8_t)(mBgColor.a * animT);
            break;
    }
    
    drawY += (int)mChoiceOffsetY;
    
    if (drawW <= 0 || drawH <= 0) return;
    
    rdpq_sync_pipe();
    rdpq_mode_push();
    rdpq_set_mode_standard();
    rdpq_mode_combiner(RDPQ_COMBINER_FLAT);
    rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
    
    if (mUseGradient && drawH > mGradientSteps) {
        float stripHeight = (float)drawH / mGradientSteps;
        for (int i = 0; i < mGradientSteps; ++i) {
            float t = (float)i / (mGradientSteps - 1);
            
            uint8_t r = (uint8_t)(mBgColor.r + (mBgColor2.r - mBgColor.r) * t);
            uint8_t g = (uint8_t)(mBgColor.g + (mBgColor2.g - mBgColor.g) * t);
            uint8_t b = (uint8_t)(mBgColor.b + (mBgColor2.b - mBgColor.b) * t);
            uint8_t a = (uint8_t)((mBgColor.a + (mBgColor2.a - mBgColor.a) * t) * animT);
            
            color_t stripColor = {r, g, b, a};
            rdpq_set_prim_color(stripColor);
            
            int y1 = drawY + (int)(i * stripHeight);
            int y2 = drawY + (int)((i + 1) * stripHeight);
            if (i == mGradientSteps - 1) y2 = drawY + drawH;
            
            rdpq_fill_rectangle(drawX, y1, drawX + drawW, y2);
        }
    } else {
        color_t bgDraw = {mBgColor.r, mBgColor.g, mBgColor.b, alpha};
        rdpq_set_prim_color(bgDraw);
        rdpq_fill_rectangle(drawX, drawY, drawX + drawW, drawY + drawH);
    }
    
    uint8_t borderAlpha = (uint8_t)(mBorderColor.a * animT);
    color_t borderDraw = {mBorderColor.r, mBorderColor.g, mBorderColor.b, borderAlpha};
    rdpq_set_prim_color(borderDraw);
    int borderWidth = 2;
    rdpq_fill_rectangle(drawX, drawY, drawX + drawW, drawY + borderWidth);
    rdpq_fill_rectangle(drawX, drawY + drawH - borderWidth, drawX + drawW, drawY + drawH);
    rdpq_fill_rectangle(drawX, drawY, drawX + borderWidth, drawY + drawH);
    rdpq_fill_rectangle(drawX + drawW - borderWidth, drawY, drawX + drawW, drawY + drawH);
    
    if (mParagraph && animT > 0.8f) {
        float textAlpha = (animT - 0.8f) / 0.2f;
        if (textAlpha > 1.0f) textAlpha = 1.0f;
        
        int textX = drawX + mPadding;
        int textY = drawY + mPadding;
        
        if (mAnimStyle == ETextBoxAnim::ScaleCenter) {
            float scaleRatio = (float)drawW / mWidth;
            textX = drawX + (int)(mPadding * scaleRatio);
            textY = drawY + (int)(mPadding * scaleRatio);
        }
        
        rdpq_paragraph_render(mParagraph, textX, textY);
    }
    
    if (isTextComplete() && mState == ETextBoxState::Open) {
        static float blinkTimer = 0.0f;
        blinkTimer += 0.1f;
        if (fmodf(blinkTimer, 1.0f) < 0.5f) {
            int indicatorX = drawX + drawW - mPadding - 8;
            int indicatorY = drawY + drawH - mPadding - 8;
            
            rdpq_set_prim_color(mBorderColor);
            rdpq_fill_rectangle(indicatorX, indicatorY, indicatorX + 6, indicatorY + 2);
            rdpq_fill_rectangle(indicatorX + 1, indicatorY + 2, indicatorX + 5, indicatorY + 4);
            rdpq_fill_rectangle(indicatorX + 2, indicatorY + 4, indicatorX + 4, indicatorY + 6);
        }
    }
    
    if (hasChoices() && isTextComplete() && mState == ETextBoxState::Open) {
        int choiceY = drawY + drawH + 6;
        int choiceHeight = 22;
        int choiceSpacing = 3;
        
        for (size_t i = 0; i < mChoices.size(); ++i) {
            bool isSelected = ((int)i == mSelectedChoice);
            
            rdpq_set_mode_standard();
            rdpq_mode_combiner(RDPQ_COMBINER_FLAT);
            rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
            
            color_t choiceBg = isSelected ? 
                color_t{0x60, 0x60, 0xA0, 0xF0} :
                color_t{0x38, 0x38, 0x58, 0xE0};
            rdpq_set_prim_color(choiceBg);
            rdpq_fill_rectangle(drawX, choiceY, drawX + drawW, choiceY + choiceHeight);
            
            color_t borderColor = isSelected ? 
                color_t{0xFF, 0xFF, 0xFF, 0xFF} :
                color_t{0xA0, 0xA0, 0xC0, 0xFF};
            rdpq_set_prim_color(borderColor);
            
            int borderThickness = isSelected ? 2 : 1;
            
            rdpq_fill_rectangle(drawX, choiceY, drawX + drawW, choiceY + borderThickness);
            rdpq_fill_rectangle(drawX, choiceY + choiceHeight - borderThickness, drawX + drawW, choiceY + choiceHeight);
            rdpq_fill_rectangle(drawX, choiceY, drawX + borderThickness, choiceY + choiceHeight);
            rdpq_fill_rectangle(drawX + drawW - borderThickness, choiceY, drawX + drawW, choiceY + choiceHeight);
            
            rdpq_textparms_t textParms = {0};
            textParms.width = (int16_t)(drawW - mPadding * 2);
            int textY = choiceY + 14;
            rdpq_text_printf(&textParms, mFontId, drawX + mPadding + 2, textY, "%s", mChoices[i].c_str());
            
            choiceY += choiceHeight + choiceSpacing;
        }
    }
    
    rdpq_mode_pop();
}

void CTextBox::setChoices(const std::vector<std::string>& choiceTexts)
{
    mChoices = choiceTexts;
    mSelectedChoice = 0;
    mConfirmedChoice = -1;
    
    if (!mChoices.empty()) {
        int choiceHeight = 22;
        int choiceSpacing = 3;
        int topMargin = 6;
        int totalChoiceHeight = topMargin + (choiceHeight + choiceSpacing) * (int)mChoices.size();
        int bottomY = mPosY + mHeight + totalChoiceHeight;
        if (bottomY > 235) {
            mTargetChoiceOffsetY = -(float)(bottomY - 235);
        } else {
            mTargetChoiceOffsetY = 0.0f;
        }
    } else {
        mTargetChoiceOffsetY = 0.0f;
    }
}

void CTextBox::clearChoices()
{
    mChoices.clear();
    mSelectedChoice = 0;
    mConfirmedChoice = -1;
    mTargetChoiceOffsetY = 0.0f;
}

bool CTextBox::handleChoiceInput(joypad_buttons_t btn)
{
    if (!hasChoices() || !isTextComplete() || mState != ETextBoxState::Open) {
        return false;
    }
    
    if (btn.d_up && mSelectedChoice > 0) {
        mSelectedChoice--;
    }
    if (btn.d_down && mSelectedChoice < (int)mChoices.size() - 1) {
        mSelectedChoice++;
    }
    
    if (btn.a) {
        mConfirmedChoice = mSelectedChoice;
        mChoices.clear();
        mSelectedChoice = 0;
        mTargetChoiceOffsetY = 0.0f;
        return true;
    }
    
    return false;
}
