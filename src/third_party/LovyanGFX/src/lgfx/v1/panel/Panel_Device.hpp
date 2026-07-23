/*----------------------------------------------------------------------------/
  Lovyan GFX - Graphics library for embedded devices.

Original Source:
 https://github.com/lovyan03/LovyanGFX/

Licence:
 [FreeBSD](https://github.com/lovyan03/LovyanGFX/blob/master/license.txt)

Author:
 [lovyan03](https://twitter.com/lovyan03)

Contributors:
 [ciniml](https://github.com/ciniml)
 [mongonta0716](https://github.com/mongonta0716)
 [tobozo](https://github.com/tobozo)
/----------------------------------------------------------------------------*/
#pragma once

#include "../Panel.hpp"

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------
  struct IBus;
  struct ILight;
  struct ITouch;
  struct touch_point_t;

  struct Panel_Device : public IPanel
  {
  public:
    Panel_Device(void);

    struct config_t
    {
      /// CS 繝斐Φ逡ｪ蜿ｷ;
      /// Number of CS pin
      int16_t pin_cs = -1;

      /// RST 繝斐Φ逡ｪ蜿ｷ;
      /// Number of RST pin
      int16_t pin_rst = -1;

      /// BUSY 繝斐Φ逡ｪ蜿ｷ;
      /// Number of BUSY pin
      int16_t pin_busy = -1;

      /// LCD繝峨Λ繧､繝舌′謇ｱ縺医ｋ逕ｻ蜒上・譛螟ｧ蟷・
      /// The maximum width of an image that the LCD driver can handle.
      uint16_t memory_width = 240;

      /// LCD繝峨Λ繧､繝舌′謇ｱ縺医ｋ逕ｻ蜒上・譛螟ｧ鬮倥＆;
      /// The maximum height of an image that the LCD driver can handle.
      uint16_t memory_height = 240;

      /// 螳滄圀縺ｫ陦ｨ遉ｺ縺ｧ縺阪ｋ蟷・
      /// Actual width of the display.
      uint16_t panel_width = 240;

      /// 螳滄圀縺ｫ陦ｨ遉ｺ縺ｧ縺阪ｋ鬮倥＆;
      /// Actual height of the display.
      uint16_t panel_height = 240;

      /// 繝代ロ繝ｫ縺ｮX譁ｹ蜷代が繝輔そ繝・ヨ驥・
      /// Number of offset pixels in the X direction.
      uint16_t offset_x = 0;

      /// 繝代ロ繝ｫ縺ｮY譁ｹ蜷代が繝輔そ繝・ヨ驥・
      /// Number of offset pixels in the Y direction.
      uint16_t offset_y = 0;

      /// 蝗櫁ｻ｢譁ｹ蜷代・繧ｪ繝輔そ繝・ヨ 0~7 (4~7縺ｯ荳贋ｸ句渚霆｢);
      /// Offset value in the direction of rotation. 0~7 (4~7 is upside down)
      uint8_t offset_rotation = 0;

      /// 繝斐け繧ｻ繝ｫ隱ｭ蜃ｺ縺怜燕縺ｮ繝繝溘・繝ｪ繝ｼ繝峨・繝薙ャ繝域焚;
      /// Number of bits in dummy read before pixel readout.
      uint8_t dummy_read_pixel = 8;

      /// 繝・・繧ｿ隱ｭ蜃ｺ縺怜燕縺ｮ繝繝溘・繝ｪ繝ｼ繝峨・繝薙ャ繝域焚;
      /// Number of bits in dummy read before data readout.
      uint8_t dummy_read_bits = 1;

      /// 繝・・繧ｿ隱ｭ蜃ｺ縺礼ｵゆｺ・凾縺ｮ繧ｦ繧ｧ繧､繝・ST7796縺ｧ蠢・ｦ・;
      uint16_t end_read_delay_us = 0;

      /// 繝・・繧ｿ隱ｭ蜃ｺ縺励′蜿ｯ閭ｽ縺句凄縺・
      /// Whether the data is readable or not.
      bool readable = true;

      /// 譏取囓縺ｮ蜿崎ｻ｢ (IPS繝代ロ繝ｫ縺ｯtrue縺ｫ險ｭ螳・;
      /// brightness inversion (e.g. IPS panel)
      bool invert = false;

      /// RGB=true / BGR=false 繝代ロ繝ｫ縺ｮ襍､縺ｨ髱偵′蜈･繧梧崛繧上▲縺ｦ縺励∪縺・ｴ蜷・true縺ｫ險ｭ螳・
      /// Set the RGB/BGR color order.
      bool rgb_order = false;

      /// 騾∽ｿ｡繝・・繧ｿ縺ｮ16bit繧｢繝ｩ繧､繝｡繝ｳ繝・繝・・繧ｿ髟ｷ繧・6bit蜊倅ｽ阪〒騾∽ｿ｡縺吶ｋ繝代ロ繝ｫ縺ｮ蝣ｴ蜷・true縺ｫ險ｭ螳・
      /// 16-bit alignment of transmitted data
      bool dlen_16bit = false;

      /// SD遲峨・繝輔ぃ繧､繝ｫ繧ｷ繧ｹ繝・Β縺ｨ縺ｮ繝舌せ蜈ｱ譛峨・譛臥┌ (true縺ｫ險ｭ螳壹☆繧九→drawJpgFile遲峨〒繝舌せ蛻ｶ蠕｡縺瑚｡後ｏ繧後ｋ);
      /// Whether or not to share the bus with the file system (if set to true, drawJpgFile etc. will control the bus)
      bool bus_shared = true;
    };

    const config_t& config(void) const { return _cfg; }
    void config(const config_t& cfg) { _cfg = cfg; }

    virtual bool init(bool use_reset);
    virtual bool initTouch(void);

    virtual void initBus(void);
    virtual void releaseBus(void);
    void setBus(IBus* bus);
    void bus(IBus* bus) { setBus(bus); };
    IBus* getBus(void) const { return _bus; }
    IBus* bus(void) const { return _bus; }

    void setLight(ILight* light) { _light = light; }
    void light(ILight* light) { _light = light; }
    ILight* getLight(void) const { return _light; }
    ILight* light(void) const { return _light; }
    void setBrightness(uint8_t brightness) override;


    void setTouch(ITouch* touch);
    void touch(ITouch* touch) { setTouch(touch); }
    ITouch* getTouch(void) const { return _touch; }
    ITouch* touch(void) const { return _touch; }
    virtual uint_fast8_t getTouchRaw(touch_point_t* tp, uint_fast8_t count);
    uint_fast8_t getTouch(touch_point_t* tp, uint_fast8_t count);
    void convertRawXY(touch_point_t *tp, uint_fast8_t count);
    void touchCalibrate(void);
    void setCalibrateAffine(float affine[6]);
    void setCalibrate(uint16_t *parameters);


    bool isReadable(void) const override { return _cfg.readable; }
    bool isBusShared(void) const override { return _cfg.bus_shared; }

    void initDMA(void) override;
    void waitDMA(void) override;
    bool dmaBusy(void) override;
    void display(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h) override;

    void writeCommand(uint32_t data, uint_fast8_t length) override;
    void writeData(uint32_t data, uint_fast8_t length) override;
    //void writePixelsDMA(const uint8_t* data, uint32_t length) override;
    void writeImageARGB(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, pixelcopy_t* param) override;
    void copyRect(uint_fast16_t dst_x, uint_fast16_t dst_y, uint_fast16_t w, uint_fast16_t h, uint_fast16_t src_x, uint_fast16_t src_y) override;

  protected:

    static constexpr uint8_t CMD_INIT_DELAY = 0x80;

    config_t _cfg;

    IBus* _bus = nullptr;
    ILight* _light = nullptr;
    ITouch* _touch = nullptr;
    bool _has_align_data = false;
    bool _dma_oom = false;  // edge state: DMA buffer alloc failing
    uint8_t _internal_rotation = 0;

    float _affine[6] = {1,0,0,0,1,0};  /// touch affine parameter

    /// _bus->getDMABuffer + 繧ｨ繝・ず繝医Μ繧ｬ繝ｼ縺ｮ繝ｭ繧ｰ縲ゅΓ繝｢繝ｪ譫ｯ貂・凾縺ｯ nullptr 繧定ｿ斐☆縺溘ａ縲・
    /// 蜻ｼ縺ｳ蜃ｺ縺怜・縺ｯ蠢・★邨先棡繧堤｢ｺ隱阪＠縲∝､ｱ謨玲凾縺ｯ譖ｸ縺崎ｾｼ縺ｿ繧偵せ繧ｭ繝・・縺吶ｋ縺薙→縲・
    uint8_t* get_dma_buffer_checked(size_t len);

    /// CS繝斐Φ縺ｮ貅門ｙ蜃ｦ逅・ｒ陦後≧縲・S繝斐Φ繧定・蜑阪〒蛻ｶ蠕｡縺吶ｋ蝣ｴ蜷医√％縺ｮ髢｢謨ｰ繧弛verride縺励※螳溯｣・☆繧九％縺ｨ縲・
    /// Performs preparation processing for the CS pin.
    /// If you want to control the CS pin on your own, override this function and implement it.
    virtual void init_cs(void);

    /// 蠑墓焚縺ｫ蠢懊§縺ｦCS繝斐Φ繧貞宛蠕｡縺吶ｋ縲Ｇalse=LOW / true=HIGH縲・S繝斐Φ繧定・蜑阪〒蛻ｶ蠕｡縺吶ｋ蝣ｴ蜷医√％縺ｮ髢｢謨ｰ繧弛verride縺励※螳溯｣・☆繧九％縺ｨ縲・
    /// Controls the CS pin to go HIGH when the argument is true.
    /// If you want to control the CS pin on your own, override this function and implement it.
    virtual void cs_control(bool level);

    /// RST繝斐Φ縺ｮ貅門ｙ蜃ｦ逅・ｒ陦後≧縲３ST繝斐Φ繧定・蜑阪〒蛻ｶ蠕｡縺吶ｋ蝣ｴ蜷医√％縺ｮ髢｢謨ｰ繧弛verride縺励※螳溯｣・☆繧九％縺ｨ縲・
    /// Performs preparation processing for the RST pin.
    /// If you want to control the RST pin on your own, override this function and implement it.
    virtual void init_rst(void);

    /// 蠑墓焚縺ｫ蠢懊§縺ｦRST繝斐Φ繧貞宛蠕｡縺吶ｋ縲Ｇalse=LOW / true=HIGH縲３ST繝斐Φ繧定・蜑阪〒蛻ｶ蠕｡縺吶ｋ蝣ｴ蜷医√％縺ｮ髢｢謨ｰ繧弛verride縺励※螳溯｣・☆繧九％縺ｨ縲・
    /// Controls the RST pin to go HIGH when the argument is true.
    /// If you want to control the RST pin on your own, override this function and implement it.
    virtual void rst_control(bool level);

    /// 繝代ロ繝ｫ縺ｮ蛻晄悄蛹悶さ繝槭Φ繝牙・繧貞ｾ励ｋ縲ら┌縺・ｴ蜷医・nullptr繧定ｿ斐☆縲・
    /// Get the panel initialization command sequence.
    virtual const uint8_t* getInitCommands(uint8_t listno) const { (void)listno; return nullptr; }

    enum fastread_dir_t
    {
      fastread_nothing,
      fastread_horizontal,
      fastread_vertical,
    };
    virtual fastread_dir_t get_fastread_dir(void) const { return fastread_nothing; }

    void command_list(const uint8_t *addr);

  };

//----------------------------------------------------------------------------

  struct Panel_NULL : public Panel_Device
  {
    Panel_NULL(void) = default;

    void initBus(void) override {}
    void releaseBus(void) override {}

    bool init(bool) override { return false; }

    void beginTransaction(void) override {}
    void endTransaction(void) override {}

    color_depth_t setColorDepth(color_depth_t depth) override { return depth; }

    void setInvert(bool) override {}
    void setRotation(uint_fast8_t) override {}
    void setSleep(bool) override {}
    void setPowerSave(bool) override {}

    void writeCommand(uint32_t, uint_fast8_t) override {}
    void writeData(uint32_t, uint_fast8_t) override {}

    void initDMA(void) override {}
    void waitDMA(void) override {}
    bool dmaBusy(void) override { return false; }
    void waitDisplay(void) override {}
    bool displayBusy(void) override { return false; }
    void display(uint_fast16_t, uint_fast16_t, uint_fast16_t, uint_fast16_t) override {}
    bool isReadable(void) const override { return false; }
    bool isBusShared(void) const override { return false; }

    void writeBlock(uint32_t, uint32_t) override {}
    void setWindow(uint_fast16_t, uint_fast16_t, uint_fast16_t, uint_fast16_t) override {}
    void drawPixelPreclipped(uint_fast16_t, uint_fast16_t, uint32_t) override {}
    void writeFillRectPreclipped(uint_fast16_t, uint_fast16_t, uint_fast16_t, uint_fast16_t, uint32_t) override {}
    void writeImage(uint_fast16_t, uint_fast16_t, uint_fast16_t, uint_fast16_t, pixelcopy_t*, bool) override {}
    void writeImageARGB(uint_fast16_t, uint_fast16_t, uint_fast16_t, uint_fast16_t, pixelcopy_t*) override {}
    void writePixels(pixelcopy_t*, uint32_t, bool) override {}

    uint32_t readCommand(uint_fast16_t, uint_fast8_t, uint_fast8_t) override { return 0; }
    uint32_t readData(uint_fast8_t, uint_fast8_t) override { return 0; }
    void readRect(uint_fast16_t, uint_fast16_t, uint_fast16_t, uint_fast16_t, void*, pixelcopy_t*) override {}
    void copyRect(uint_fast16_t, uint_fast16_t, uint_fast16_t, uint_fast16_t, uint_fast16_t, uint_fast16_t) override {}
  };

//----------------------------------------------------------------------------
 }
}

