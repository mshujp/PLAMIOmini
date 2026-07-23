#include "lgfx_fonts.hpp"

#include "platforms/common.hpp"
#include "misc/pixelcopy.hpp"
#include "LGFXBase.hpp"

#include <stdint.h>
#include <stddef.h>
#include <math.h>
#include <string.h>
#include <vector>
#include "../internal/algorithm.h"

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif
#ifdef abs
#undef abs
#endif

namespace lgfx
{
 inline namespace v1
 {
  struct glcd_fontinfo_t
  {
    uint8_t start;
    uint8_t end;
    uint8_t datawidth;
  };

  size_t IFont::drawCharDummy(LGFXBase* gfx, int32_t x, int32_t y, int32_t w, int32_t h, const TextStyle* style, int32_t& filled_x) const
  {
    int32_t sx = 65536 * style->size_x;
    w = (w * sx) >> 16;
    int32_t sy = 65536 * style->size_y;
    h = (h * sy) >> 16;
    gfx->startWrite();
    if (style->fore_rgb888 != style->back_rgb888)
    {
      gfx->fillRect(x, y, w, h, style->back_rgb888);
      filled_x = x + w;
    }
    if (2 < w && 2 < h)
    {
      gfx->drawRect(x+1, y+1, w-2, h-2, style->fore_rgb888);
    }
    gfx->endWrite();
    return w;
  }

  void BaseFont::getDefaultMetric(FontMetrics *metrics) const
  {
    metrics->width    = width;
    metrics->x_advance = width;
    metrics->x_offset  = 0;
    metrics->height    = height;
    metrics->y_advance = height;
    metrics->y_offset  = 0;
    metrics->baseline  = baseline;
  }
  void BDFfont::getDefaultMetric(FontMetrics *metrics) const
  {
    BaseFont::getDefaultMetric(metrics);
    metrics->y_advance = y_advance;
  }

  bool GLCDfont::updateFontMetric(FontMetrics*, uint16_t uniCode) const {
    auto info = reinterpret_cast<const glcd_fontinfo_t*>(widthtbl);
    return info->start <= uniCode && uniCode <= info->end;
  }

  size_t GLCDfont::drawChar(LGFXBase* gfx, int32_t x, int32_t y, uint16_t c, const TextStyle* style, FontMetrics* metrics, int32_t& filled_x) const
  {
    (void)metrics;
    auto info = reinterpret_cast<const glcd_fontinfo_t*>(widthtbl);
    if (c < pgm_read_byte(&info->start) || pgm_read_byte(&info->end) < c)
    {
      return drawCharDummy(gfx, x, y, this->width, this->height, style, filled_x);
    }
    if (!style->cp437 && (c >= 176))
    {
      c++; // Handle 'classic' charset behavior
    }

    c -= pgm_read_byte(&info->start);

    const int32_t fontWidth  = this->width;
    const int32_t fontHeight = this->height;

    const auto datawidth = pgm_read_byte(&info->datawidth);
    auto font_addr = this->chartbl + (c * datawidth);
    auto cc = gfx->getColorConverter();
    bool fillbg = (style->back_rgb888 != style->fore_rgb888);
    uint32_t colortbl[2] = { cc->convert(style->back_rgb888), cc->convert(style->fore_rgb888) };

    const int32_t sy = 65536 * style->size_y;
    const int32_t sx = 65536 * style->size_x;

    gfx->startWrite();

    uint32_t x1 = 0;
    int_fast8_t i = 0;
    do
    {
      uint_fast8_t line = pgm_read_byte(&font_addr[i]);
      uint_fast8_t flg = (line & 0x01);
      int_fast8_t j = 1;
      uint32_t y1 = 0;
      uint32_t x0 = x1;
      x1 = (++i * sx) >> 16;
      uint32_t w = x1 - x0;
      do
      {
        while (flg == ((line >> j) & 0x01) && ++j < fontHeight);
        uint32_t y0 = y1;
        y1 = (j * sy) >> 16;
        if (flg || fillbg)
        {
          gfx->setRawColor(colortbl[flg]);
          gfx->writeFillRect(x, y + y0, w, y1 - y0);
        }
        flg = !flg;
      } while (j < fontHeight);
      x += w;
    } while (i < datawidth);

    uint32_t x2 = (fontWidth * sx) >> 16;
    if (fillbg && datawidth < fontWidth)
    {
      gfx->setRawColor(colortbl[0]);
      gfx->writeFillRect(x, y, x2 - x1, (fontHeight * sy) >> 16);
    }
    gfx->endWrite();

    return x2;
  }

  static size_t draw_char_bmp(LGFXBase* gfx, int32_t x, int32_t y, const TextStyle* style, const uint8_t* font_addr, int_fast8_t fontWidth, int_fast8_t fontHeight, int_fast8_t w, int_fast8_t margin)
  {
    uint32_t colortbl[2] = {gfx->getColorConverter()->convert(style->back_rgb888), gfx->getColorConverter()->convert(style->fore_rgb888)};
    bool fillbg = (style->back_rgb888 != style->fore_rgb888);

    //int32_t clip_left   = gfx->_clip_l;
    //int32_t clip_right  = gfx->_clip_r;
    //int32_t clip_top    = gfx->_clip_t;
    //int32_t clip_bottom = gfx->_clip_b;

    int32_t sx = 65536 * style->size_x;
    int32_t sy = 65536 * style->size_y;

    //if ((x <= clip_right) && (clip_left < (x + fontWidth * sx ))
    // && (y <= clip_bottom) && (clip_top < (y + sh )))
    {
      gfx->startWrite();
      if (fillbg && margin)
      {
        int32_t x0 = ((fontWidth - margin) * sx) >> 16;
        int32_t x1 = ((fontWidth         ) * sx) >> 16;
        if (x0 < x1) {
          gfx->setRawColor(colortbl[0]);
          gfx->writeFillRect(x + x0, y, x1 - x0, (fontHeight * sy) >> 16);
        }
      }
      int32_t i = 0;
      int32_t y1 = 0;
      int32_t y0 = - 1;
      int32_t height = (sy * fontHeight) >> 16;
      do {
        bool fill = y0 != y1;
        y0 = y1;
        y1 = (++i * sy) >> 16;
        int32_t h = (y1 < height && y0 == y1) ? 1 : (y1 - y0);
        uint8_t line = pgm_read_byte(&font_addr[0]);
        bool flg = line & 0x80;
        int_fast8_t j = 1;
        int_fast8_t je = fontWidth - margin;
        int32_t x0 = 0;
        do {
          do {
            if (0 == (j & 7)) line = pgm_read_byte(&font_addr[j >> 3]);
          } while (flg == (bool)(line & (0x80) >> (j&7)) && ++j < je);
          int32_t x1 = (j * sx) >> 16;
          if (flg || (fillbg && fill)) {
            gfx->setRawColor(colortbl[flg]);
            if (flg && x1 == x0) ++x1;
            gfx->writeFillRect(x + x0, y + y0, x1 - x0, h);
          }
          x0 = x1;
          flg = !flg;
        } while (j < je);
        font_addr += w;
      } while (i < fontHeight);
      gfx->endWrite();
    }

    return fontWidth * sx >> 16;
  }


  bool FixedBMPfont::updateFontMetric(FontMetrics *metrics, uint16_t uniCode) const {
    metrics->x_advance = metrics->width = this->width;
    auto info = reinterpret_cast<const glcd_fontinfo_t*>(widthtbl);
    return info->start <= uniCode && uniCode <= info->end;
  }

  bool BMPfont::updateFontMetric(FontMetrics *metrics, uint16_t uniCode) const {
    bool res = ((uniCode -= 0x20u) < 0x60u);
    if (!res) uniCode = 0;
    metrics->x_advance = metrics->width = pgm_read_byte(&this->widthtbl[uniCode]);
    return res;
  }

  bool BDFfont::updateFontMetric(FontMetrics *metrics, uint16_t uniCode) const {
    metrics->x_advance = metrics->width = (uniCode < 0x0100) ? halfwidth : width;
    return true;
  }

  size_t FixedBMPfont::drawChar(LGFXBase* gfx, int32_t x, int32_t y, uint16_t uniCode, const TextStyle* style, FontMetrics* metrics, int32_t& filled_x) const
  { // BMP font
    (void)metrics;
    const int_fast16_t fontHeight = this->height;

    auto info = reinterpret_cast<const glcd_fontinfo_t*>(widthtbl);
    if (info->start > uniCode || uniCode > info->end) { return drawCharDummy(gfx, x, y, width, fontHeight, style, filled_x); }

    if (!style->cp437 && (uniCode >= 176))
    {
      uniCode++; // Handle 'classic' charset behavior
    }
    uniCode -= info->start;
    int_fast8_t w = (width + 7) >> 3;
    auto font_addr = (const uint8_t*) &chartbl[uniCode * w * fontHeight];
    return draw_char_bmp(gfx, x, y, style, font_addr, width, fontHeight, (width + 7) >> 3, 0);
  }

  size_t BMPfont::drawChar(LGFXBase* gfx, int32_t x, int32_t y, uint16_t uniCode, const TextStyle* style, FontMetrics* metrics, int32_t& filled_x) const
  { // BMP font
    (void)metrics;
    if ((uniCode -= 0x20u) >= 0x60u) return drawCharDummy(gfx, x, y, this->widthtbl[0], this->height, style, filled_x);
    const int_fast8_t fontWidth = pgm_read_byte(&this->widthtbl[uniCode]);
    const int_fast8_t fontHeight = this->height;

    auto font_addr = ((const uint8_t**)this->chartbl)[uniCode];
    return draw_char_bmp(gfx, x, y, style, font_addr, fontWidth, fontHeight, (fontWidth + 6) >> 3, 1);
  }

  size_t BDFfont::drawChar(LGFXBase* gfx, int32_t x, int32_t y, uint16_t c, const TextStyle* style, FontMetrics* metrics, int32_t& filled_x) const
  {
    (void)metrics;
    const int_fast8_t bytesize = (this->width + 7) >> 3;
    const int_fast8_t fontHeight = this->height;
    const int_fast8_t fontWidth = (c < 0x0100) ? this->halfwidth : this->width;
    auto it = std::lower_bound(this->indextbl, &this->indextbl[this->indexsize], c);
    if (*it != c) return drawCharDummy(gfx, x, y, fontWidth, fontHeight, style, filled_x);

    const uint8_t* font_addr = &this->chartbl[std::distance(this->indextbl, it) * fontHeight * bytesize];
    return draw_char_bmp(gfx, x, y, style, font_addr, fontWidth, fontHeight, bytesize, 0);
  }

  size_t RLEfont::drawChar(LGFXBase* gfx, int32_t x, int32_t y, uint16_t code, const TextStyle* style, FontMetrics* metrics, int32_t& filled_x) const
  { // RLE font
    (void)metrics;
    if ((code -= 0x20u) >= 0x60u) return drawCharDummy(gfx, x, y, this->widthtbl[0], this->height, style, filled_x);

    const uint_fast16_t fontWidth = pgm_read_byte(&this->widthtbl[code]);
    const uint_fast16_t fontHeight = this->height;

    auto font_addr = ((const uint8_t**)this->chartbl)[code];

    uint32_t colortbl[2] = {gfx->getColorConverter()->convert(style->back_rgb888), gfx->getColorConverter()->convert(style->fore_rgb888)};
    bool fillbg = (style->back_rgb888 != style->fore_rgb888);

    //int32_t clip_left   = gfx->_clip_l;
    //int32_t clip_right  = gfx->_clip_r;
    //int32_t clip_top    = gfx->_clip_t;
    //int32_t clip_bottom = gfx->_clip_b;

    int32_t sx = 65536 * style->size_x;
    int32_t sy = 65536 * style->size_y;

    //if ((x <= clip_right) && (clip_left < (x + fontWidth  * sx ))
    // && (y <= clip_bottom) && (clip_top < (y + fontHeight * sy )))
    {
      bool flg = false;
      uint_fast8_t line = 0, i = 1, j = 0;
      int32_t len;
      int32_t y0 = 0;
      int32_t y1 = sy >> 16;
      int32_t x0 = 0;
      gfx->startWrite();
      do {
        line = pgm_read_byte(font_addr++);
        flg = line & 0x80;
        line = (line & 0x7F)+1;
        do {
          len = (line > fontWidth - j) ? fontWidth - j : line;
          line -= len;
          j += len;
          int32_t x1 = (j * sx) >> 16;
          if (fillbg || flg) {
            gfx->setRawColor(colortbl[flg]);
            gfx->writeFillRect( x + x0, y + y0, x1 - x0, y1 - y0);
          }
          x0 = x1;
          if (j == fontWidth)
          {
            j = 0;
            x0 = 0;
            y0 = y1;
            y1 = (++i * sy) >> 16;
          }
        } while (line);
      } while (i <= fontHeight);
      gfx->endWrite();
    }

    return fontWidth * sx >> 16;
  }


//----------------------------------------------------------------------------

  bool GFXfont::updateFontMetric(lgfx::FontMetrics *metrics, uint16_t uniCode) const
  {
    auto glyph_ = getGlyph(uniCode);
    bool res = glyph_;
    if (!res)
    {
      glyph_ = getGlyph(0x20);
      if (!glyph_)
      {
        metrics->x_offset = 0;
        metrics->width = metrics->x_advance = pgm_read_byte(&this->yAdvance) >> 1;
        return false;
      }
    }
    metrics->x_offset  = (int8_t)pgm_read_byte(&glyph_->xOffset);
    metrics->width     = pgm_read_byte(&glyph_->width);
    metrics->x_advance = pgm_read_byte(&glyph_->xAdvance);
    return res;
  }

  GFXglyph* GFXfont::getGlyph(uint16_t uniCode) const
  {
    auto f = pgm_read_word(&first);
    if (uniCode > pgm_read_word(&last)
    ||  uniCode < f) return nullptr;
    uint_fast16_t custom_range_num = pgm_read_word_unaligned(&range_num);
    if (custom_range_num == 0) {
      uniCode -= f;
      return &(((GFXglyph*)pgm_read_ptr( &glyph ))[uniCode]);
    }
    auto range_pst = range;
    size_t i = 0;
    while ((uniCode > pgm_read_word(&range_pst[i].end))
        || (uniCode < pgm_read_word(&range_pst[i].start))) {
      if (++i == custom_range_num) return nullptr;
    }
    uniCode -= pgm_read_word(&range_pst[i].start) - pgm_read_word(&range_pst[i].base);
    return &(((GFXglyph*)pgm_read_ptr( &glyph ))[uniCode]);
  }

  void GFXfont::getDefaultMetric(lgfx::FontMetrics *metrics) const
  {
    int_fast8_t glyph_ab = 0;   // glyph delta Y (height) above baseline
    int_fast8_t glyph_bb = 0;   // glyph delta Y (height) below baseline
    size_t numChars = pgm_read_word(&last) - pgm_read_word(&first);

    size_t custom_range_num = pgm_read_word_unaligned(&range_num);
    if (custom_range_num != 0) {
      EncodeRange *range_pst = range;
      size_t i = 0;
      numChars = custom_range_num;
      do {
        numChars += pgm_read_word(& range_pst[i].end) - pgm_read_word(& range_pst[i].start);
      } while (++i < custom_range_num);
    }

    // Find the biggest above and below baseline offsets
    size_t c = 0;
    do
    {
      GFXglyph* glyph1 = &(((GFXglyph*)pgm_read_ptr( &glyph ))[c]);
      int_fast8_t ab = - (int8_t)(pgm_read_byte(& (glyph1->yOffset)));
      if (ab > glyph_ab) glyph_ab = ab;
      int_fast8_t bb = pgm_read_byte(& glyph1->height) - ab;
      if (bb > glyph_bb) glyph_bb = bb;
    } while ( ++c < numChars );

    metrics->baseline = glyph_ab;
    metrics->y_offset = - glyph_ab;
    metrics->height   = glyph_bb + glyph_ab;
    metrics->y_advance = pgm_read_byte(& yAdvance);
  }

  size_t GFXfont::drawChar(LGFXBase* gfx, int32_t x, int32_t y, uint16_t uniCode, const TextStyle* style, FontMetrics* metrics, int32_t& filled_x) const
  {
    int32_t sy = 65536 * style->size_y;
    y += (metrics->y_offset * sy) >> 16;
    auto glyph_ = this->getGlyph(uniCode);
    if (!glyph_)
    {
      glyph_ = this->getGlyph(0x20);
      if (glyph_) return drawCharDummy(gfx, x, y, pgm_read_byte(&glyph_->xAdvance), metrics->height, style, filled_x);
      return 0;
    }

    int32_t w = pgm_read_byte(&glyph_->width);
    int32_t h = pgm_read_byte(&glyph_->height);

    int32_t sx = 65536 * style->size_x;

    int32_t xAdvance = sx * pgm_read_byte(&glyph_->xAdvance) >> 16;
    int32_t xoffset  = sx * ((int8_t)pgm_read_byte(&glyph_->xOffset)) >> 16;

    uint32_t colortbl[2] = {gfx->getColorConverter()->convert(style->back_rgb888), gfx->getColorConverter()->convert(style->fore_rgb888)};
    bool fillbg = (style->back_rgb888 != style->fore_rgb888);
    int32_t left  = 0;
    int32_t right = 0;
    if (fillbg) {
      left  = std::max<int>(filled_x, x + (xoffset < 0 ? xoffset : 0));
      right = x + std::max<int>((w * sx >> 16) + xoffset, xAdvance);
      filled_x = right;
      gfx->setRawColor(colortbl[0]);
    }

    x += xoffset;
    int32_t yoffset = (- metrics->y_offset) + (int8_t)pgm_read_byte(&glyph_->yOffset);

    gfx->startWrite();

    if (left < right) {
      if (yoffset > 0) {
        gfx->writeFillRect(left, y, right - left, (yoffset * sy) >> 16);
      }
      int32_t y0 = ((yoffset + h)   * sy) >> 16;
      int32_t y1 = (metrics->height * sy) >> 16;
      if (y0 < y1) {
        gfx->writeFillRect(left, y + y0, right - left, y1 - y0);
      }
    }

    if (h)
    {
      uint8_t *bitmap_ = &this->bitmap[pgm_read_dword(&glyph_->bitmapOffset)];
      uint_fast8_t mask = 0x80;
      int32_t btmp = pgm_read_byte(bitmap_); /// btmp邵ｺ・ｮ隴崢闕ｳ雍具ｽｽ髦ｪ繝ｳ郢昴・繝ｨ (髫ｨ・ｦ陷ｿ・ｷ郢晁侭繝｣郢昴・ 郢ｧ蛛ｵ繝ｵ郢晢ｽｩ郢ｧ・ｰ邵ｺ・ｨ邵ｺ蜉ｱ窶ｻ隰・ｽｱ邵ｺ繝ｻ笳・ｹｧ竏ｵ隕・ｸｺ蛹ｻ窶ｻ uint邵ｺ・ｫ邵ｺ蜉ｱ竊醍ｸｺ繝ｻ;
      if (btmp & mask) { btmp = ~btmp; }
      uint32_t bitlen = 0;

      gfx->setRawColor(colortbl[1]);
      uint32_t limit_width = ( w            * sx) >> 16;
      int32_t limit_height = ((h + yoffset) * sy) >> 16;
      int32_t y1 = (yoffset * sy) >> 16;
      int32_t y0 = y1 - 1;
      int32_t i = 0;
      do {
        bool fill = y0 != y1;
        y0 = y1;
        y1 = ((++i + yoffset) * sy) >> 16;
        int32_t fh = (y1 < limit_height && y1 == y0) ? 1 : (y1 - y0);

        if (left < right && fill) {
          gfx->setRawColor(colortbl[0]);
          gfx->writeFillRect(left, y + y0, right - left, fh);
          gfx->setRawColor(colortbl[1]);
        }
        uint32_t j = 0;
        uint32_t x0 = 0;
        uint32_t remain = w;
        do
        {
          if (bitlen == 0)
          {
            btmp = ~btmp;
            do
            { /// 郢晁侭繝｣郢晉｣ｯﾂ・｣驍ｯ螢ｽ辟夂ｹｧ雋槫徐陟募干笘・ｹｧ荵斟晉ｹ晢ｽｼ郢昴・
              do
              {
                ++bitlen;

                /// 1Byte邵ｺ・ｶ郢ｧ讌｢・ｵ・ｰ隴滂ｽｻ邵ｺ・ｧ邵ｺ髦ｪ笳・ｹｧ逕ｻ・ｬ・｡邵ｺ・ｮ郢昴・繝ｻ郢ｧ・ｿ郢ｧ雋槫徐陟募干笘・ｹｧ荵敖繝ｻ
                if (0 == (mask >>= 1))
                {
                  goto label_nextbyte;
/// goto郢ｧ蜑・ｽｽ・ｿ騾包ｽｨ邵ｺ蜉ｱ窶ｻ郢晢ｽｫ郢晢ｽｼ郢晄懶ｽ､謔ｶ竊楢怎・ｺ郢ｧ迢礼ｊ騾包ｽｱ邵ｺ・ｯ鬨ｾ貅ｷ・ｺ・ｦ陷ｷ蜿ｰ・ｸ鄙ｫ繝ｻ邵ｺ貅假ｽ∫ｸｲ繧仰・｣驍ｯ螢ｹﾎ晉ｹ晢ｽｼ郢晉軸蜃ｾ邵ｺ・ｫ郢晢ｽｫ郢晢ｽｼ郢晄懊・邵ｺ・ｮ陷・ｽｦ騾・・・帝￥・ｭ邵ｺ荳岩・郢ｧ蜿･譟題ｭｫ諛岩ｲ邵ｺ繧・ｽ・
/// 邵ｲ蠕後Ι郢晢ｽｼ郢ｧ・ｿ陷ｿ髢・ｾ蜉ｱ窶ｲ陟｢繝ｻ・ｦ竏壺・陜｣・ｴ陷ｷ蛹ｻﾂ髦ｪ竊堵oto郢ｧ・ｸ郢晢ｽ｣郢晢ｽｳ郢晏干・・ｸｺ蟶呻ｽ狗ｸｺ阮吮・邵ｺ・ｫ郢ｧ蛹ｻ・顔ｸｲ竏堋蠕後Ι郢晢ｽｼ郢ｧ・ｿ陷ｿ髢・ｾ蜉ｱ窶ｲ闕ｳ蟠趣ｽｦ竏壺・陜｣・ｴ陷ｷ蛹ｻﾂ髦ｪ繝ｻ郢ｧ・ｸ郢晢ｽ｣郢晢ｽｳ郢晏干窶ｲ闕ｳ蟠趣ｽｦ竏壺・邵ｺ・ｪ郢ｧ荵敖繝ｻ
                }
              } while (btmp & mask);
              break; /// 郢晁侭繝｣郢晏現窶ｲ鬨ｾ豕後・郢ｧ蠕娯螺陜｣・ｴ陷ｷ蛹ｻ繝ｻ郢晢ｽｫ郢晢ｽｼ郢晏干・定ｬ壽㈱・郢ｧ繝ｻ

label_nextbyte: /// 隹ｺ・｡邵ｺ・ｮ郢昴・繝ｻ郢ｧ・ｿ郢ｧ雋槫徐陟募干笘・ｹｧ繝ｻ
              mask = 0x80;
              btmp = pgm_read_byte(++bitmap_) ^ (btmp < 0 ? ~0 : 0);
            } while (btmp & mask);
          }

          uint32_t l = std::min(bitlen, remain);
          remain -= l;
          bitlen -= l;
          j += l;
          uint32_t x1 = (j * sx) >> 16;
          if (btmp >= 0) {
            uint32_t fw = (x1 < limit_width && x1 == x0) ? 1 : (x1 - x0);
            gfx->writeFillRect(x + x0, y + y0, fw, fh);
          }
          x0 = x1;
        } while (remain);
      } while (i < h);
    }
    gfx->endWrite();
    return xAdvance;
  }

//----------------------------------------------------------------------------

  struct u8g2_font_decode_t
  {
    u8g2_font_decode_t(const uint8_t* ptr) : decode_ptr(ptr), decode_bit_pos(0) {}

    const uint8_t* decode_ptr;      /* pointer to the compressed data */
    uint8_t decode_bit_pos;     /* bitpos inside a byte of the compressed data */

    uint_fast8_t get_unsigned_bits(uint_fast8_t cnt)
    {
      uint_fast8_t bit_pos = this->decode_bit_pos;
      uint_fast8_t val = pgm_read_byte(this->decode_ptr) >> bit_pos;

      auto bit_pos_plus_cnt = bit_pos + cnt;
      if ( bit_pos_plus_cnt >= 8 )
      {
        bit_pos_plus_cnt -= 8;
        val |= pgm_read_byte(++this->decode_ptr) << (8-bit_pos);
      }
      this->decode_bit_pos = bit_pos_plus_cnt;
      return val & ((1U << cnt) - 1);
    }

    int_fast8_t get_signed_bits(uint_fast8_t cnt)
    {
      return (int_fast8_t)get_unsigned_bits(cnt) - (1 << (cnt-1));
    }
  };


  const uint8_t* U8g2font::getGlyph(uint16_t encoding) const
  {
    const uint8_t *font = &this->_font[23];

    if ( encoding <= 255 )
    {
      if ( encoding >= 'a' )      { font += this->start_pos_lower_a(); }
      else if ( encoding >= 'A' ) { font += this->start_pos_upper_A(); }

      for ( ; pgm_read_byte(&font[1]); font += pgm_read_byte(&font[1]))
      {
        if ( pgm_read_byte(&font[0]) == encoding ) { return font + 2; }  /* skip encoding and glyph size */
      }
    }
    else
    {
      uint_fast16_t e;
      const uint8_t *unicode_lut;

      font += this->start_pos_unicode();
      unicode_lut = font;

      do
      {
        font += (pgm_read_byte(&unicode_lut[0]) << 8) + pgm_read_byte(&unicode_lut[1]);
        e     = (pgm_read_byte(&unicode_lut[2]) << 8) + pgm_read_byte(&unicode_lut[3]);
        unicode_lut += 4;
      } while ( e < encoding );

      for ( ; 0 != (e = (pgm_read_byte(&font[0]) << 8) + pgm_read_byte(&font[1])) ; font += pgm_read_byte(&font[2]))
      {
        if ( e == encoding ) { return font + 3; }  /* skip encoding and glyph size */
      }
    }
    return nullptr;
  }

  void U8g2font::getDefaultMetric(lgfx::FontMetrics *metrics) const
  {
    metrics->height    = max_char_height();
    metrics->y_advance = metrics->height;
    metrics->baseline  = metrics->height + y_offset();
    metrics->y_offset  = -metrics->baseline;
    metrics->x_offset  = 0;
  }

  bool U8g2font::updateFontMetric(lgfx::FontMetrics *metrics, uint16_t uniCode) const
  {
    u8g2_font_decode_t decode(getGlyph(uniCode));
    if ( decode.decode_ptr )
    {
      metrics->width     = decode.get_unsigned_bits(this->bits_per_char_width());
                          decode.get_unsigned_bits(this->bits_per_char_height());
      metrics->x_offset  = decode.get_signed_bits  (this->bits_per_char_x());
                          decode.get_signed_bits  (this->bits_per_char_y());
      metrics->x_advance = decode.get_signed_bits  (this->bits_per_delta_x());
      return true;
    }
    metrics->width = metrics->x_advance = this->max_char_width();
    metrics->x_offset = 0;
    return false;
  }

  size_t U8g2font::drawChar(LGFXBase* gfx, int32_t x, int32_t y, uint16_t uniCode, const TextStyle* style, FontMetrics* metrics, int32_t& filled_x) const
  {
    int32_t sy = 65536 * style->size_y;
    y += (metrics->y_offset * sy) >> 16;
    u8g2_font_decode_t decode(getGlyph(uniCode));
    if ( decode.decode_ptr == nullptr ) return drawCharDummy(gfx, x, y, this->max_char_width(), metrics->height, style, filled_x);

    uint32_t w = decode.get_unsigned_bits(bits_per_char_width());
    uint32_t h = decode.get_unsigned_bits(bits_per_char_height());

    int32_t sx = 65536 * style->size_x;

    int32_t xoffset = (decode.get_signed_bits(bits_per_char_x()) * sx) >> 16;

    int32_t yoffset = -(int32_t)(decode.get_signed_bits(bits_per_char_y()) + h + metrics->y_offset);

    int32_t xAdvance = (decode.get_signed_bits(bits_per_delta_x()) * sx) >> 16;

    uint32_t colortbl[2] = {gfx->getColorConverter()->convert(style->back_rgb888), gfx->getColorConverter()->convert(style->fore_rgb888)};
    bool fillbg = (style->back_rgb888 != style->fore_rgb888);
    int32_t left  = 0;
    int32_t right = 0;
    if (fillbg) {
      left  = std::max<int>(filled_x, x + (xoffset < 0 ? xoffset : 0));
      right = x + std::max<int>(((w * sx) >> 16) + xoffset, xAdvance);
      filled_x = right;
      gfx->setRawColor(colortbl[0]);
    }
    x += xoffset;
    gfx->startWrite();

    if (left < right)
    {
      if (yoffset > 0) {
        gfx->writeFillRect(left, y, right - left, (yoffset * sy) >> 16);
      }
      int32_t y0 = ((yoffset + h)   * sy) >> 16;
      int32_t y1 = (metrics->height * sy) >> 16;
      if (y0 < y1) {
        gfx->writeFillRect(left, y + y0, right - left, y1 - y0);
      }
    }

    if ( w > 0 )
    {
      if (left < right)
      {
        int32_t y0  = (  yoffset      * sy) >> 16;
        int32_t len = (((yoffset + h) * sy) >> 16) - y0;
        if (left < x)
        {
          gfx->writeFillRect(left, y + y0, x - left, len);
        }
        int32_t xwsx = x + ((w * sx) >> 16);
        if (xwsx < right)
        {
          gfx->writeFillRect(xwsx, y + y0, right - xwsx, len);
        }
      }
      left -= x;
      uint32_t ab[2];
      uint32_t lx = 0;
      uint32_t ly = 0;
      int32_t y0 = ((yoffset    ) * sy) >> 16;
      int32_t y1 = ((yoffset + 1) * sy) >> 16;
      do
      {
        ab[0] = decode.get_unsigned_bits(bits_per_0());
        ab[1] = decode.get_unsigned_bits(bits_per_1());
        bool i = 0;
        do
        {
          uint32_t length = ab[i];
          while (length)
          {
            uint32_t len = (length > w - lx) ? w - lx : length;
            length -= len;
            if (i || fillbg)
            {
              int32_t x0 = (lx * sx) >> 16;
              if (!i && x0 < left) x0 = left;
              int32_t x1 = ((lx + len) * sx) >> 16;
              if (x0 < x1)
              {
                gfx->setRawColor(colortbl[i]);
                gfx->writeFillRect( x + x0
                                  , y + y0
                                  , x1 - x0
                                  , y1 - y0);
              }
            }
            lx += len;
            if (lx == w)
            {
              lx = 0;
              ++ly;
              y0 = y1;
              y1 = ((ly + yoffset + 1) * sy) >> 16;
            }
          }
          i = !i;
        } while (i || decode.get_unsigned_bits(1) != 0 );
      } while (ly < h);
    }
    gfx->endWrite();
    return xAdvance;
  }

//----------------------------------------------------------------------------

  static size_t draw_alpha_bitmap_common(
      LGFXBase* gfx,
      int32_t x,
      int32_t y,
      const TextStyle* style,
      FontMetrics* metrics,
      int32_t& filled_x,
      int32_t xAdvance,
      int32_t xoffset,
      int32_t yoffset,
      uint32_t box_w,
      uint32_t box_h,
      const uint8_t* bitmap,
      uint32_t glyph_stride,
      uint32_t alpha_max)
  {
    int32_t sy = 65536 * style->size_y;
    int32_t sx = 65536 * style->size_x;

    auto cc = gfx->getColorConverter();
    uint32_t col_back = cc->convert(style->back_rgb888);
    uint32_t col_fore = cc->convert(style->fore_rgb888);
    bool fillbg = (style->back_rgb888 != style->fore_rgb888);
    int32_t glyph_w_scaled = (box_w * sx) >> 16;

    int32_t left = 0;
    int32_t right = 0;
    if (fillbg)
    {
      left  = std::max<int>(filled_x, x + (xoffset < 0 ? xoffset : 0));
      right = x + std::max<int>(glyph_w_scaled + xoffset, xAdvance);
      filled_x = right;
    }

    int32_t draw_x = x + xoffset;

    uint32_t back_rgb = fillbg ? style->back_rgb888 : gfx->getBaseColor();
    int32_t fore_r = (style->fore_rgb888 >> 16) & 0xFF;
    int32_t fore_g = (style->fore_rgb888 >> 8) & 0xFF;
    int32_t fore_b = style->fore_rgb888 & 0xFF;
    int32_t back_r = (back_rgb >> 16) & 0xFF;
    int32_t back_g = (back_rgb >> 8) & 0xFF;
    int32_t back_b = back_rgb & 0xFF;

    gfx->startWrite();

    if (fillbg && left < right)
    {
      gfx->setRawColor(col_back);
      if (yoffset > 0)
      {
        gfx->writeFillRect(left, y, right - left, (yoffset * sy) >> 16);
      }
      int32_t y0 = ((yoffset + (int32_t)box_h) * sy) >> 16;
      int32_t y1 = (metrics->height * sy) >> 16;
      if (y0 < y1)
      {
        gfx->writeFillRect(left, y + y0, right - left, y1 - y0);
      }
    }

    if (bitmap != nullptr && box_w && box_h)
    {
      for (uint32_t py = 0; py < box_h; ++py)
      {
        int32_t y0 = ((yoffset + (int32_t)py) * sy) >> 16;
        int32_t y1 = ((yoffset + (int32_t)py + 1) * sy) >> 16;
        if (y1 <= y0)
        {
          continue;
        }

        if (fillbg && left < right)
        {
          gfx->setRawColor(col_back);
          if (left < draw_x)
          {
            gfx->writeFillRect(left, y + y0, draw_x - left, y1 - y0);
          }
          int32_t draw_right = draw_x + glyph_w_scaled;
          if (draw_right < right)
          {
            gfx->writeFillRect(draw_right, y + y0, right - draw_right, y1 - y0);
          }
        }

        for (uint32_t px = 0; px < box_w; ++px)
        {
          uint32_t alpha = bitmap[py * glyph_stride + px];

          if (!fillbg && alpha == 0)
          {
            continue;
          }

          int32_t x0 = ((int32_t)px * sx) >> 16;
          int32_t x1 = (((int32_t)px + 1) * sx) >> 16;
          if (x1 <= x0)
          {
            continue;
          }

          uint32_t raw;
          if (alpha == 0)
          {
            raw = col_back;
          }
          else if (alpha >= alpha_max)
          {
            raw = col_fore;
          }
          else
          {
            int32_t r = back_r + ((fore_r - back_r) * (int32_t)alpha + (int32_t)(alpha_max >> 1)) / (int32_t)alpha_max;
            int32_t g = back_g + ((fore_g - back_g) * (int32_t)alpha + (int32_t)(alpha_max >> 1)) / (int32_t)alpha_max;
            int32_t b = back_b + ((fore_b - back_b) * (int32_t)alpha + (int32_t)(alpha_max >> 1)) / (int32_t)alpha_max;
            raw = cc->convert(((uint32_t)r << 16) | ((uint32_t)g << 8) | (uint32_t)b);
          }
          gfx->setRawColor(raw);
          gfx->writeFillRect(draw_x + x0, y + y0, x1 - x0, y1 - y0);
        }
      }
    }

    gfx->endWrite();
    return xAdvance;
  }

//----------------------------------------------------------------------------

  namespace
  {
    static inline uint16_t read_u16le(const uint8_t* p)
    {
      return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
    }

    static inline int16_t read_s16le(const uint8_t* p)
    {
      return (int16_t)read_u16le(p);
    }

    static inline uint32_t read_u32le(const uint8_t* p)
    {
      return (uint32_t)p[0]
           | ((uint32_t)p[1] << 8)
           | ((uint32_t)p[2] << 16)
           | ((uint32_t)p[3] << 24);
    }

    static inline bool is_marker(const uint8_t* p, const char* marker)
    {
      return p[0] == (uint8_t)marker[0]
          && p[1] == (uint8_t)marker[1]
          && p[2] == (uint8_t)marker[2]
          && p[3] == (uint8_t)marker[3];
    }

    struct bit_stream_t
    {
      const uint8_t* data = nullptr;
      uint32_t bit_length = 0;
      uint32_t bit_pos = 0;

      uint32_t read_bits(uint8_t count)
      {
        if (count == 0) return 0;
        uint32_t result = 0;
        while (count--)
        {
          if (bit_pos >= bit_length) break;
          uint32_t byte_index = bit_pos >> 3;
          uint32_t bit_index = 7 - (bit_pos & 7);
          result = (result << 1) | ((data[byte_index] >> bit_index) & 1U);
          ++bit_pos;
        }
        return result;
      }

      int32_t read_sbits(uint8_t count)
      {
        if (count == 0) return 0;
        uint32_t v = read_bits(count);
        uint32_t sign = 1U << (count - 1);
        if (v & sign)
        {
          v |= (~0U) << count;
        }
        return (int32_t)v;
      }
    };

    static bool decode_rle_bitmap(bit_stream_t* bs, uint8_t bpp, uint32_t pixel_count, uint8_t* dst)
    {
      enum RleState : uint8_t
      {
        RLE_SINGLE = 0,
        RLE_REPEATED,
        RLE_COUNTER,
      };

      uint32_t out = 0;
      uint8_t prev_v = 0;
      uint8_t count = 0;
      RleState state = RLE_SINGLE;

      while (out < pixel_count)
      {
        uint8_t ret = 0;

        if (state == RLE_SINGLE)
        {
          if (bs->bit_pos + bpp > bs->bit_length) break;
          ret = (uint8_t)bs->read_bits(bpp);

          if (bs->bit_pos != bpp && prev_v == ret)
          {
            count = 0;
            state = RLE_REPEATED;
          }

          prev_v = ret;
        }
        else if (state == RLE_REPEATED)
        {
          if (bs->bit_pos >= bs->bit_length) break;
          uint8_t v = (uint8_t)bs->read_bits(1);
          ++count;

          if (v == 1)
          {
            ret = prev_v;
            if (count == 11)
            {
              if (bs->bit_pos + 6 > bs->bit_length) break;
              count = (uint8_t)bs->read_bits(6);
              if (count != 0)
              {
                state = RLE_COUNTER;
              }
              else
              {
                if (bs->bit_pos + bpp > bs->bit_length) break;
                ret = (uint8_t)bs->read_bits(bpp);
                prev_v = ret;
                state = RLE_SINGLE;
              }
            }
          }
          else
          {
            if (bs->bit_pos + bpp > bs->bit_length) break;
            ret = (uint8_t)bs->read_bits(bpp);
            prev_v = ret;
            state = RLE_SINGLE;
          }
        }
        else // RLE_COUNTER
        {
          ret = prev_v;
          if (count) --count;
          if (count == 0)
          {
            if (bs->bit_pos + bpp > bs->bit_length) break;
            ret = (uint8_t)bs->read_bits(bpp);
            prev_v = ret;
            state = RLE_SINGLE;
          }
        }

        dst[out++] = ret;
      }

      while (out < pixel_count) dst[out++] = 0;
      return true;
    }
  }

  BFFfont::~BFFfont()
  {
    unloadFont();
  }

  bool BFFfont::unloadFont(void)
  {
    _fontLoaded = false;
    if (cmap_data) {
      heap_free(cmap_data);
      cmap_data = nullptr;
    }
    cmap_data_size = 0;
    if (cmap_subtables) {
      heap_free(cmap_subtables);
      cmap_subtables = nullptr;
    }
    cmap_subtable_count = 0;
    if (loca_table) {
      heap_free(loca_table);
      loca_table = nullptr;
    }
    loca_entries = 0;
    if (_fontData) {
      _fontData->preRead();
      _fontData->close();
      _fontData->postRead();
      _fontData = nullptr;
    }
    return true;
  }

  bool BFFfont::loadFont(DataWrapper* data)
  {
    unloadFont();
    if (data == nullptr) return false;

    _fontData = data;

    uint32_t head_record_offset = 0;
    uint32_t head_record_size = 0;

    data->preRead();
    if (!data->seek(0)) {
      data->postRead();
      return false;
    }

    uint32_t offset = 0;
    uint8_t rec_header[8];
    for (int i = 0; i < 32; ++i)
    {
      if (!data->seek(offset)) break;
      if (data->read(rec_header, 8) != 8) break;

      uint32_t rec_size = read_u32le(rec_header);
      if (rec_size < 8) break;

      if (is_marker(&rec_header[4], "head")) {
        head_record_offset = offset;
        head_record_size = rec_size;
      } else if (is_marker(&rec_header[4], "cmap")) {
        cmap_record_offset = offset;
        cmap_record_size = rec_size;
      } else if (is_marker(&rec_header[4], "loca")) {
        loca_record_offset = offset;
        loca_record_size = rec_size;
      } else if (is_marker(&rec_header[4], "glyf")) {
        glyf_record_offset = offset;
        glyf_record_size = rec_size;
      }

      // Stop as soon as all required records are found.
      // Pointer-backed data may not have explicit length bound, so continuing
      // scan can accidentally walk into adjacent arrays and overwrite records.
      if (head_record_size && cmap_record_size && loca_record_size && glyf_record_size)
      {
        break;
      }

      if (rec_size > 0x7FFFFFFFU - offset) break;
      offset += rec_size;
    }

    if (head_record_size < 44 || cmap_record_size < 12 || loca_record_size < 12 || glyf_record_size < 8)
    {
      data->postRead();
      unloadFont();
      return false;
    }

    /*header*/
    {
      uint32_t payload_size = head_record_size - 8;
      auto buf = (uint8_t*)alloca(payload_size);
      data->seek(head_record_offset + 8);
      if (data->read(buf, payload_size) != (int)payload_size)
      {
        data->postRead();
        unloadFont();
        return false;
      }

      // head payload layout follows font_spec.md (little-endian)
      // [0] version(4), [4] extra table num(2)
      font_size             = read_u16le(&buf[6]);
      ascent                = (int16_t)read_u16le(&buf[8]);
      descent               = read_s16le(&buf[10]);
      typo_ascent           = read_u16le(&buf[12]);
      typo_descent          = read_s16le(&buf[14]);
      typo_line_gap         = read_u16le(&buf[16]);
      min_y                 = read_s16le(&buf[18]);
      max_y                 = read_s16le(&buf[20]);
      default_advance_width = read_u16le(&buf[22]);
      kerning_scale         = read_u16le(&buf[24]);
      index_to_loc_format   = buf[26];
      glyph_id_format       = buf[27];
      advance_width_format  = buf[28];
      bits_per_pixel        = buf[29];
      bbox_xy_bits          = buf[30];
      bbox_wh_bits          = buf[31];
      advance_width_bits    = buf[32];
      compression_algorithm = buf[33];
      subpixel_rendering    = buf[34];
    }

    if (bits_per_pixel == 0 || bits_per_pixel > 4)
    {
      data->postRead();
      unloadFont();
      return false;
    }

    // Load cmap payload
    cmap_data_size = cmap_record_size - 8;
    cmap_data = (uint8_t*)heap_alloc_psram(cmap_data_size);
    if (cmap_data == nullptr) cmap_data = (uint8_t*)heap_alloc(cmap_data_size);
    if (cmap_data == nullptr)
    {
      data->postRead();
      unloadFont();
      return false;
    }
    data->seek(cmap_record_offset + 8);
    if (data->read(cmap_data, cmap_data_size) != (int)cmap_data_size)
    {
      data->postRead();
      unloadFont();
      return false;
    }

    if (cmap_data_size < 4)
    {
      data->postRead();
      unloadFont();
      return false;
    }

    cmap_subtable_count = read_u32le(&cmap_data[0]);
    if (cmap_subtable_count > 0)
    {
      uint32_t headers_end = 4 + cmap_subtable_count * 16;
      if (headers_end > cmap_data_size)
      {
        data->postRead();
        unloadFont();
        return false;
      }

      cmap_subtables = (CMapSubtable*)heap_alloc(cmap_subtable_count * sizeof(CMapSubtable));
      if (cmap_subtables == nullptr)
      {
        data->postRead();
        unloadFont();
        return false;
      }

      for (uint32_t i = 0; i < cmap_subtable_count; ++i)
      {
        const uint8_t* h = &cmap_data[4 + i * 16];
        cmap_subtables[i].data_offset     = read_u32le(&h[0]);
        cmap_subtables[i].range_start     = read_u32le(&h[4]);
        cmap_subtables[i].range_length    = read_u16le(&h[8]);
        cmap_subtables[i].glyph_id_offset = read_u16le(&h[10]);
        cmap_subtables[i].entries_count   = read_u16le(&h[12]);
        cmap_subtables[i].format_type     = h[14];
      }

      // lv_font_conv stores cmap data offsets from table-record start (includes 8-byte record header).
      // Here cmap_data points to payload-only, so normalize if needed.
      uint32_t payload_valid = 0;
      uint32_t record_valid = 0;
      for (uint32_t i = 0; i < cmap_subtable_count; ++i)
      {
        uint32_t off = cmap_subtables[i].data_offset;
        if (off == 0) {
          ++payload_valid;
          ++record_valid;
          continue;
        }
        if (off < cmap_data_size) ++payload_valid;
        if (off >= 8 && (off - 8) < cmap_data_size) ++record_valid;
      }
      if (record_valid > payload_valid)
      {
        for (uint32_t i = 0; i < cmap_subtable_count; ++i)
        {
          uint32_t off = cmap_subtables[i].data_offset;
          if (off >= 8) cmap_subtables[i].data_offset = off - 8;
        }
      }
    }

    // Load loca table
    uint8_t loca_hdr[4];
    data->seek(loca_record_offset + 8);
    if (data->read(loca_hdr, 4) != 4)
    {
      data->postRead();
      unloadFont();
      return false;
    }
    loca_entries = read_u32le(loca_hdr);
    if (loca_entries == 0)
    {
      data->postRead();
      unloadFont();
      return false;
    }

    loca_table = (uint32_t*)heap_alloc_psram(loca_entries * sizeof(uint32_t));
    if (loca_table == nullptr) loca_table = (uint32_t*)heap_alloc(loca_entries * sizeof(uint32_t));
    if (loca_table == nullptr)
    {
      data->postRead();
      unloadFont();
      return false;
    }

    if (index_to_loc_format == 0)
    {
      for (uint32_t i = 0; i < loca_entries; ++i)
      {
        uint8_t u16[2];
        if (data->read(u16, 2) != 2)
        {
          data->postRead();
          unloadFont();
          return false;
        }
        loca_table[i] = read_u16le(u16);
      }
    }
    else
    {
      for (uint32_t i = 0; i < loca_entries; ++i)
      {
        uint8_t u32[4];
        if (data->read(u32, 4) != 4)
        {
          data->postRead();
          unloadFont();
          return false;
        }
        loca_table[i] = read_u32le(u32);
      }
    }

    // lv_font_conv may store loca offsets from glyf record start (includes 8-byte record header).
    // Normalize to payload offsets so downstream logic can use [0 .. glyf_payload_size].
    // To avoid false positives, score both hypotheses (shift 0 / shift 8) by actually parsing
    // several glyph headers and selecting the more plausible one.
    {
      uint32_t glyf_payload_size = glyf_record_size - 8;
      uint32_t header_bits = advance_width_bits + bbox_xy_bits + bbox_xy_bits + bbox_wh_bits + bbox_wh_bits;
      uint32_t header_bytes = (header_bits + 7) >> 3;
      if (header_bytes == 0) header_bytes = 1;

      auto score_loca_mode = [&](uint32_t shift_bits8) -> int32_t
      {
        if (header_bytes > 16) return -1;
        uint32_t shift = shift_bits8 ? 8u : 0u;

        // Probe several printable glyphs (skip gid0 if possible).
        uint32_t probe_begin = (loca_entries > 1) ? 1 : 0;
        uint32_t probe_end = std::min<uint32_t>(loca_entries, probe_begin + 12);

        int32_t score = 0;
        uint8_t hdr[16];

        for (uint32_t gid = probe_begin; gid < probe_end; ++gid)
        {
          uint32_t raw_off = loca_table[gid];
          if (raw_off < shift) continue;
          uint32_t off = raw_off - shift;
          if (off >= glyf_payload_size) continue;

          uint32_t next = glyf_payload_size;
          if (gid + 1 < loca_entries)
          {
            uint32_t raw_next = loca_table[gid + 1];
            if (raw_next >= shift)
            {
              uint32_t n = raw_next - shift;
              if (n >= off && n <= glyf_payload_size) next = n;
            }
          }
          if (next <= off || (next - off) < header_bytes) continue;

          data->seek(glyf_record_offset + 8 + off);
          if (data->read(hdr, header_bytes) != (int)header_bytes) continue;

          bit_stream_t bs;
          bs.data = hdr;
          bs.bit_length = header_bytes * 8;

          uint32_t adv = advance_width_bits ? bs.read_bits(advance_width_bits) : default_advance_width;
          int32_t bx = (int16_t)bs.read_sbits(bbox_xy_bits);
          int32_t by = (int16_t)bs.read_sbits(bbox_xy_bits);
          uint32_t bw = bs.read_bits(bbox_wh_bits);
          uint32_t bh = bs.read_bits(bbox_wh_bits);

          // Plausibility scoring.
          if (adv > 0) score += 1;
          if (bw > 0 && bh > 0) score += 3;
          if (bw <= (uint32_t)(font_size * 3 + 8) && bh <= (uint32_t)(font_size * 3 + 8)) score += 2;
          if (std::abs(bx) <= 32 && std::abs(by) <= 32) score += 1;
        }
        return score;
      };

      bool can_shift8 = true;
      for (uint32_t i = 0; i < loca_entries; ++i)
      {
        if (loca_table[i] < 8) { can_shift8 = false; break; }
      }

      int32_t score0 = score_loca_mode(0);
      int32_t score8 = can_shift8 ? score_loca_mode(1) : -1;

      bool use_shift8 = false;
      if (score8 >= 0)
      {
        if (score8 > score0 + 2)
        {
          use_shift8 = true;
        }
        else if (score8 == score0 && loca_entries && loca_table[0] == 8)
        {
          // Tie-breaker for common lv_font_conv output.
          use_shift8 = true;
        }
      }

      if (use_shift8)
      {
        for (uint32_t i = 0; i < loca_entries; ++i)
        {
          loca_table[i] -= 8;
        }
      }
    }

    data->postRead();
    _fontLoaded = true;
    return true;
  }

  void BFFfont::getDefaultMetric(FontMetrics *metrics) const
  {
    metrics->x_offset  = 0;
    metrics->width     = default_advance_width;
    metrics->x_advance = default_advance_width;

    int32_t h = ascent + std::abs((int)descent);
    if (h <= 0) h = (font_size > 0) ? font_size : 16;
    metrics->height = h;
    metrics->baseline = ascent;
    metrics->y_offset = -metrics->baseline;

    int32_t yadv = typo_ascent + std::abs((int)typo_descent) + typo_line_gap;
    if (yadv <= 0) yadv = h;
    metrics->y_advance = yadv;
  }

  bool BFFfont::mapCodepointToGlyph(uint32_t codepoint, uint16_t* glyph_id) const
  {
    if (glyph_id == nullptr || cmap_subtable_count == 0 || cmap_subtables == nullptr || cmap_data == nullptr) return false;

    int32_t l = 0;
    int32_t r = (int32_t)cmap_subtable_count - 1;
    const CMapSubtable* st = nullptr;

    while (l <= r)
    {
      int32_t m = (l + r) >> 1;
      auto& cur = cmap_subtables[m];
      uint32_t start = cur.range_start;
      uint32_t end = start + cur.range_length;
      if (codepoint < start)
      {
        r = m - 1;
      }
      else if (codepoint >= end)
      {
        l = m + 1;
      }
      else
      {
        st = &cur;
        break;
      }
    }
    if (st == nullptr) return false;

    uint32_t index = codepoint - st->range_start;
    uint32_t gid = 0;

    switch (st->format_type)
    {
    default:
      return false;

    case 0: // format 0 (delta-coded uint8 array)
      if (st->data_offset == 0) return false;
      if (st->data_offset + index >= cmap_data_size) return false;
      gid = st->glyph_id_offset + cmap_data[st->data_offset + index];
      if (gid == 0) return false;
      *glyph_id = gid;
      return true;

    case 1: // sparse
      if (st->data_offset == 0) return false;
      if (st->entries_count == 0) return false;
      {
        uint32_t cp_off = st->data_offset;
        uint32_t gid_off = cp_off + st->entries_count * 2;
        if (gid_off + st->entries_count * 2 > cmap_data_size) return false;
        int32_t lo = 0;
        int32_t hi = st->entries_count - 1;
        while (lo <= hi)
        {
          int32_t mid = (lo + hi) >> 1;
          uint16_t delta = read_u16le(&cmap_data[cp_off + mid * 2]);
          if (index < delta) hi = mid - 1;
          else if (index > delta) lo = mid + 1;
          else
          {
            gid = st->glyph_id_offset + read_u16le(&cmap_data[gid_off + mid * 2]);
            if (gid == 0) return false;
            *glyph_id = gid;
            return true;
          }
        }
      }
      return false;

    case 2: // format 0 tiny
      gid = st->glyph_id_offset + index;
      if (gid == 0) return false;
      *glyph_id = gid;
      return true;

    case 3: // sparse tiny
      if (st->data_offset == 0 || st->entries_count == 0) return false;
      if (st->data_offset + st->entries_count * 2 > cmap_data_size) return false;
      {
        int32_t lo = 0;
        int32_t hi = st->entries_count - 1;
        while (lo <= hi)
        {
          int32_t mid = (lo + hi) >> 1;
          uint16_t delta = read_u16le(&cmap_data[st->data_offset + mid * 2]);
          if (index < delta) hi = mid - 1;
          else if (index > delta) lo = mid + 1;
          else
          {
            gid = st->glyph_id_offset + mid;
            if (gid == 0) return false;
            *glyph_id = gid;
            return true;
          }
        }
      }
      return false;
    }
  }

  bool BFFfont::getGlyphOffsetAndLength(uint16_t glyph_id, uint32_t* offset, uint32_t* length) const
  {
    if (offset == nullptr || length == nullptr || loca_table == nullptr || loca_entries == 0) return false;
    if (glyph_id >= loca_entries) return false;

    uint32_t off = loca_table[glyph_id];
    uint32_t glyf_payload_size = glyf_record_size - 8;
    if (off >= glyf_payload_size) return false;

    uint32_t next = glyf_payload_size;
    if ((uint32_t)glyph_id + 1 < loca_entries)
    {
      uint32_t n = loca_table[glyph_id + 1];
      if (n >= off && n <= glyf_payload_size) next = n;
    }

    *offset = glyf_record_offset + 8 + off;
    *length = next - off;
    return true;
  }

  bool BFFfont::loadGlyphInfo(uint16_t glyph_id, GlyphInfo* info) const
  {
    if (!_fontLoaded || _fontData == nullptr || info == nullptr) return false;

    uint32_t offset = 0;
    uint32_t length = 0;
    if (!getGlyphOffsetAndLength(glyph_id, &offset, &length)) return false;

    uint32_t header_bits = advance_width_bits + bbox_xy_bits + bbox_xy_bits + bbox_wh_bits + bbox_wh_bits;
    uint32_t header_bytes = (header_bits + 7) >> 3;
    if (header_bytes == 0) header_bytes = 1;
    if (header_bytes > length) return false;

    uint8_t tmp[32];
    if (header_bytes > sizeof(tmp)) return false;

    _fontData->preRead();
    _fontData->seek(offset);
    bool ok = (_fontData->read(tmp, header_bytes) == (int)header_bytes);
    _fontData->postRead();
    if (!ok) return false;

    bit_stream_t bs;
    bs.data = tmp;
    bs.bit_length = header_bytes * 8;

    info->glyph_id = glyph_id;
    info->advance_raw = advance_width_bits ? bs.read_bits(advance_width_bits) : default_advance_width;
    info->bbox_x = (int16_t)bs.read_sbits(bbox_xy_bits);
    info->bbox_y = (int16_t)bs.read_sbits(bbox_xy_bits);
    info->bitmap_w = (uint16_t)bs.read_bits(bbox_wh_bits);
    info->bitmap_h = (uint16_t)bs.read_bits(bbox_wh_bits);
    // Keep original box size exactly as stored in font data.
    // (LVGL loader also does not shrink box_w/box_h for subpixel mode.)
    info->bbox_w = info->bitmap_w;
    info->bbox_h = info->bitmap_h;
    info->valid = true;
    return true;
  }

  bool BFFfont::decodeGlyphBitmap(uint16_t glyph_id, GlyphInfo* info, uint8_t** out_bitmap) const
  {
    if (out_bitmap == nullptr || info == nullptr) return false;
    *out_bitmap = nullptr;
    if (!info->valid && !loadGlyphInfo(glyph_id, info)) return false;

    uint32_t raw_w = info->bitmap_w;
    uint32_t raw_h = info->bitmap_h;
    uint32_t pixel_count = raw_w * raw_h;
    if (pixel_count == 0)
    {
      *out_bitmap = nullptr;
      return true;
    }

    uint32_t offset = 0;
    uint32_t length = 0;
    if (!getGlyphOffsetAndLength(glyph_id, &offset, &length)) return false;

    uint32_t header_bits = advance_width_bits + bbox_xy_bits + bbox_xy_bits + bbox_wh_bits + bbox_wh_bits;
    uint32_t header_bytes = (header_bits + 7) >> 3;
    if (header_bytes > length) return false;

    auto glyph_buf = (uint8_t*)heap_alloc_psram(length);
    if (glyph_buf == nullptr) glyph_buf = (uint8_t*)heap_alloc(length);
    if (glyph_buf == nullptr) return false;

    _fontData->preRead();
    _fontData->seek(offset);
    bool ok = (_fontData->read(glyph_buf, length) == (int)length);
    _fontData->postRead();
    if (!ok)
    {
      heap_free(glyph_buf);
      return false;
    }

    auto bitmap = (uint8_t*)heap_alloc_psram(pixel_count);
    if (bitmap == nullptr) bitmap = (uint8_t*)heap_alloc(pixel_count);
    if (bitmap == nullptr)
    {
      heap_free(glyph_buf);
      return false;
    }

    bit_stream_t bs;
    bs.data = glyph_buf;
    bs.bit_length = length * 8;
    bs.bit_pos = header_bits;

    if (compression_algorithm == 0)
    {
      for (uint32_t i = 0; i < pixel_count; ++i) bitmap[i] = bs.read_bits(bits_per_pixel);
    }
    else if (compression_algorithm == 1 || compression_algorithm == 2)
    {
      decode_rle_bitmap(&bs, bits_per_pixel, pixel_count, bitmap);
      if (compression_algorithm == 1)
      {
        for (uint32_t y = 1; y < raw_h; ++y)
        {
          uint8_t* row = &bitmap[y * raw_w];
          uint8_t* prev = row - raw_w;
          for (uint32_t x = 0; x < raw_w; ++x)
          {
            row[x] ^= prev[x];
          }
        }
      }
    }
    else
    {
      heap_free(bitmap);
      heap_free(glyph_buf);
      return false;
    }

    heap_free(glyph_buf);

    // If source glyph is subpixel-rendered (RGB triplets), collapse to grayscale
    // so non-subpixel panels can get a closer visual result to LVGL AA rendering.
    if (subpixel_rendering && raw_w >= 3)
    {
      uint32_t out_w = raw_w / 3;
      if (out_w == 0) out_w = 1;

      auto gray = (uint8_t*)heap_alloc_psram(out_w * raw_h);
      if (gray == nullptr) gray = (uint8_t*)heap_alloc(out_w * raw_h);
      if (gray != nullptr)
      {
        for (uint32_t y = 0; y < raw_h; ++y)
        {
          const uint8_t* src = &bitmap[y * raw_w];
          uint8_t* dst = &gray[y * out_w];
          for (uint32_t x = 0; x < out_w; ++x)
          {
            uint32_t s = x * 3;
            uint32_t r = src[s + 0];
            uint32_t g = src[s + 1];
            uint32_t b = src[s + 2];
            // Use integer luma approximation: 0.299R + 0.587G + 0.114B
            dst[x] = (uint8_t)((r * 77U + g * 150U + b * 29U + 128U) >> 8);
          }
        }

        heap_free(bitmap);
        bitmap = gray;
        info->bitmap_w = (uint16_t)out_w;
        info->bbox_w = (uint16_t)out_w;
      }
    }

    *out_bitmap = bitmap;
    return true;
  }

  bool BFFfont::updateFontMetric(FontMetrics *metrics, uint16_t uniCode) const
  {
    uint16_t gid = 0;
    bool found = mapCodepointToGlyph(uniCode, &gid);
    if (!found) gid = 0;

    GlyphInfo info;
    if (!loadGlyphInfo(gid, &info))
    {
      metrics->x_offset = 0;
      metrics->width = metrics->x_advance = default_advance_width;
      return false;
    }

    int32_t adv = info.advance_raw;
    if (advance_width_format == 1) adv = (adv + 8) >> 4;
    metrics->x_offset = info.bbox_x;
    metrics->width = info.bbox_w;
    metrics->x_advance = adv;
    return found;
  }

  size_t BFFfont::drawChar(LGFXBase* gfx, int32_t x, int32_t y, uint16_t uniCode, const TextStyle* style, FontMetrics* metrics, int32_t& filled_x) const
  {
    int32_t sy = 65536 * style->size_y;
    int32_t sx = 65536 * style->size_x;
    y += (metrics->y_offset * sy) >> 16;

    uint16_t gid = 0;
    bool found = mapCodepointToGlyph(uniCode, &gid);
    if (!found) gid = 0;

    GlyphInfo info;
    uint8_t* bitmap = nullptr;
    if (!decodeGlyphBitmap(gid, &info, &bitmap))
    {
      return drawCharDummy(gfx, x, y, default_advance_width, metrics->height, style, filled_x);
    }

    int32_t xAdvance = info.advance_raw;
    if (advance_width_format == 1)
    {
      xAdvance = (xAdvance * sx) >> 20;
    }
    else
    {
      xAdvance = (xAdvance * sx) >> 16;
    }

    int32_t xoffset = (info.bbox_x * sx) >> 16;
    // BFF spec stores bbox_y as bottom position from baseline (upward-positive).
    // Renderer needs top-left draw origin, so convert with: top = baseline - (bbox_y + bbox_h).
    int32_t yoffset = metrics->baseline - (info.bbox_y + info.bbox_h);

    uint32_t max_alpha = (1U << bits_per_pixel) - 1;
    size_t drawn = draw_alpha_bitmap_common(
        gfx,
        x,
        y,
        style,
        metrics,
        filled_x,
        xAdvance,
        xoffset,
        yoffset,
        info.bbox_w,
        info.bbox_h,
        bitmap,
        info.bbox_w,
        max_alpha);
    if (bitmap) heap_free(bitmap);
    return drawn;
  }

//----------------------------------------------------------------------------

  void VLWfont::getDefaultMetric(FontMetrics *metrics) const
  {
    metrics->x_offset  = 0;
    metrics->y_offset  = 0;
    metrics->baseline  = maxAscent;
    metrics->y_advance = yAdvance;
    metrics->height    = yAdvance;
  }

  VLWfont::~VLWfont() {
    unloadFont();
  }

  bool VLWfont::unloadFont(void)
  {
    _fontLoaded = false;
    if (gUnicode)  { heap_free(gUnicode);  gUnicode  = nullptr; }
    if (gWidth)    { heap_free(gWidth);    gWidth    = nullptr; }
    if (gxAdvance) { heap_free(gxAdvance); gxAdvance = nullptr; }
    if (gdX)       { heap_free(gdX);       gdX       = nullptr; }
    if (gBitmap)   { heap_free(gBitmap);   gBitmap   = nullptr; }
    if (_fontData) {
      _fontData->preRead();
      _fontData->close();
      _fontData->postRead();
      _fontData = nullptr;
    }
    return true;
  }

  bool VLWfont::getUnicodeIndex(uint16_t unicode, uint16_t *index) const
  {
    if (gUnicode[gCount-1] < unicode) return false;
    auto poi = std::lower_bound(gUnicode, &gUnicode[gCount], unicode);
    *index = std::distance(gUnicode, poi);
    return (*poi == unicode);
  }

  bool VLWfont::updateFontMetric(FontMetrics *metrics, uint16_t uniCode) const {
    uint16_t gNum = 0;
    if (getUnicodeIndex(uniCode, &gNum)) {
      if (gWidth && gxAdvance && gdX[gNum]) {
        metrics->width     = gWidth[gNum];
        metrics->x_advance = gxAdvance[gNum];
        metrics->x_offset  = gdX[gNum];
      } else {
        auto file = _fontData;

        file->preRead();

        file->seek(28 + gNum * 28);  // headerPtr
        uint32_t buffer[6];
        file->read((uint8_t*)buffer, 24);
        metrics->width     = getSwap32(buffer[1]); // Width of glyph
        metrics->x_advance = getSwap32(buffer[2]); // xAdvance - to move x cursor
        metrics->x_offset  = (int32_t)((int8_t)getSwap32(buffer[4])); // x delta from cursor

        file->postRead();
      }
      return true;
    }
    metrics->width = metrics->x_advance = this->spaceWidth;
    metrics->x_offset = 0;
    return (uniCode == 0x20);
  }


  bool VLWfont::loadFont(DataWrapper* data) {
    _fontData = data;
    {
      uint32_t buf[6];
      data->read((uint8_t*)buf, 6 * 4); // 24 Byte read

      gCount   = getSwap32(buf[0]); // glyph count in file
                //getSwap32(buf[1]); // vlw encoder version - discard
      yAdvance = getSwap32(buf[2]); // Font size in points, not pixels
                //getSwap32(buf[3]); // discard
      ascent   = abs((int32_t)getSwap32(buf[4])); // top of "d"
      descent  = abs((int32_t)getSwap32(buf[5])); // bottom of "p"
    }

    // These next gFont values might be updated when the Metrics are fetched
    maxAscent  = ascent;   // Determined from metrics
    maxDescent = descent;  // Determined from metrics
    yAdvance   = std::max((int)yAdvance, ascent + descent);
    spaceWidth = yAdvance * 2 / 7;  // Guess at space width

//printf("LGFX:ascent:%d  descent:%d\r\n", ascent, descent);

    if (!gCount) return false;

//printf("LGFX:font count:%d\r\n", gCount);

    uint32_t bitmapPtr = 24 + (uint32_t)gCount * 28;

    gBitmap   = (uint32_t*)heap_alloc_psram( gCount * 4); // seek pointer to glyph bitmap in the file
    gUnicode  = (uint16_t*)heap_alloc_psram( gCount * 2); // Unicode 16 bit Basic Multilingual Plane (0-FFFF)
    gWidth    =  (uint8_t*)heap_alloc_psram( gCount );    // Width of glyph
    gxAdvance =  (uint8_t*)heap_alloc_psram( gCount );    // xAdvance - to move x cursor
    gdX       =   (int8_t*)heap_alloc_psram( gCount );    // offset for bitmap left edge relative to cursor X

    if (nullptr == gBitmap  ) gBitmap   = (uint32_t*)heap_alloc( gCount * 4); // seek pointer to glyph bitmap in the file
    if (nullptr == gUnicode ) gUnicode  = (uint16_t*)heap_alloc( gCount * 2); // Unicode 16 bit Basic Multilingual Plane (0-FFFF)
    if (nullptr == gWidth   ) gWidth    =  (uint8_t*)heap_alloc( gCount );    // Width of glyph
    if (nullptr == gxAdvance) gxAdvance =  (uint8_t*)heap_alloc( gCount );    // xAdvance - to move x cursor
    if (nullptr == gdX      ) gdX       =   (int8_t*)heap_alloc( gCount );    // offset for bitmap left edge relative to cursor X

    if (!gUnicode
      || !gBitmap
      || !gWidth
      || !gxAdvance
      || !gdX) {
//printf("LGFX:can not alloc font table\r\n");
      return false;
    }

    _fontLoaded = true;

    size_t gNum = 0;
    _fontData->seek(24);  // headerPtr
    uint32_t buffer[7];
    do {
      _fontData->read((uint8_t*)buffer, 7 * 4); // 28 Byte read
      uint16_t unicode = getSwap32(buffer[0]); // Unicode code point value
      uint32_t w = (uint8_t)getSwap32(buffer[2]); // Width of glyph
      if (gUnicode)   gUnicode[gNum]  = unicode;
      if (gWidth)     gWidth[gNum]    = w;
      if (gxAdvance)  gxAdvance[gNum] = (uint8_t)getSwap32(buffer[3]); // xAdvance - to move x cursor
      if (gdX)        gdX[gNum]       =  (int8_t)getSwap32(buffer[5]); // x delta from cursor

      uint16_t height = getSwap32(buffer[1]); // Height of glyph
      if ((unicode > 0xFF) || ((unicode > 0x20) && (unicode < 0xA0) && (unicode != 0x7F))) {
        int16_t dY =  (int16_t)getSwap32(buffer[4]); // y delta from baseline
//printf("LGFX:unicode:%x  dY:%d\r\n", unicode, dY);
        if (maxAscent < dY && unicode != 0x3000) {
          maxAscent = dY;
        }
        if (maxDescent < (height - dY) && unicode != 0x3000) {
//printf("LGFX:maxDescent:%d\r\n", maxDescent);
          maxDescent = height - dY;
        }
      }

      if (gBitmap)  gBitmap[gNum] = bitmapPtr;
      bitmapPtr += w * height;
    } while (++gNum < gCount);

    yAdvance = maxAscent + maxDescent;

//printf("LGFX:maxDescent:%d\r\n", maxDescent);
    return true;
  }

//----------------------------------------------------------------------------

  size_t VLWfont::drawChar(LGFXBase* gfx, int32_t x, int32_t y, uint16_t code, const TextStyle* style, FontMetrics* metrics, int32_t& filled_x) const
  {
    auto file = this->_fontData;

    uint32_t buffer[6] = {0};
    uint16_t gNum = 0;

    int32_t sy = 65536 * style->size_y;
    y += (metrics->y_offset * sy) >> 16;

    if (code == 0x20) {
      gNum = 0xFFFF;
      buffer[2] = getSwap32(this->spaceWidth);
    } else if (!this->getUnicodeIndex(code, &gNum)) {
      return drawCharDummy(gfx, x, y, this->spaceWidth, metrics->height, style, filled_x);
    } else {
      file->preRead();
      file->seek(28 + gNum * 28);
      file->read((uint8_t*)buffer, 24);
      file->seek(this->gBitmap[gNum]);
    }


    int32_t h        = getSwap32(buffer[0]); // Height of glyph
    int32_t w        = getSwap32(buffer[1]); // Width of glyph
    int32_t sx       = 65536 * style->size_x;
    int32_t xAdvance = (getSwap32(buffer[2]) * sx) >> 16; // xAdvance - to move x cursor
    int32_t xoffset  = ((int32_t)((int8_t)getSwap32(buffer[4])) * sx) >> 16; // x delta from cursor
    int32_t dY       = (int16_t)getSwap32(buffer[3]); // y delta from baseline
    int32_t yoffset  = (this->maxAscent - dY);
//      int32_t yoffset = (gfx->_font_metrics.y_offset) - dY;

    auto pixel = (uint8_t*)alloca(w * h);
    if (gNum != 0xFFFF) {
      file->read(pixel, w * h);
      file->postRead();
    }

    gfx->startWrite();

    uint32_t colortbl[2] = {gfx->getColorConverter()->convert(style->back_rgb888), gfx->getColorConverter()->convert(style->fore_rgb888)};
    bool fillbg = (style->back_rgb888 != style->fore_rgb888);
    int32_t left  = 0;
    int32_t right = 0;
    if (fillbg) {
      left  = std::max(filled_x, x + (xoffset < 0 ? xoffset : 0));
      right = x + std::max<int>(((w * sx) >> 16) + xoffset, xAdvance);
      filled_x = right;
    }
    x += xoffset;

    int32_t clip_left;
    int32_t clip_top;
    int32_t clip_w;
    int32_t clip_h;

    gfx->getClipRect(&clip_left, &clip_top, &clip_w, &clip_h);
    int32_t clip_right = clip_left + clip_w-1;
    int32_t clip_bottom = clip_top + clip_h-1;

    int32_t bx = x;
    int32_t bw = (w * sx) >> 16;
    if (x < clip_left) { bw += (x - clip_left); bx = clip_left; }

    if (bw > clip_right+1 - bx) bw = clip_right+1 - bx;

    if (bw >= 0)
    {
      int32_t fore_r = ((style->fore_rgb888>>16)&0xFF);
      int32_t fore_g = ((style->fore_rgb888>> 8)&0xFF);
      int32_t fore_b = ((style->fore_rgb888)    &0xFF);

      if (fillbg || !gfx->isReadable() || gfx->hasPalette())
      { // fill background mode  or unreadable panel  or palette sprite mode
        if (left < right && fillbg) {
          gfx->setRawColor(colortbl[0]);
          if (yoffset > 0) {
            gfx->writeFillRect(left, y, right - left, (yoffset * sy) >> 16);
          }
          int32_t y0 = ((yoffset + h)   * sy) >> 16;
          int32_t y1 = (metrics->height * sy) >> 16;
          if (y0 < y1) {
            gfx->writeFillRect(left, y + y0, right - left, y1 - y0);
          }
        }

        if (0 < w) {
          uint32_t back = fillbg ? style->back_rgb888 : gfx->getBaseColor();
          int32_t back_r = ((back>>16)&0xFF);
          int32_t back_g = ((back>> 8)&0xFF);
          int32_t back_b = ( back     &0xFF);
          int32_t i = 0;
          int32_t y0, y1 = (yoffset * sy) >> 16;
          do {
            y0 = y1;
            if (y0 > (clip_bottom - y)) break;
            y1 = ((yoffset + i + 1) * sy) >> 16;
            if (left < right) {
              gfx->setRawColor(colortbl[0]);
              gfx->writeFillRect(left, y + y0, right - left, y1 - y0);
            }
            int32_t j = 0;
            do {
              int32_t x0 = (j * sx) >> 16;
              while (pixel[j] != 0xFF) {
                int32_t x1 = ((j + 1) * sx) >> 16;
                if (pixel[j] != 0 && x0 < x1) {
                  int32_t p = 1 + (uint32_t)pixel[j];
                  gfx->setColor(color888( ( fore_r * p + back_r * (257 - p)) >> 8
                                        , ( fore_g * p + back_g * (257 - p)) >> 8
                                        , ( fore_b * p + back_b * (257 - p)) >> 8 ));
                  gfx->writeFillRect(x + x0, y + y0, x1 - x0, y1 - y0);
                }
                x0 = x1;
                if (++j == w || (clip_right - x) < x0) break;
              }
              if (j == w || (clip_right - x) < x0) break;
              gfx->setRawColor(colortbl[1]);
              do { ++j; } while (j != w && pixel[j] == 0xFF);
              gfx->writeFillRect(x + x0, y + y0, ((j * sx) >> 16) - x0, y1 - y0);
            } while (j != w);
            pixel += w;
          } while (++i < h);
        }
      }
      else // alpha blend mode
      {
        auto buf = (bgr888_t*)alloca((bw * ((sy + 65535) >> 16)) * sizeof(bgr888_t));

        pixelcopy_t p_(buf, gfx->getColorConverter()->depth, rgb888_3Byte, gfx->hasPalette());
        int32_t y0, y1 = (yoffset * sy) >> 16;
        int32_t i = 0;
        do {
          y0 = y1;
          if (y0 > (clip_bottom - y)) break;
          y1 = ((yoffset + i + 1) * sy) >> 16;
          int32_t by = y + y0;
          int32_t bh = y1 - y0;

          if (by < clip_top) { bh += by - clip_top; by = clip_top; }
          if (bh > 0) {
            int32_t j0 = 0;

            // search first and last pixel
            while (j0 != w && !pixel[j0    ]) { ++j0; }
            if (j0 != w) {
              int32_t j1 = w;
              while (j0 != j1 && !pixel[j1 - 1]) { --j1; }
              int32_t rx = (j0 * sx) >> 16;
              int32_t rw = (j1 * sx) >> 16;
              if (rx < bx    -x) rx = bx    -x;
              if (rw > bx+bw -x) rw = bx+bw -x;
              rw -= rx;

              if (0 < rw) {
                gfx->readRectRGB(x + rx, by, rw, bh, (uint8_t*)buf);

                int32_t x0, x1 = ((j0 * sx) >> 16) - rx;
                do {
                  x0 = x1;
                  if (x0 < 0) x0 = 0;
                  x1 = (int)(((j0 + 1) * sx) >> 16) - rx;
                  if (x1 > rw) x1 = rw;
                  if (pixel[j0] && x0 < x1) {
                    int32_t p = 1 + pixel[j0];
                    do {
                      int32_t yy = 0;
                      do {
                        auto bgr = &buf[x0 + yy * rw];
                        bgr->r = ( fore_r * p + bgr->r * (257 - p)) >> 8;
                        bgr->g = ( fore_g * p + bgr->g * (257 - p)) >> 8;
                        bgr->b = ( fore_b * p + bgr->b * (257 - p)) >> 8;
                      } while (++yy != bh);
                    } while (++x0 != x1);
                  }
                } while (++j0 < j1);
                gfx->pushImage(x + rx, by, rw, bh, &p_);
              }
            }
          }
          pixel += w;
        } while (++i < h);
      }
    }
    gfx->endWrite();
    return xAdvance;
  }

//----------------------------------------------------------------------------

  // deprecated array.
  const IFont* fontdata [] =
  {
    &fonts::Font0,  // GLCD font (Font 0)
    &fonts::Font0,  // Font 1 current unused
    &fonts::Font2,
    &fonts::Font0,  // Font 3 current unused
    &fonts::Font4,
    &fonts::Font0,  // Font 5 current unused
    &fonts::Font6,
    &fonts::Font7,
    &fonts::Font8,
    nullptr,
  };

//----------------------------------------------------------------------------
  namespace fonts
  {
    using namespace lgfx;

    // Original Adafruit_GFX "Free Fonts"





    // Sans serif fonts


    #include "../Fonts/GFXFF/FreeSansBold9pt7b.h"  // FF25 or FSSB9
    #include "../Fonts/GFXFF/FreeSansBold12pt7b.h" // FF26 or FSSB12
    #include "../Fonts/GFXFF/FreeSansBold18pt7b.h" // FF27 or FSSB18


    // Serif fonts




    // Custom fonts
    #include "../Fonts/Custom/DejaVu9.h"
    #include "../Fonts/Custom/DejaVu12.h"
    #include "../Fonts/Custom/DejaVu18.h"
    #include "../Fonts/Custom/DejaVu24.h"
    #include "../Fonts/Custom/DejaVu40.h"

    #include "../Fonts/glcdfont.h"
    #include "../Fonts/Font16.h"
    #include "../Fonts/Font32rle.h"
    #include "../Fonts/Font64rle.h"
    #include "../Fonts/Font7srle.h"
    #include "../Fonts/Font72rle.h"
    #include "../Fonts/Font8x8C64.h"
    #include "../Fonts/Ascii24x48.h"
    #include "../Fonts/Ascii8x16.h"

    static constexpr uint8_t font0_info[]         = {  0, 255, 5 }; // start code, end code, width
    static constexpr uint8_t font8x8c64_info[]    = { 32, 143, 8 }; // start code, end code, width
    static constexpr uint8_t fontlib24x48_info[]  = { 32, 126, 0 }; // start code, end code

    const GLCDfont Font0 = { font      , font0_info, 6, 8, 7 };
    const BMPfont  Font2 = { chrtbl_f16, widtbl_f16, 0, chr_hgt_f16, baseline_f16 };
    const RLEfont  Font4 = { chrtbl_f32, widtbl_f32, 0, chr_hgt_f32, baseline_f32 };
    const RLEfont  Font6 = { chrtbl_f64, widtbl_f64, 0, chr_hgt_f64, baseline_f64 };
    const RLEfont  Font7 = { chrtbl_f7s, widtbl_f7s, 0, chr_hgt_f7s, baseline_f7s };
    const RLEfont  Font8 = { chrtbl_f72, widtbl_f72, 0, chr_hgt_f72, baseline_f72 };
    const GLCDfont Font8x8C64 = { font8x8_c64, font8x8c64_info, 8, 8, 7 };
    const FixedBMPfont AsciiFont8x16  = { FontLib8x16 , font0_info,  8, 16, 13 };
    const FixedBMPfont AsciiFont24x48 = { FontLib24x48, fontlib24x48_info, 24, 48, 40 };









  }
 }
}

