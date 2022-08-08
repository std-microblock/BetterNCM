// Copyright (c) 2013 Marshall A. Greenblatt. All rights reserved.
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

#ifndef CEF_INCLUDE_CEF_REQUEST_CONTEXT_H_
#define CEF_INCLUDE_CEF_REQUEST_CONTEXT_H_
#pragma once

#include <vector>

#include "include/cef_callback.h"
#include "include/cef_cookie.h"
#include "include/cef_extension.h"
#include "include/cef_extension_handler.h"
#include "include/cef_media_router.h"
#include "include/cef_values.h"

class CefRequestContextHandler;
class CefSchemeHandlerFactory;

///
// Callback interface for CefRequestContext::ResolveHost.
///
/*--cef(source=client)--*/
class CefResolveCallback : public virtual CefBaseRefCounted {
 public:
  ///
  // Called on the UI thread after the ResolveHost request has completed.
  // |result| will be the result code. |resolved_ips| will be the list of
  // resolved IP addresses or empty if the resolution failed.
  ///
  /*--cef(optional_param=resolved_ips)--*/
  virtual void OnResolveCompleted(
      cef_errorcode_t result,
      const std::vector<CefString>& resolved_ips) = 0;
};

///
// A request context provides request handling for a set of related browser
// or URL request objects. A request context can be specified when creating a
// new browser via the CefBrowserHost static factory methods or when creating a
// new URL request via the CefURLRequest static factory methods. Browser objects
// with different request contexts will never be hosted in the same render
// process. Browser objects with the same request context may or may not be
// hosted in the same render process depending on the process model. Browser
// objects created indirectly via the JavaScript window.open function or
// targeted links will share the same render process and the same request
// context as the source browser. When running in single-process mode there is
// only a single render process (the main process) and so all browsers created
// in single-process mode will share the same request context. This will be the
// first request context passed into a CefBrowserHost static factory method and
// all other request context objects will be ignored.
///
/*--cef(source=library,no_debugct_check)--*/
class CefRequestContext : public virtual CefBaseRefCounted {
 public:
  ///
  // Returns the global context object.
  ///
  /*--cef()--*/
  static CefRefPtr<CefRequestContext> GetGlobalContext();

  ///
  // Creates a new context object with the specified |settings| and optional
  // |handler|.
  ///
  /*--cef(optional_param=handler)--*/
  static CefRefPtr<CefRequestContext> CreateContext(
      const CefRequestContextSettings& settings,
      CefRefPtr<CefRequestContextHandler> handler);

  ///
  // Creates a new context object that shares storage with |other| and uses an
  // optional |handler|.
  ///
  /*--cef(capi_name=cef_create_context_shared,optional_param=handler)--*/
  static CefRefPtr<CefRequestContext> CreateContext(
      CefRefPtr<CefRequestContext> other,
      CefRefPtr<CefRequestContextHandler> handler);

  ///
  // Returns true if this object is pointing to the same context as |that|
  // object.
  ///
  /*--cef()--*/
  virtual bool IsSame(CefRefPtr<CefRequestContext> other) = 0;

  ///
  // Returns true if this object is sharing the same storage as |that| object.
  ///
  /*--cef()--*/
  virtual bool IsSharingWith(CefRefPtr<CefRequestContext> other) = 0;

  ///
  // Returns true if this object is the global context. The global context is
  // used by default when creating a browser or URL request with a NULL context
  // argument.
  ///
  /*--cef()--*/
  virtual bool IsGlobal() = 0;

  ///
  // Returns the handler for this context if any.
  ///
  /*--cef()--*/
  virtual CefRefPtr<CefRequestContextHandler> GetHandler() = 0;

  ///
  // Returns the cache path for this object. If empty an "incognito mode"
  // in-memory cache is being used.
  ///
  /*--cef()--*/
  virtual CefString GetCachePath() = 0;

  ///
  // Returns the cookie manager for this object. If |callback| is non-NULL it
  // will be executed asnychronously on the UI thread after the manager's
  // storage has been initialized.
  ///
  /*--cef(optional_param=callback)--*/
  virtual CefRefPtr<CefCookieManager> GetCookieManager(
      CefRefPtr<CefCompletionCallback> callback) = 0;

  ///
  // Register a scheme handler factory for the specified |scheme_name| and
  // optional |domain_name|. An empty |domain_name| value for a standard scheme
  // will cause the factory to match all domain names. The |domain_name| value
  // will be ignored for non-standard schemes. If |scheme_name| is a built-in
  // scheme and no handler is returned by |factory| then the built-in scheme
  // handler factory will be called. If |scheme_name| is a custom scheme then
  // you must also implement the CefApp::OnRegisterCustomSchemes() method in all
  // processes. This function may be called multiple times to change or remove
  // the factory that matches the specified |scheme_name| and optional
  // |domain_name|. Returns false if an error occurs. This function may be
  // called on any thread in the browser process.
  ///
  /*--cef(optional_param=domain_name,optional_param=factory)--*/
  virtual bool RegisterSchemeHandlerFactory(
      const CefString& scheme_name,
      const CefString& domain_name,
      CefRefPtr<CefSchemeHandlerFactory> factory) = 0;

  ///
  // Clear all registered scheme handler factories. Returns false on error. This
  // function may be called on any thread in the browser process.
  ///
  /*--cef()--*/
  virtual bool ClearSchemeHandlerFactories() = 0;

  ///
  // Tells all renderer processes associated with this context to throw away
  // their plugin list cache. If |reload_pages| is true they will also reload
  // all pages with plugins. CefRequestContextHandler::OnBeforePluginLoad may
  // be called to rebuild the plugin list cache.
  ///
  /*--cef()--*/
  virtual void PurgePluginListCache(bool reload_pages) = 0;

  ///
  // Returns true if a preference with the specified |name| exists. This method
  // must be called on the browser process UI thread.
  ///
  /*--cef()--*/
  virtual bool HasPreference(const CefString& name) = 0;

  ///
  // Returns the value for the preference with the specified |name|. Returns
  // NULL if the preference does not exist. The returned object contains a copy
  // of the underlying preference value and modifications to the returned object
  // will not modify the underlying preference value. This method must be called
  // on the browser process UI thread.
  ///
  /*--cef()--*/
  virtual CefRefPtr<CefValue> GetPreference(const CefString& name) = 0;

  ///
  // Returns all preferences as a dictionary. If |include_defaults| is true then
  // preferences currently at their default value will be included. The returned
  // object contains a copy of the underlying preference values and
  // modifications to the returned object will not modify the underlying
  // preference values. This method must be called on the browser process UI
  // thread.
  ///
  /*--cef()--*/
  virtual CefRefPtr<CefDictionaryValue> GetAllPreferences(
      bool include_defaults) = 0;

  ///
  // Returns true if the preference with the specified |name| can be modified
  // using SetPreference. As one example preferences set via the command-line
  // usually cannot be modified. This method must be called on the browser
  // process UI thread.
  ///
  /*--cef()--*/
  virtual bool CanSetPreference(const CefString& name) = 0;

  ///
  // Set the |value| associated with preference |name|. Returns true if the
  // value is set successfully and false otherwise. If |value| is NULL the
  // preference will be restored to its default value. If setting the preference
  // fails then |error| will be populated with a detailed description of the
  // problem. This method must be called on the browser process UI thread.
  ///
  /*--cef(optional_param=value)--*/
  virtual bool SetPreference(const CefString& name,
                             CefRefPtr<CefValue> value,
                             CefString& error) = 0;

  ///
  // Clears all certificate exceptions that were added as part of handling
  // CefRequestHandler::OnCertificateError(). If you call this it is
  // recommended that you also call CloseAllConnections() or you risk not
  // being prompted again for server certificates if you reconnect quickly.
  // If |callback| is non-NULL it will be executed on the UI thread after
  // completion.
  ///
  /*--cef(optional_param=callback)--*/
  virtual void ClearCertificateExceptions(
      CefRefPtr<CefCompletionCallback> callback) = 0;

  ///
  // Clears all HTTP authentication credentials that were added as part of
  // handling GetAuthCredentials. If |callback| is non-NULL it will be executed
  // on the UI thread after completion.
  ///
  /*--cef(optional_param=callback)--*/
  virtual void ClearHttpAuthCredentials(
      CefRefPtr<CefCompletionCallback> callback) = 0;

  ///
  // Clears all active and idle connections that Chromium currently has.
  // This is only recommended if you have released all other CEF objects but
  // don't yet want to call CefShutdown(). If |callback| is non-NULL it will be
  // executed on the UI thread after completion.
  ///
  /*--cef(optional_param=callback)--*/
  virtual void CloseAllConnections(
      CefRefPtr<CefCompletionCallback> callback) = 0;

  ///
  // Attempts to resolve |origin| to a list of associated IP addresses.
  // |callback| will be executed on the UI thread after completion.
  ///
  /*--cef()--*/
  virtual void ResolveHost(const CefString& origin,
                           CefRefPtr<CefResolveCallback> callback) = 0;

  ///
  // Load an extension.
  //
  // If extension resources will be read from disk using the default load
  // implementation then |root_directory| should be the absolute path to the
  // extension resources directory and |manifest| should be NULL. If extension
  // resources will be provided by the client (e.g. via CefRequestHandler and/or
  // CefExtensionHandler) then |root_directory| should be a path component
  // unique to the extension (if not absolute this will be internally prefixed
  // with the PK_DIR_RESOURCES path) and |manifest| should contain the contents
  // that would otherwise be read from the "manifest.json" file on disk.
  //
  // The loaded extension will be accessible in all contexts sharing the same
  // storage (HasExtension returns true). However, only the context on which
  // this method was called is considered the loader (DidLoadExtension returns
  // true) and only the loader will receive CefRequestContextHandler callbacks
  // for the extension.
  //
  // CefExtensionHandler::OnExtensionLoaded will be called on load success or
  // CefExtensionHandler::OnExtensionLoadFailed will be called on load failure.
  //
  // If the extension specifies a background script via the "background"
  // manifest key then CefExtensionHandler::OnBeforeBackgroundBrowser will be
  // called to create the background browser. See that method for additional
  // information about background scripts.
  //
  // For visible extension views the client application should evaluate the
  // manifest to determine the correct extension URL to load and then pass that
  // URL to the CefBrowserHost::CreateBrowser* function after the extension has
  // loaded. For example, the client can look for the "browser_action" manifest
  // key as documented at https://developer.chrome.com/extensions/browserAction.
  // Extension URLs take the form "chrome-extension://<extension_id>/<path>".
  //
  // Browsers that host extensions differ from normal browsers as follows:
  //  - Can access chrome.* JavaScript APIs if allowed by the manifest. Visit
  //    chrome://extensions-support for the list of extension APIs currently
  //    supported by CEF.
  //  - Main frame navigation to non-extension content is blocked.
  //  - Pinch-zooming is disabled.
  //  - CefBrowserHost::GetExtension returns the hosted extension.
  //  - CefBrowserHost::IsBackgroundHost returns true for background hosts.
  //
  // See https://developer.chrome.com/extensions for extension implementation
  // and usage documentation.
  ///
  /*--cef(optional_param=manifest,optional_param=handler)--*/
  virtual void LoadExtension(const CefString& root_directory,
                             CefRefPtr<CefDictionaryValue> manifest,
                             CefRefPtr<CefExtensionHandler> handler) = 0;

  ///
  // Returns true if this context was used to load the extension identified by
  // |extension_id|. Other contexts sharing the same storage will also have
  // access to the extension (see HasExtension). This method must be called on
  // the browser process UI thread.
  ///
  /*--cef()--*/
  virtual bool DidLoadExtension(const CefString& extension_id) = 0;

  ///
  // Returns true if this context has access to the extension identified by
  // |extension_id|. This may not be the context that was used to load the
  // extension (see DidLoadExtension). This method must be called on the browser
  // process UI thread.
  ///
  /*--cef()--*/
  virtual bool HasExtension(const CefString& extension_id) = 0;

  ///
  // Retrieve the list of all extensions that this context has access to (see
  // HasExtension). |extension_ids| will be populated with the list of extension
  // ID values. Returns true on success. This method must be called on the
  // browser process UI thread.
  ///
  /*--cef()--*/
  virtual bool GetExtensions(std::vector<CefString>& extension_ids) = 0;

  ///
  // Returns the extension matching |extension_id| or NULL if no matching
  // extension is accessible in this context (see HasExtension). This method
  // must be called on the browser process UI thread.
  ///
  /*--cef()--*/
  virtual CefRefPtr<CefExtension> GetExtension(
      const CefString& extension_id) = 0;

  ///
  // Returns the MediaRouter object associated with this context.  If |callback|
  // is non-NULL it will be executed asnychronously on the UI thread after the
  // manager's context has been initialized.
  ///
  /*--cef(optional_param=callback)--*/
  virtual CefRefPtr<CefMediaRouter> GetMediaRouter(
      CefRefPtr<CefCompletionCallback> callback) = 0;
};

#endif  // CEF_INCLUDE_CEF_REQUEST_CONTEXT_H_
