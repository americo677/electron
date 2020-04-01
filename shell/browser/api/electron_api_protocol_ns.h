// Copyright (c) 2019 GitHub, Inc.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

#ifndef SHELL_BROWSER_API_ELECTRON_API_PROTOCOL_NS_H_
#define SHELL_BROWSER_API_ELECTRON_API_PROTOCOL_NS_H_

#include <string>
#include <vector>

#include "content/public/browser/content_browser_client.h"
#include "gin/handle.h"
#include "shell/browser/api/trackable_object.h"
#include "shell/browser/net/electron_url_loader_factory.h"
#include "shell/common/gin_helper/dictionary.h"

namespace electron {

class ElectronBrowserContext;

namespace api {

std::vector<std::string> GetStandardSchemes();

void RegisterSchemesAsPrivileged(gin_helper::ErrorThrower thrower,
                                 v8::Local<v8::Value> val);

// Possible errors.
enum class ProtocolError {
  OK,  // no error
  REGISTERED,
  NOT_REGISTERED,
  INTERCEPTED,
  NOT_INTERCEPTED,
};

// Protocol implementation based on network services.
class ProtocolNS : public mate::TrackableObject<ProtocolNS> {
 public:
  static gin::Handle<ProtocolNS> Create(
      v8::Isolate* isolate,
      ElectronBrowserContext* browser_context);

  static void BuildPrototype(v8::Isolate* isolate,
                             v8::Local<v8::FunctionTemplate> prototype);

  using URLLoaderFactoryType =
      content::ContentBrowserClient::URLLoaderFactoryType;

  // Used by ElectronBrowserClient for creating URLLoaderFactory.
  void RegisterURLLoaderFactories(
      URLLoaderFactoryType type,
      content::ContentBrowserClient::NonNetworkURLLoaderFactoryMap* factories);

  const HandlersMap& intercept_handlers() const { return intercept_handlers_; }

 private:
  ProtocolNS(v8::Isolate* isolate, ElectronBrowserContext* browser_context);
  ~ProtocolNS() override;

  // Callback types.
  using CompletionCallback =
      base::RepeatingCallback<void(v8::Local<v8::Value>)>;

  // JS APIs.
  ProtocolError RegisterProtocol(ProtocolType type,
                                 const std::string& scheme,
                                 const ProtocolHandler& handler);
  void UnregisterProtocol(const std::string& scheme, gin::Arguments* args);
  bool IsProtocolRegistered(const std::string& scheme);

  ProtocolError InterceptProtocol(ProtocolType type,
                                  const std::string& scheme,
                                  const ProtocolHandler& handler);
  void UninterceptProtocol(const std::string& scheme, gin::Arguments* args);
  bool IsProtocolIntercepted(const std::string& scheme);

  // Old async version of IsProtocolRegistered.
  v8::Local<v8::Promise> IsProtocolHandled(const std::string& scheme,
                                           gin::Arguments* args);

  // Helper for converting old registration APIs to new RegisterProtocol API.
  template <ProtocolType type>
  void RegisterProtocolFor(const std::string& scheme,
                           const ProtocolHandler& handler,
                           gin::Arguments* args) {
    HandleOptionalCallback(args, RegisterProtocol(type, scheme, handler));
  }
  template <ProtocolType type>
  void InterceptProtocolFor(const std::string& scheme,
                            const ProtocolHandler& handler,
                            gin::Arguments* args) {
    HandleOptionalCallback(args, InterceptProtocol(type, scheme, handler));
  }

  // Be compatible with old interface, which accepts optional callback.
  void HandleOptionalCallback(gin::Arguments* args, ProtocolError error);

  HandlersMap handlers_;
  HandlersMap intercept_handlers_;
};

}  // namespace api

}  // namespace electron

#endif  // SHELL_BROWSER_API_ELECTRON_API_PROTOCOL_NS_H_
