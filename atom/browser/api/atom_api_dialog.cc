// Copyright (c) 2013 GitHub, Inc.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

#include <string>
#include <utility>
#include <vector>

#include "atom/browser/api/atom_api_window.h"
#include "atom/browser/native_window.h"
#include "atom/browser/ui/file_dialog.h"
#include "atom/browser/ui/message_box.h"
#include "atom/common/native_mate_converters/callback.h"
#include "atom/common/native_mate_converters/file_path_converter.h"
#include "atom/common/native_mate_converters/image_converter.h"
#include "native_mate/dictionary.h"

#include "atom/common/node_includes.h"

namespace mate {

template<>
struct Converter<ui::SelectFileDialog::Type> {
  static bool FromV8(v8::Isolate* isolate,
                     v8::Local<v8::Value> val,
                     ui::SelectFileDialog::Type* out) {
    std::string type;
    if (!ConvertFromV8(isolate, val, &type))
      return false;
    if (type == "select-folder") {
        *out = ui::SelectFileDialog::SELECT_FOLDER;
    } else if (type == "select-upload-folder") {
        *out = ui::SelectFileDialog::SELECT_UPLOAD_FOLDER;
    } else if (type == "select-saveas-file") {
        *out = ui::SelectFileDialog::SELECT_SAVEAS_FILE;
    } else if (type == "select-open-file") {
        *out = ui::SelectFileDialog::SELECT_OPEN_FILE;
    } else if (type == "select-open-multi-file") {
        *out = ui::SelectFileDialog::SELECT_OPEN_MULTI_FILE;
    } else {
        *out = ui::SelectFileDialog::SELECT_NONE;
    }
    return true;
  }
};

template<>
struct Converter<file_dialog::DialogSettings> {
  static bool FromV8(v8::Isolate* isolate,
                     v8::Local<v8::Value> val,
                     file_dialog::DialogSettings* out) {
    mate::Dictionary dict;
    if (!ConvertFromV8(isolate, val, &dict))
      return false;
    dict.Get("window", &(out->parent_window));
    dict.Get("title", &(out->title));
    dict.Get("defaultPath", &(out->default_path));
    dict.Get("type", &(out->type));
    dict.Get("extensions", &(out->extensions));
    dict.Get("extensionDescriptionOverrides",
             &(out->extension_description_overrides));
    dict.Get("includeAllFiles", &(out->include_all_files));
    return true;
  }
};

}  // namespace mate

namespace {

void ShowMessageBox(int type,
                    const std::vector<std::string>& buttons,
                    int default_id,
                    int cancel_id,
                    int options,
                    const std::string& title,
                    const std::string& message,
                    const std::string& detail,
                    const gfx::ImageSkia& icon,
                    atom::NativeWindow* window,
                    mate::Arguments* args) {
  v8::Local<v8::Value> peek = args->PeekNext();
  atom::MessageBoxCallback callback;
  if (mate::Converter<atom::MessageBoxCallback>::FromV8(args->isolate(),
                                                        peek,
                                                        &callback)) {
    atom::ShowMessageBox(window, (atom::MessageBoxType)type, buttons,
                         default_id, cancel_id, options, title,
                         message, detail, icon, callback);
  } else {
    int chosen = atom::ShowMessageBox(window, (atom::MessageBoxType)type,
                                      buttons, default_id, cancel_id,
                                      options, title, message, detail, icon);
    args->Return(chosen);
  }
}

void ShowDialog(const file_dialog::DialogSettings& settings,
                mate::Arguments* args) {
  v8::Local<v8::Value> peek = args->PeekNext();
  file_dialog::DialogCallback callback;
  if (mate::Converter<file_dialog::DialogCallback>::FromV8(args->isolate(),
                                                           peek,
                                                           &callback)) {
    file_dialog::FileDialog::Show(settings, callback);
  } else {
    std::vector<base::FilePath> paths;
    if (file_dialog::FileDialog::Show(settings, &paths))
      args->Return(paths);
  }
}

void Initialize(v8::Local<v8::Object> exports, v8::Local<v8::Value> unused,
                v8::Local<v8::Context> context, void* priv) {
  mate::Dictionary dict(context->GetIsolate(), exports);
  dict.SetMethod("showMessageBox", &ShowMessageBox);
  dict.SetMethod("showErrorBox", &atom::ShowErrorBox);
  dict.SetMethod("showDialog", &ShowDialog);
}

}  // namespace

NODE_MODULE_CONTEXT_AWARE_BUILTIN(atom_browser_dialog, Initialize)
