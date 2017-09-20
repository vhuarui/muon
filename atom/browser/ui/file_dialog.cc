// Copyright (c) 2017 Brave Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "atom/browser/ui/file_dialog.h"

#include <memory>

#include "base/bind.h"
#include "base/path_service.h"
#include "base/run_loop.h"
#include "base/strings/utf_string_conversions.h"
#include "chrome/browser/ui/chrome_select_file_policy.h"
#include "chrome/common/chrome_paths.h"

namespace file_dialog {

void OnShowSaveDialog(bool* target_result,
                      std::vector<base::FilePath>* target_path, bool result,
                      const std::vector<base::FilePath>& paths) {
  if (result) {
    *target_path = paths;
  }
  *target_result = result;
  base::RunLoop::QuitCurrentDeprecated();
}

// Converts a list of FilePaths to a list of ui::SelectedFileInfo.
std::vector<ui::SelectedFileInfo> FilePathListToSelectedFileInfoList(
    const std::vector<base::FilePath>& paths) {
  std::vector<ui::SelectedFileInfo> selected_files;
  for (size_t i = 0; i < paths.size(); ++i) {
    selected_files.push_back(ui::SelectedFileInfo(paths[i], paths[i]));
  }
  return selected_files;
}

std::vector<base::FilePath> SelectedFileInfoListtoFilePathList(
    const std::vector<ui::SelectedFileInfo>& files) {
  std::vector<base::FilePath> paths;
  for (size_t i = 0; i < files.size(); ++i) {
    paths.push_back(files[i].file_path);
  }
  return paths;
}

FileDialog::FileDialog(const DialogSettings& settings,
                               const DialogCallback& callback)
  : file_selected_callback_(callback) {
  select_file_dialog_ =
    ui::SelectFileDialog::Create(
      this,
      std::make_unique<ChromeSelectFilePolicy>(nullptr));
    if (!select_file_dialog_.get())
          return;

    ui::SelectFileDialog::FileTypeInfo file_type_info;
    base::FilePath::StringType default_extension =
      (settings.extensions.size() && settings.extensions[0].size())
        ? settings.extensions[0][0]
        : FILE_PATH_LITERAL("");
    for (size_t i = 0; i < settings.extensions.size(); ++i) {
      file_type_info.extensions.push_back(settings.extensions[i]);
    }
    if (file_type_info.extensions.empty()) {
      base::FilePath::StringType extension =
        settings.default_path.FinalExtension();
      if (!extension.empty()) {
        file_type_info.extensions.push_back(
          std::vector<base::FilePath::StringType>());
        extension.erase(extension.begin());  // drop the .
        file_type_info.extensions[0].push_back(extension);
      }
    }

    base::FilePath default_path = settings.default_path;
    if (default_path.empty()) {
      PathService::Get(chrome::DIR_DEFAULT_DOWNLOADS, &default_path);
    }
    file_type_info.include_all_files = settings.include_all_files;
    file_type_info.extension_description_overrides =
      settings.extension_description_overrides;
    file_type_info.allowed_paths =
      ui::SelectFileDialog::FileTypeInfo::NATIVE_OR_DRIVE_PATH;
    atom::NativeWindow* native_window = settings.parent_window;
    gfx::NativeWindow owning_window = native_window
      ? native_window->GetNativeWindow()
      : NULL;

    base::string16 title;
    if (!settings.title.empty()) {
      title = base::UTF8ToUTF16(settings.title);
    }

    select_file_dialog_->SelectFile(settings.type,
                                    title,
                                    default_path,
                                    &file_type_info,
                                    0,
                                    default_extension,
                                    owning_window,
                                    NULL);
}

FileDialog::~FileDialog() {}

void FileDialog::FileSelected(const base::FilePath& path,
                              int index,
                              void* params) {
  FileSelectedWithExtraInfo(ui::SelectedFileInfo(path, path), index, params);
}

void FileDialog::FileSelectedWithExtraInfo(const ui::SelectedFileInfo& file,
                                           int index,
                                           void* params) {
  std::vector<base::FilePath> files;
  files.push_back(file.file_path);
  file_selected_callback_.Run(true, files);
  delete this;
}
void FileDialog::MultiFilesSelected(const std::vector<base::FilePath>& files,
                                    void* params) {
  std::vector<ui::SelectedFileInfo> selected_files =
    FilePathListToSelectedFileInfoList(files);

  MultiFilesSelectedWithExtraInfo(selected_files, params);
}

void FileDialog::MultiFilesSelectedWithExtraInfo(
    const std::vector<ui::SelectedFileInfo>& files,
    void* params) {
  file_selected_callback_.Run(true, SelectedFileInfoListtoFilePathList(files));
  delete this;
}

void FileDialog::FileSelectionCanceled(void* params) {
  std::vector<base::FilePath> files;
  files.push_back(base::FilePath());
  file_selected_callback_.Run(false, files);

  delete this;
}

// staic
void FileDialog::Show(const DialogSettings& settings,
                      const DialogCallback& callback) {
  new FileDialog(settings, callback);
}

// staic
bool FileDialog::Show(const DialogSettings& settings,
                      std::vector<base::FilePath>* paths) {
  bool result;
  const DialogCallback callback = base::Bind(OnShowSaveDialog,
                                                 &result, paths);
  new FileDialog(settings, callback);
  base::RunLoop run_loop(base::RunLoop::Type::kNestableTasksAllowed);
  run_loop.Run();
  return result;
}

}  // namespace file_dialog

