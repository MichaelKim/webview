#include "webview.h"

void callback(wv::WebView &w, std::string str) {
  std::cout << str << std::endl;

  if (str == "reset") {
    w.eval("alert('heyo')");
  }
}

int main() {
  wv::WebView w{800, 600, true, true, "Webkit Test", "http://localhost:8080"};

  w.setCallback(callback);

  if (w.init() == -1) {
    return 1;
  }

  while (w.run() == 0)
    ;

  return 0;
}