# Shopify Signal Coach parallel-preview handoff

Existing page, preserved unchanged:
`https://pulsesensor.com/pages/pulsesensor-and-webserial`

Existing page ID: `gid://shopify/Page/117788999785`

- `page-signal-coach.html` is the separate tester-preview page body.
- `page-before-2026-08-12.html` is a content-preserving audit snapshot of the
  existing page with trailing whitespace normalized.
- Create the preview with title `Signal Coach` and handle `signal-coach`.
- Publish the GitHub Pages dashboard before creating the preview so its iframe
  target is already live.
- Do not update or unpublish the existing WebSerial page while tester validation
  is in progress.
