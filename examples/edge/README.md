# Edge (Chromium) Example

This is an experimental basic example of the new WebView2 control, based off of the new Microsoft Edge (Chromium). As such, this example is only available on Windows.

## Setup

The setup steps are based off of the [official WebView2 docs](https://docs.microsoft.com/en-us/microsoft-edge/hosting/webview2/gettingstarted).

Here is a brief summary of setting up WebView2:
1. Install Microsoft Edge (Chromium). Install the Canary channel as the minimum required version is 82.0.488.0 (currently the Beta channel is too old).
    - With Windows 10 version 2004, the new Edge comes already installed. However, its version is too old for WebView2 (currently 81.0.416.81).
2. Install Visual Studio.
3. Open the Visual Studio project in this directory.
4. Right-click the project, click "Manage NuGet Packages", and install the following packages:
    - Microsoft.Windows.ImplementationLibrary
    - Microsoft.Web.WebView2
5. Run!