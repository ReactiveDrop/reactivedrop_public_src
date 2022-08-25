@echo off

cd ..\..\..
set escapedCD=%CD:\=\\%
for /f "tokens=* delims=" %%A in ('git rev-parse HEAD') do (
	echo {"documents":{"%escapedCD%\\*":"https://raw.githubusercontent.com/ReactiveDrop/reactivedrop_public_src/%%A/*"}} > src\sourcelink.json
)
