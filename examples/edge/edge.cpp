// Basic webview example
#define WEBVIEW_EDGE
#include "../../webview.hpp"

WEBVIEW_MAIN{
  wv::WebView w{
      800, 600, true, true, Str("WebView"), Str("http://www.google.com")};

  if (w.init() == -1) {
    return 1;
  }

  while (w.run() == 0)
    ;

  return 0;
}