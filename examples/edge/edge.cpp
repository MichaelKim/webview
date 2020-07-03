// Basic webview example
#define WEBVIEW_EDGE
#include "../../webview.hpp"
#include <filesystem>

long long factorial(long long n) {
	return n <= 1 ? 1 : n * factorial(n - 1);
}

void callback(wv::WebView& w, std::string& arg) {
	try {
		long long num = std::stoll(arg);
		w.eval(Str("result(") + std::to_wstring(factorial(num)) + Str(")"));
	}
	catch (std::exception e) {
		w.eval(Str("result('Invalid number')"));
	}
}

WEBVIEW_MAIN{
	auto cwd = std::filesystem::current_path();

	wv::WebView w{800, 600, true, true, Str("WebView Callback"), Str("file:///") + wv::String(cwd / "index.html")};

	// This can be called before or after w.init();
	w.setCallback(callback);

	if (w.init() == -1) {
	  return 1;
	}

	while (w.run() == 0)
	  ;

	return 0;
}