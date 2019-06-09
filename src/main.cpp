#include "webview.h"

void callback(wv::WebView &w, std::string &str) {
  std::cout << str << std::endl;

  if (str == "reset") {
    // w.css("body { background-color: green; }");
    // w.eval("alert('heyo')");
    w.navigate("http://google.com");
  } else if (str == "exit") {
    w.exit();
  }
}

int main() {
  wv::WebView w{800, 600, true, true, "Webkit Test", "http://localhost:8080"};

  w.setCallback(callback);
  w.setBgColor(250, 250, 210, 255);

  if (w.init() == -1) {
    return 1;
  }

  while (w.run() == 0)
    ;

  return 0;
}