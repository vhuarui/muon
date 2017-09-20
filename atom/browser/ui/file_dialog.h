// Copyright (c) 2013 GitHub, Inc.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

#ifndef ATOM_BROWSER_UI_FILE_DIALOG_H_
#define ATOM_BROWSER_UI_FILE_DIALOG_H_

#include <string>
#include <utility>
#include <vector>

#include "atom/browser/native_window.h"
#include "base/callback.h"
#include "base/callback_forward.h"
#include "base/files/file_path.h"
#include "ui/shell_dialogs/select_file_dialog.h"
#include "ui/shell_dialogs/selected_file_info.h"

namespace atom {
class NativeWindow;
}

namespace file_dialog {

typedef base::Callback<void(
    bool result, const std::vector<base::FilePath>& paths)> DialogCallback;

struct DialogSettings {
  atom::NativeWindow* parent_window = nullptr;
  std::string title;
  base::FilePath default_path;
  std::vector<std::vector<base::FilePath::StringType>> extensions;
  std::vector<base::string16> extension_description_overrides;
  ui::SelectFileDialog::Type type;
  bool include_all_files = true;
};

class FileDialog : public ui::SelectFileDialog::Listener {
 public:
  static void Show(const DialogSettings& settings,
                   const DialogCallback& callback);

  static bool Show(const DialogSettings& settings,
                   std::vector<base::FilePath>* paths);

 private:
  FileDialog(const DialogSettings& settings,
                 const DialogCallback& callback);
  ~FileDialog() override;

  // SelectFileDialog::Listener implementation.
  void FileSelected(const base::FilePath& path,
                    int index,
                    void* params) override;
  void FileSelectedWithExtraInfo(const ui::SelectedFileInfo& file,
                                 int index,
                                 void* params) override;
  void MultiFilesSelected(const std::vector<base::FilePath>& files,
                          void* params) override;
  void MultiFilesSelectedWithExtraInfo(
      const std::vector<ui::SelectedFileInfo>& files,
      void* params) override;
  void FileSelectionCanceled(void* params) override;

  // Dialog box used for opening and saving files.
  scoped_refptr<ui::SelectFileDialog> select_file_dialog_;

  DialogCallback file_selected_callback_;
};

}  // namespace file_dialog

#endif  // ATOM_BROWSER_UI_FILE_DIALOG_H_
