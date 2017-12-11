// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/prerender/prerender_contents.h"

#include <stddef.h>

#include "build/build_config.h"
#include "chrome/browser/prerender/prerender_manager.h"
#include "chrome/browser/prerender/prerender_manager_factory.h"
#include "content/public/browser/web_contents.h"
#include "components/history/core/browser/history_types.h"

namespace prerender {

void PrerenderContents::DidNavigate(
    const history::HistoryAddPageArgs& add_page_args) {}

void PrerenderContents::Destroy(FinalStatus final_status) {
  DCHECK_NE(final_status, FINAL_STATUS_USED);

  if (prerendering_has_been_cancelled_)
    return;

  SetFinalStatus(final_status);

  prerendering_has_been_cancelled_ = true;
  prerender_manager_->MoveEntryToPendingDelete(this, final_status);
}

void PrerenderContents::SetFinalStatus(FinalStatus final_status) {
  DCHECK_GE(final_status, FINAL_STATUS_USED);
  DCHECK_LT(final_status, FINAL_STATUS_MAX);

  DCHECK_EQ(FINAL_STATUS_MAX, final_status_);

  final_status_ = final_status;
}

// static
PrerenderContents* PrerenderContents::FromWebContents(
    content::WebContents* web_contents) {
  if (!web_contents)
    return NULL;
  PrerenderManager* prerender_manager =
      PrerenderManagerFactory::GetForBrowserContext(
          web_contents->GetBrowserContext());
  if (!prerender_manager)
    return NULL;
  return prerender_manager->GetPrerenderContents(web_contents);
}

}  // namespace prerender
