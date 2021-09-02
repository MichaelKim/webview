#include "webview.hpp"

WEBVIEW_MAIN {
    wv::WebView w;

    w.navigate(Str(R"(data:text/html,
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta http-equiv="X-UA-Compatible" content="IE=edge">
</head>
<body>
  <script type="text/javascript">
    window.onload = function() {
      window.external.invoke("onload");
    }
  </script>
</body>
</html>)"));

    w.setCallback([](wv::WebView &webview, const wv::String &arg) {
        if (arg == Str("onload")) {
            webview.exit();
        }
    });

    if (w.init() == -1) {
        return 1;
    }

    while (w.run() == 0)
        ;

    return 0;
}
