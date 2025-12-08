/*
 * SPDX-FileCopyrightText: 2025~2025 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#include "fcitx-utils/keysym.h"
#include "testdir.h"
#include "testfrontend_public.h"
#include <fcitx-config/rawconfig.h>
#include <fcitx-utils/key.h>
#include <fcitx-utils/log.h>
#include <fcitx-utils/macros.h>
#include <fcitx-utils/standardpaths.h>
#include <fcitx-utils/testing.h>
#include <fcitx/addonmanager.h>
#include <fcitx/inputmethodgroup.h>
#include <fcitx/inputmethodmanager.h>
#include <fcitx/instance.h>
#include <utility>

using namespace fcitx;

namespace {

void testBasic(Instance *instance) {

    instance->eventDispatcher().schedule([instance]() {
        auto *libthai = instance->addonManager().addon("libthai", true);
        FCITX_ASSERT(libthai);
        auto defaultGroup = instance->inputMethodManager().currentGroup();
        defaultGroup.inputMethodList().clear();
        defaultGroup.inputMethodList().push_back(
            InputMethodGroupItem("keyboard-us"));
        defaultGroup.inputMethodList().push_back(
            InputMethodGroupItem("libthai"));
        defaultGroup.setDefaultInputMethod("");
        instance->inputMethodManager().setGroup(std::move(defaultGroup));

        auto *testfrontend = instance->addonManager().addon("testfrontend");
        auto uuid =
            testfrontend->call<ITestFrontend::createInputContext>("testapp");
        auto *ic = instance->inputContextManager().findByUUID(uuid);
        FCITX_ASSERT(ic);
        instance->setCurrentInputMethod(ic, "libthai", true);

        testfrontend->call<ITestFrontend::pushCommitExpectation>("ฟ");
        FCITX_ASSERT(testfrontend->call<ITestFrontend::sendKeyEvent>(
            uuid, Key(FcitxKey_a, KeyState::NoState, 38), false));

        RawConfig config;
        config.setValueByPath("KeyboardMap", "Manoonchai");
        libthai->setConfig(config);

        testfrontend->call<ITestFrontend::pushCommitExpectation>("ง");
        FCITX_ASSERT(testfrontend->call<ITestFrontend::sendKeyEvent>(
            uuid, Key(FcitxKey_a, KeyState::NoState, 38), false));

        instance->exit();
    });
}

} // namespace

int main() {
    // NOLINTBEGIN(bugprone-suspicious-missing-comma)
    setupTestingEnvironment(
        TESTING_BINARY_DIR, {"bin"},
        {TESTING_BINARY_DIR "/test", TESTING_BINARY_DIR "/im",
         TESTING_BINARY_DIR "/modules", TESTING_SOURCE_DIR "/modules",
         StandardPaths::fcitxPath("pkgdatadir")});
    // NOLINTEND(bugprone-suspicious-missing-comma)
    char arg0[] = "testlibthai";
    char arg1[] = "--disable=all";
    char arg2[] = "--enable=testim,testfrontend,libthai";
    char *argv[] = {arg0, arg1, arg2};
    Log::setLogRule("default=5,libthai=5");
    Instance instance(FCITX_ARRAY_SIZE(argv), argv);
    instance.addonManager().registerDefaultLoader(nullptr);
    testBasic(&instance);
    instance.exec();
    return 0;
}
