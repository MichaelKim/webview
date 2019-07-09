// Basic webview example

#include "webview.h"

#ifdef WIN32
int __stdcall WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
#else
int main() {
#endif
  wv::WebView w{
      800, 600, true, true, Str("WebView"), Str("http://www.google.com")};

  if (w.init() == -1) {
    return 1;
  }

  while (w.run() == 0)
    ;

  return 0;
}