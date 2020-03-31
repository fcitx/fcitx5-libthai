//
// Copyright (C) 2020~2020 by CSSlayer
// wengxt@gmail.com
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
#include "engine.h"
#include <deque>
#include <fcitx-utils/log.h>
#include <fcitx-utils/utf8.h>
#include <fcitx/inputcontext.h>
#include <fcitx/inputcontextmanager.h>
#include <thai/thcell.h>
#include <thai/thinp.h>

namespace fcitx {

FCITX_DEFINE_LOG_CATEGORY(libthai_log, "libthai");

#define LIBTHAI_DEBUG() FCITX_LOGC(libthai_log, Debug)

constexpr auto FALLBACK_BUFF_SIZE = 4;

std::vector<uint8_t> tryConvert(iconv_t conv, std::string_view s) {
    for (auto iter = std::begin(s), e = std::end(s); iter != e;
         iter = utf8::nextChar(iter)) {
        std::vector<uint8_t> result;
        result.resize(s.size() * 10);
        size_t byteLength = s.size();
        size_t byteRemains = result.size();
        char *data = const_cast<char *>(s.data());
        char *outData = reinterpret_cast<char *>(result.data());
        auto err = iconv(conv, &data, &byteLength, &outData, &byteRemains);
        if (err == static_cast<size_t>(-1)) {
            continue;
        }
        byteLength = 0;
        err = iconv(conv, nullptr, &byteLength, &outData, &byteRemains);
        if (err == static_cast<size_t>(-1)) {
            continue;
        }
        if (data != s.data() + s.size()) {
            continue;
        }
        result.resize(result.size() - byteRemains);
        return result;
    }
    return {};
}

class LibThaiState : public InputContextProperty {
public:
    LibThaiState(LibThaiEngine *engine, InputContext &ic)
        : engine_(engine), ic_(&ic) {}

    ~LibThaiState() {}

    void rememberPrevChars(thchar_t newChar) {
        if (buffer_.size() == FALLBACK_BUFF_SIZE) {
            buffer_.pop_front();
        }
        buffer_.push_back(newChar);
    }

    bool commitString(thchar_t *chr, size_t length) {
        auto s =
            tryConvert(engine_->convToUtf8(),
                       std::string_view(reinterpret_cast<char *>(chr), length));
        if (s.empty()) {
            return false;
        }
        std::string commit{s.begin(), s.end()};
        LIBTHAI_DEBUG() << "Commit String: " << commit;
        ic_->commitString(commit);
        return true;
    }

    void forgetPrevChars() { buffer_.clear(); }

    void prevCell(thcell_t *res) {
        th_init_cell(res);
        auto chars = prevChars();
        if (!chars.empty()) {
            th_prev_cell(chars.data(), chars.size(), res, true);
        }
    }

    std::vector<thchar_t> prevChars() {
        if (ic_->capabilityFlags().test(CapabilityFlag::SurroundingText)) {
            auto &surroundingText = ic_->surroundingText();
            std::string_view text = surroundingText.text();
            auto length = utf8::lengthValidated(text);
            if (length == utf8::INVALID_LENGTH) {
                return {};
            }

            if (length > FALLBACK_BUFF_SIZE) {
                auto byte = utf8::ncharByteLength(text.begin(),
                                                  length - FALLBACK_BUFF_SIZE);
                text = text.substr(byte);
            }
            LIBTHAI_DEBUG() << "SurroundingText is: " << text;
            return tryConvert(engine_->convFromUtf8(), text);
        }
        return {buffer_.begin(), buffer_.end()};
    }

private:
    LibThaiEngine *engine_;
    InputContext *ic_;
    std::deque<uint8_t> buffer_;
};

LibThaiEngine::LibThaiEngine(Instance *instance)
    : instance_(instance), convFromUtf8_("UTF-8", "TIS-620"),
      convToUtf8_("TIS-620", "UTF-8"), factory_([this](InputContext &ic) {
          return new LibThaiState(this, ic);
      }) {

    if (!convFromUtf8_ || !convToUtf8_) {
        throw std::runtime_error("Failed to open iconv for libthai");
    }
    instance->inputContextManager().registerProperty("libthaiState", &factory_);
    reloadConfig();
}

LibThaiEngine::~LibThaiEngine() {}

void LibThaiEngine::activate(const InputMethodEntry &, InputContextEvent &) {}

void LibThaiEngine::deactivate(const InputMethodEntry &, InputContextEvent &) {}

bool isContextIntactKey(Key key) {

    return (((key.sym() & 0xFF00) == 0xFF00) &&
            (/* IsModifierKey */
             key.isModifier() || (key.sym() == FcitxKey_Mode_switch) ||
             (key.sym() == FcitxKey_Num_Lock))) ||
           (((key.sym() & 0xFE00) == 0xFE00) &&
            (FcitxKey_ISO_Lock <= key.sym() &&
             key.sym() <= FcitxKey_ISO_Last_Group_Lock));
}

bool isContextLostKey(Key key) {

    return ((key.sym() & 0xFF00) == 0xFF00) &&
           (key.sym() == FcitxKey_BackSpace || key.sym() == FcitxKey_Tab ||
            key.sym() == FcitxKey_Linefeed || key.sym() == FcitxKey_Clear ||
            key.sym() == FcitxKey_Return || key.sym() == FcitxKey_Pause ||
            key.sym() == FcitxKey_Scroll_Lock ||
            key.sym() == FcitxKey_Sys_Req || key.sym() == FcitxKey_Escape ||
            key.sym() == FcitxKey_Delete ||
            /* IsCursorkey */
            (FcitxKey_Home <= key.sym() && key.sym() <= FcitxKey_Begin) ||
            /* IsKeypadKey, non-chars only */
            (FcitxKey_KP_Space <= key.sym() &&
             key.sym() <= FcitxKey_KP_Delete) ||
            /* IsMiscFunctionKey */
            (FcitxKey_Select <= key.sym() && key.sym() <= FcitxKey_Break) ||
            /* IsFunctionKey */
            (FcitxKey_F1 <= key.sym() && key.sym() <= FcitxKey_F35));
}

void LibThaiEngine::keyEvent(const InputMethodEntry &, KeyEvent &keyEvent) {
    auto key = keyEvent.rawKey();
    if (keyEvent.isRelease()) {
        return;
    }
    auto state = keyEvent.inputContext()->propertyFor(&factory_);
    // If any ctrl alt super modifier is pressed, ignore.
    if (key.states().testAny(KeyStates{KeyState::Ctrl_Alt, KeyState::Super}) ||
        isContextLostKey(key)) {
        state->forgetPrevChars();
        return;
    }
    if (key.sym() == FcitxKey_None || isContextIntactKey(key)) {
        return;
    }
    int shiftLevel;
    uint8_t newChar;

    // Calculate shift level based on shift and mod5.
    if (!key.states().testAny(KeyStates({KeyState::Shift, KeyState::Mod5}))) {
        shiftLevel = 0;
    } else {
        if (key.states().test(KeyState::Mod5)) {
            shiftLevel = 2;
        } else {
            shiftLevel = 1;
        }
    }

    // Keypad is handled separately.
    if ((FcitxKey_KP_0 <= key.sym()) && (key.sym() <= FcitxKey_KP_9) &&
        key.states().test(KeyState::NumLock) &&
        ((2 == shiftLevel) || (key.states().test(KeyState::CapsLock)))) {
        newChar = key.sym() - FcitxKey_KP_0 + 0xf0;
    } else {
        newChar =
            ThaiKeycodeToChar(*config_.keyboardMap, key.code(), shiftLevel);
        if (0 == newChar) {
            return;
        }
    }
    LIBTHAI_DEBUG() << key.toString() << " ShiftLevel: " << shiftLevel
                    << " New Char: " << static_cast<int>(newChar);

    // No correction -> just reject or commit
    if (!*config_.correction) {
        auto prevChars = state->prevChars();
        thchar_t prevChar = 0;
        if (!prevChars.empty()) {
            prevChar = prevChars.back();
        }
        if (!th_isaccept(prevChar, newChar, *config_.strictness)) {
            return keyEvent.filterAndAccept();
        }
        if (state->commitString(&newChar, 1)) {
            keyEvent.filterAndAccept();
        }
        return;
    }

    thinpconv_t conv;
    thcell_t contextCell;
    state->prevCell(&contextCell);
    if (!th_validate_leveled(contextCell, newChar, &conv,
                             *config_.strictness)) {
        return keyEvent.filterAndAccept();
    }

    if (conv.offset < 0) {
        // SurroundingText not supported, so just reject the key.
        if (!keyEvent.inputContext()->capabilityFlags().test(
                CapabilityFlag::SurroundingText)) {
            return keyEvent.filter();
        }

        keyEvent.inputContext()->deleteSurroundingText(conv.offset,
                                                       -conv.offset);
    }
    state->forgetPrevChars();
    state->rememberPrevChars(newChar);
    if (state->commitString(conv.conv,
                            strlen(reinterpret_cast<char *>(conv.conv)))) {
        return keyEvent.filterAndAccept();
    }
}

void LibThaiEngine::reset(const InputMethodEntry &, InputContextEvent &event) {
    auto state = event.inputContext()->propertyFor(&factory_);
    state->forgetPrevChars();
}

} // namespace fcitx

FCITX_ADDON_FACTORY(fcitx::LibThaiFactory);
