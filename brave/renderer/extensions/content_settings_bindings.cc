// Copyright (c) 2013 GitHub, Inc.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

#include "brave/renderer/extensions/content_settings_bindings.h"

#include <string>
#include <vector>

#include "atom/common/native_mate_converters/string16_converter.h"
#include "atom/common/native_mate_converters/value_converter.h"
#include "atom/renderer/content_settings_manager.h"
#include "content/public/renderer/render_frame.h"
#include "content/public/renderer/render_view.h"
#include "extensions/renderer/script_context.h"
#include "third_party/WebKit/public/web/WebView.h"
#include "third_party/WebKit/public/web/WebLocalFrame.h"
#include "third_party/WebKit/public/web/WebDocument.h"

using atom::ContentSettingsManager;

namespace mate {

template<>
struct Converter<ContentSetting> {
  static v8::Local<v8::Value> ToV8(v8::Isolate* isolate,
                                   ContentSetting val) {
    std::string setting;
    switch (val) {
      case CONTENT_SETTING_ALLOW: setting = "allow"; break;
      case CONTENT_SETTING_BLOCK: setting = "block"; break;
      case CONTENT_SETTING_ASK: setting = "ask"; break;
      case CONTENT_SETTING_SESSION_ONLY: setting = "session"; break;
      default: setting = "default"; break;
    }
    return mate::ConvertToV8(isolate, setting);
  }
};

}  // namespace mate

namespace brave {

ContentSettingsBindings::ContentSettingsBindings(
    extensions::ScriptContext* context)
    : extensions::ObjectBackedNativeHandler(context) {
  RouteFunction(
      "getCurrent",
      base::Bind(&ContentSettingsBindings::GetCurrentSetting,
          base::Unretained(this)));
  RouteFunction(
      "getContentTypes",
      base::Bind(&ContentSettingsBindings::GetContentTypes,
          base::Unretained(this)));
}

ContentSettingsBindings::~ContentSettingsBindings() {
}

void ContentSettingsBindings::GetCurrentSetting(
      const v8::FunctionCallbackInfo<v8::Value>& args) {
  const std::string content_type =
      mate::V8ToString(args[0].As<v8::String>());
  bool incognito = args[1].As<v8::Boolean>()->Value();

  ContentSetting setting =
    atom::ContentSettingsManager::GetInstance()->GetSetting(
          ContentSettingsManager::GetOriginOrURL(
              context()->GetRenderFrame()->GetWebFrame()),
          context()->web_frame()->GetDocument().Url(),
          content_type,
          incognito);

  args.GetReturnValue().Set(
    mate::Converter<ContentSetting>::ToV8(context()->isolate(), setting));
}

void ContentSettingsBindings::GetContentTypes(
      const v8::FunctionCallbackInfo<v8::Value>& args) {
  std::vector<std::string> content_types =
    atom::ContentSettingsManager::GetInstance()->GetContentTypes();

  args.GetReturnValue().Set(
      mate::Converter<std::vector<std::string>>::ToV8(
          context()->isolate(), content_types));
}

}  // namespace brave
