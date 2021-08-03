# Limitations

## EdgeHTML

- The webview cannot be run as an administrator.
- The webview cannot navigate to local HTML files (i.e: `file:///...`).
- To navigate to `localhost`, you must add a loopback exception beforehand:

```pwsh
  CheckNetIsolation.exe LoopbackExempt -a -n=Microsoft.Win32WebViewHost_cw5n1h2txyewy
```

<!-- - To use `preEval`, the displayed page must have at least one `<script>` tag. When no URL is provided, the default URL already contains a `<script>` tag. -->

These can be avoided by using the new Chromium Edge webview.
