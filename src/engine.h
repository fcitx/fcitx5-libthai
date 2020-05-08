/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#ifndef _FCITX5_LIBTHAI_ENGINE_H_
#define _FCITX5_LIBTHAI_ENGINE_H_

#include "thaikb.h"
#include <fcitx-config/iniparser.h>
#include <fcitx-utils/i18n.h>
#include <fcitx/addonfactory.h>
#include <fcitx/addonmanager.h>
#include <fcitx/inputcontextproperty.h>
#include <fcitx/inputmethodengine.h>
#include <fcitx/instance.h>
#include <iconv.h>
#include <thai/thinp.h>

namespace fcitx {

FCITX_CONFIG_ENUM_NAME_WITH_I18N(ThaiKBMap, N_("KETMANEE"), N_("PATTACHOTE"),
                                 N_("TIS820_2538"));
FCITX_CONFIG_ENUM_NAME_WITH_I18N(thstrict_t, N_("Passthrough"),
                                 N_("Basic check"), N_("Strict"));

FCITX_CONFIGURATION(
    LibThaiConfig,
    OptionWithAnnotation<ThaiKBMap, ThaiKBMapI18NAnnotation> keyboardMap{
        this, "KeyboardMap", _("Keyboard Map"), ThaiKBMap::KETMANEE};
    Option<bool> correction{this, "Correction", _("Correction"), true};
    OptionWithAnnotation<thstrict_t, thstrict_tI18NAnnotation> strictness{
        this, "Strictness", _("Strictness"), ISC_BASICCHECK};

);

class LibThaiState;

class IconvWrapper {
public:
    IconvWrapper(const char *from, const char *to)
        : conv_(iconv_open(to, from)) {}

    ~IconvWrapper() {
        if (*this) {
            iconv_close(conv_);
        }
    }

    operator iconv_t() const { return conv_; }
    operator bool() const { return conv_ != reinterpret_cast<iconv_t>(-1); }

private:
    iconv_t conv_;
};

class LibThaiEngine final : public InputMethodEngine {
public:
    LibThaiEngine(Instance *instance);
    ~LibThaiEngine();

    void activate(const InputMethodEntry &entry,
                  InputContextEvent &event) override;
    void keyEvent(const InputMethodEntry &entry, KeyEvent &keyEvent) override;
    void reset(const InputMethodEntry &entry,
               InputContextEvent &event) override;
    void deactivate(const fcitx::InputMethodEntry &,
                    fcitx::InputContextEvent &event) override;
    const fcitx::Configuration *getConfig() const override { return &config_; }
    void setConfig(const fcitx::RawConfig &raw) override {
        config_.load(raw, true);
        safeSaveAsIni(config_, "conf/libthai.conf");
    }

    void reloadConfig() override { readAsIni(config_, "conf/libthai.conf"); }

    iconv_t convFromUtf8() const { return convFromUtf8_; }
    iconv_t convToUtf8() const { return convToUtf8_; }

private:
    Instance *instance_;
    IconvWrapper convFromUtf8_;
    IconvWrapper convToUtf8_;
    LibThaiConfig config_;
    FactoryFor<LibThaiState> factory_;
};

class LibThaiFactory : public AddonFactory {
public:
    AddonInstance *create(AddonManager *manager) override {
        return new LibThaiEngine(manager->instance());
    }
};
} // namespace fcitx

#endif // _FCITX5_LIBTHAI_ENGINE_H_
