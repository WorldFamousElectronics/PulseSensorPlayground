# Shopify Signal Coach parallel-preview handoff

Existing page, preserved unchanged:
`https://pulsesensor.com/pages/pulsesensor-and-webserial`

Existing page ID: `gid://shopify/Page/117788999785`

Signal Coach preview: `https://pulsesensor.com/pages/signal-coach`

Signal Coach page ID: `gid://shopify/Page/157388144745`

- `page-signal-coach.html` is the separate tester-preview page body.
- `page-before-2026-08-12.html` is a content-preserving audit snapshot of the
  existing page with trailing whitespace normalized.
- The preview was published with title `Signal Coach` and handle `signal-coach`
  on 2026-08-12.
- Publish the GitHub Pages dashboard before creating the preview so its iframe
  target is already live.
- Do not update or unpublish the existing WebSerial page while tester validation
  is in progress.
