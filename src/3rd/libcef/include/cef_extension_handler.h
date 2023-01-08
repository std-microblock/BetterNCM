// Copyright (c) 2017 Marshall A. Greenblatt. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//    * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//    * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//    * Neither the name of Google Inc. nor the name Chromium Embedded
// Framework nor the names of its contributors may be used to endorse
// or promote products derived from this software without specific prior
// written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// ---------------------------------------------------------------------------
//
// The contents of this file must follow a specific format in order to
// support the CEF translator tool. See the translator.README.txt file in the
// tools directory for more information.
//

#ifndef CEF_INCLUDE_CEF_EXTENSION_HANDLER_H_
#define CEF_INCLUDE_CEF_EXTENSION_HANDLER_H_
#pragma once

#include "include/cef_base.h"
#include "include/cef_browser.h"
#include "include/cef_extension.h"
#include "include/cef_stream.h"

class CefClient;

///
// Callback interface used for asynchronous continuation of
// CefExtensionHandler::GetExtensionResource.
///
/*--cef(source=library)--*/
class CefGetExtensionResourceCallback : public CefBaseRefCounted {
 public:
  ///
  // Continue the request. Read the resource contents from |stream|.
  ///
  /*--cef(capi_name=cont,optional_param=stream)--*/
  virtual void Continue(CefRefPtr<CefStreamReader> stream) = 0;

  ///
  // Cancel the request.
  ///
  /*--cef()--*/
  virtual void Cancel() = 0;
};

///
// Implement this interface to handle events related to browser extensions.
// The methods of this class will be called on the UI thread. See
// CefRequestContext::LoadExtension for information about extension loading.
///
/*--cef(source=client)--*/
class CefExtensionHandler : public virtual CefBaseRefCounted {
 public:
  ///
  // Called if the CefRequestContext::LoadExtension request fails. |result| will
  // be the error code.
  ///
  /*--cef()--*/
  virtual void OnExtensionLoadFailed(cef_errorcode_t result) {}

  ///
  // Called if the CefRequestContext::LoadExtension request succeeds.
  // |extension| is the loaded extension.
  ///
  /*--cef()--*/
  virtual void OnExtensionLoaded(CefRefPtr<CefExtension> extension) {}

  ///
  // Called after the CefExtension::Unload request has completed.
  ///
  /*--cef()--*/
  virtual void OnExtensionUnloaded(CefRefPtr<CefExtension> extension) {}

  ///
  // Called when an extension needs a browser to host a background script
  // specified via the "background" manifest key. The browser will have no
  // visible window and cannot be displayed. |extension| is the extension that
  // is loading the background script. |url| is an internally generated
  // reference to an HTML page that will be used to load the background script
  // via a <script> src attribute. To allow creation of the browser optionally
  // modify |client| and |settings| and return false. To cancel creation of the
  // browser (and consequently cancel load of the background script) return
  // true. Successful creation will be indicated by a call to
  // CefLifeSpanHandler::OnAfterCreated, and CefBrowserHost::IsBackgroundHost
  // will return true for the resulting browser. See
  // https://developer.chrome.com/extensions/event_pages for more information
  // about extension background script usage.
  ///
  /*--cef()--*/
  virtual bool OnBeforeBackgroundBrowser(CefRefPtr<CefExtension> extension,
                                         const CefString& url,
                                         CefRefPtr<CefClient>& client,
                                         CefBrowserSettings& settings) {
    return false;
  }

  ///
  // Called when an extension API (e.g. chrome.tabs.create) requests creation of
  // a new browser. |extension| and |browser| are the source of the API call.
  // |active_browser| may optionally be specified via the windowId property or
  // returned via the GetActiveBrowser() callback and provides the default
  // |client| and |settings| values for the new browser. |index| is the position
  // value optionally specified via the index property. |url| is the URL that
  // will be loaded in the browser. |active| is true if the new browser should
  // be active when opened.  To allow creation of the browser optionally modify
  // |windowInfo|, |client| and |settings| and return false. To cancel creation
  // of the browser return true. Successful creation will be indicated by a call
  // to CefLifeSpanHandler::OnAfterCreated. Any modifications to |windowInfo|
  // will be ignored if |active_browser| is wrapped in a CefBrowserView.
  ///
  /*--cef()--*/
  virtual bool OnBeforeBrowser(CefRefPtr<CefExtension> extension,
                               CefRefPtr<CefBrowser> browser,
                               CefRefPtr<CefBrowser> active_browser,
                               int index,
                               const CefString& url,
                               bool active,
                               CefWindowInfo& windowInfo,
                               CefRefPtr<CefClient>& client,
                               CefBrowserSettings& settings) {
    return false;
  }

  ///
  // Called when no tabId is specified to an extension API call that accepts a
  // tabId parameter (e.g. chrome.tabs.*). |extension| and |browser| are the
  // source of the API call. Return the browser that will be acted on by the API
  // call or return NULL to act on |browser|. The returned browser must share
  // the same CefRequestContext as |browser|. Incognito browsers should not be
  // considered unless the source extension has incognito access enabled, in
  // which case |include_incognito| will be true.
  ///
  /*--cef()--*/
  virtual CefRefPtr<CefBrowser> GetActiveBrowser(
      CefRefPtr<CefExtension> extension,
      CefRefPtr<CefBrowser> browser,
      bool include_incognito) {
    return nullptr;
  }

  ///
  // Called when the tabId associated with |target_browser| is specified to an
  // extension API call that accepts a tabId parameter (e.g. chrome.tabs.*).
  // |extension| and |browser| are the source of the API call. Return true
  // to allow access of false to deny access. Access to incognito browsers
  // should not be allowed unless the source extension has incognito access
  // enabled, in which case |include_incognito| will be true.
  ///
  /*--cef()--*/
  virtual bool CanAccessBrowser(CefRefPtr<CefExtension> extension,
                                CefRefPtr<CefBrowser> browser,
                                bool include_incognito,
                                CefRefPtr<CefBrowser> target_browser) {
    return true;
  }

  ///
  // Called to retrieve an extension resource that would normally be loaded from
  // disk (e.g. if a file parameter is specified to chrome.tabs.executeScript).
  // |extension| and |browser| are the source of the resource request. |file| is
  // the requested relative file path. To handle the resource request return
  // true and execute |callback| either synchronously or asynchronously. For the
  // default behavior which reads the resource from the extension directory on
  // disk return false. Localization substitutions will not be applied to
  // resources handled via this method.
  ///
  /*--cef()--*/
  virtual bool GetExtensionResource(
      CefRefPtr<CefExtension> extension,
      CefRefPtr<CefBrowser> browser,
      const CefString& file,
      CefRefPtr<CefGetExtensionResourceCallback> callback) {
    return false;
  }
};

#endif  // CEF_INCLUDE_CEF_EXTENSION_HANDLER_H_
