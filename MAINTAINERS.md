## Firmware release process
1. Tag the commit to be released with the next firmware version
1. Build the firmware using Docker
2. Sign the binary firmware
3. Upload signed binary to the Github releases
4. Bump ./STABLE_RELEASE file with latest version
5. Run update script on the update server (if needed)
